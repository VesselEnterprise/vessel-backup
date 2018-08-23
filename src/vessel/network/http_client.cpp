#include <vessel/network/http_client.hpp>

using namespace Vessel::Networking;

HttpClient::HttpClient(const std::string& uri) : m_ssl_ctx(boost::asio::ssl::context::tlsv12)
{
    set_defaults();

     //Set local database object
    m_ldb = &Vessel::Database::LocalDatabase::get_database();

        //Create new log obj
    m_log = new Vessel::Logging::Log("asio");

    //Set SSL Opts
    m_ssl_ctx.set_default_verify_paths();

    //Determine protocol, hostname, etc
    parse_url(uri);

}

HttpClient::~HttpClient()
{
    if ( m_socket ) {
        if ( m_socket->is_open() ) {
            m_socket->close();
        }
    }

    if ( m_ssl_socket ) {
        if ( m_ssl_socket->lowest_layer().is_open() ) {
            m_ssl_socket->lowest_layer().close();
        }
    }

    m_socket.reset();
    m_ssl_socket.reset();

}

void HttpClient::set_defaults()
{
    m_verify_cert = true;
    m_connected = false;
    m_use_ssl = false;
    m_timeout = boost::posix_time::seconds(60);
    m_verify_cert = true;
    m_connected = false;
}

void HttpClient::parse_url( const std::string& host )
{

    m_port = 0; //Reset port

    //Parse the URL parts using regex - Excellent solution: https://stackoverflow.com/questions/5620235/cpp-regular-expression-to-validate-url
    std::regex url_regex (R"(^(([^:\/?#]+):)?(//([^\/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?)", std::regex::extended );
    std::smatch url_match_result;
    std::vector<std::string> url_parts;

    if (std::regex_match(host, url_match_result, url_regex))
    {
        for (const auto& res : url_match_result)
        {
            url_parts.push_back(res);
            //std::cout << res << '\n';
        }
    }
    else {
        throw HttpException(HttpException::InvalidUrl, "Invalid URL has been provided: " + host );
    }

    m_protocol = url_parts[2];
    m_hostname = url_parts[4];

    //Check if port is present in m_hostname
    std::size_t port_pos = m_hostname.find_first_of(':');
    if ( port_pos != std::string::npos )
    {
        std::string port_s = m_hostname.substr( port_pos+1, std::string::npos );
        if ( port_s.length() > 0 )
        {
            m_port = std::stoi(port_s);
        }
        //Remove port from hostname
        m_hostname = m_hostname.substr(0, port_pos);
    }

    if ( m_port <= 0 )
    {
        if ( m_protocol == "http" )
        {
            m_port = 80;
        }
        else if ( m_protocol == "https" )
        {
            m_port = 443;
        }
    }

    if ( m_protocol == "https" )
    {
        m_use_ssl = true;
    }
    else
    {
        m_use_ssl = false;
    }

}

void HttpClient::set_timeout( boost::posix_time::time_duration t )
{
    m_timeout = t;
}

void HttpClient::set_ssl(bool flag)
{
    m_use_ssl = flag;
}

bool HttpClient::connect()
{

    //Reset sockets
    m_socket.reset( new boost::asio::ip::tcp::socket(m_io_service) );
    m_ssl_socket.reset( new boost::asio::ssl::stream<boost::asio::ip::tcp::socket>(m_io_service,m_ssl_ctx) );

    //Reset deadline timer
    m_deadline_timer.reset ( new boost::asio::deadline_timer(m_io_service) );

    //Clear connection status code
    m_conn_status.clear();

    //Set timeout
    m_deadline_timer->expires_from_now(m_timeout); //m_deadline_timer.expires_at(boost::posix_time::pos_infin);

    check_deadline(); //Run indefinitely

    boost::asio::ip::tcp::resolver resolver(m_io_service);
    boost::asio::ip::tcp::resolver::query query( m_hostname.c_str(), std::to_string(m_port) );
    boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

    //Block the call to ASYNC connect until the error code changes
    m_conn_status = boost::asio::error::would_block;

    if ( !m_use_ssl ) {

        boost::asio::async_connect(*m_socket, endpoint_iterator, boost::bind(&HttpClient::handle_connect, this, boost::asio::placeholders::error) );

    }
    else {

        m_ssl_socket->set_verify_callback(boost::asio::ssl::rfc2818_verification(m_hostname));

        if ( m_verify_cert ) {
            m_ssl_socket->set_verify_mode(boost::asio::ssl::verify_peer);
        }
        else {
            m_ssl_socket->set_verify_mode(boost::asio::ssl::verify_none);
        }

        boost::asio::async_connect(m_ssl_socket->lowest_layer(), endpoint_iterator, boost::bind(&HttpClient::handle_connect, this, boost::asio::placeholders::error) );

    }

    //Run ASYNC connect until operation completed
    do m_io_service.run_one(); while (m_conn_status == boost::asio::error::would_block);

    if ( !m_conn_status )
    {
        m_connected=true;
    }
    else
    {
        //throw boost::system::system_error( m_conn_status ? m_conn_status : boost::asio::error::operation_aborted );
        m_connected=false;
        throw HttpException(HttpException::ConnectFailed, std::string("Failed to connect to " + m_hostname) );
    }

    return m_connected;
}

void HttpClient::disconnect()
{

    boost::system::error_code disconnect_ec;
    try
    {
        if ( m_socket ) {
            if ( m_socket->is_open() ) {
                m_socket->shutdown( boost::asio::ip::tcp::socket::shutdown_both, disconnect_ec );
                m_socket->close();
            }
        }

        if ( m_ssl_socket ) {
            if ( m_ssl_socket->lowest_layer().is_open() ) {
                //m_ssl_socket->shutdown(); //Shutdown the SSL stream
                m_ssl_socket->lowest_layer().shutdown( boost::asio::ip::tcp::socket::shutdown_both, disconnect_ec );
                m_ssl_socket->lowest_layer().close(); //Close the TCP socket
            }
        }
        //Cancel the deadline
        m_deadline_timer->cancel();

        if ( disconnect_ec ) {
            //Handle disconnect error
            //m_log->....
        }
    }
    catch ( const boost::system::system_error & e )
    {
        std::cout << "ASIO Socket error: " << e.what() << "\n";
        m_conn_status = e.code();
    }

    m_io_service.stop();
    m_io_service.reset();
    m_socket.reset();
    m_ssl_socket.reset();

    //Clear error codes
    m_response_ec.clear();
    m_connected=false;

}

void HttpClient::handle_connect(const boost::system::error_code& e)
{

    if ( !e )
    {

        if ( !m_use_ssl )
        {
            m_conn_status = e;
            return;
        }
        else //SSL client must perform handshake
        {
            m_ssl_good=false; //Reset flag
            m_ssl_socket->async_handshake(boost::asio::ssl::stream_base::client,boost::bind(&HttpClient::handle_handshake, this, boost::asio::placeholders::error) );

            //Do not return until handshake is complete
            do m_io_service.run_one(); while (m_conn_status == boost::asio::error::would_block && !m_ssl_good);
        }

    }

    //Update connection status error code
    m_conn_status = e;

}

void HttpClient::handle_handshake(const boost::system::error_code& e )
{

    if (!e)
    {
        m_ssl_good=true;
        m_conn_status = e;

        std::cout << "SSL Handshake successful" << "\n";

    }
    else
    {
        m_ssl_good=false;
        m_conn_status = e;
        throw HttpException(HttpException::HandshakeFailed, e.message() );
    }

}

void HttpClient::read_chunked_content( const boost::system::error_code& e, size_t bytes_transferred )
{

    if ( bytes_transferred <= 0 )
    {
        m_response_ec = boost::asio::error::eof;
        return;
    }

    /*
    std::string data( (std::istreambuf_iterator<char>(m_response_buffer.get()) ), std::istreambuf_iterator<char>() );
    std::stringstream chunk_stream;
    chunk_stream << std::hex << chunk_sz_s;
    chunk_stream >> chunk_sz;
    */

    std::cout << "Buffer size: " << m_response_buffer->size() << "; Bytes transferred: " << bytes_transferred << '\n';

    std::string chunk_sz_s;

    std::istream chunk_stream( m_response_buffer.get() );

    //Read the chunk size
    std::getline(chunk_stream, chunk_sz_s);

    //Remove carriage return
    boost::replace_all(chunk_sz_s, "\r", "");

    std::cout << "String chunk size is: " << chunk_sz_s << '\n';

    int chunk_sz = std::stoul(chunk_sz_s, nullptr, 16);

    std::cout << "Read chunk size: " << chunk_sz << '\n';

    if ( chunk_sz <= 0 ) //There is no more data to read
    {
        m_response_ec = boost::asio::error::eof;
        return;
    }

    std::cout << "Response buffer after read: " << m_response_buffer->size() << '\n';

    int buffer_sz = m_response_buffer->size();
    int bytes_to_read = ( chunk_sz - buffer_sz ) + 2; //Add two for "\r\n" delimiter

    std::cout << "Bytes to read: " << bytes_to_read << '\n';

    //Consume the buffer up until the delimiter
    //m_response_buffer->consume(bytes_transferred);

    //Read the rest of the chunk into the buffer
    if ( bytes_to_read > 0 )
    {
        if ( m_use_ssl )
        {
            boost::asio::read(*m_ssl_socket, *m_response_buffer, boost::asio::transfer_exactly(bytes_to_read), m_response_ec);
        }
        else
        {
            boost::asio::read(*m_socket, *m_response_buffer, boost::asio::transfer_exactly(bytes_to_read), m_response_ec);
        }
    }

    //Read any remaining data in the buffer
    if ( m_response_buffer->size() > 0 )
    {
        std::cout << "Read the buffer data..." << '\n';
        read_buffer_data();

        //Remove the delimiter from the end of data "\r\n"
        m_response_data = m_response_data.substr(0, m_response_data.size()-2 );
    }

    //Read the next chunk
    if ( m_use_ssl )
    {
        boost::asio::async_read_until( *m_ssl_socket, *m_response_buffer, "\r\n", boost::bind(&HttpClient::read_chunked_content, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred) );
    }
    else
    {
        boost::asio::async_read_until( *m_socket, *m_response_buffer, "\r\n", boost::bind(&HttpClient::read_chunked_content, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred) );
    }

}

void HttpClient::handle_response( const boost::system::error_code& e )
{

    m_response_ec = e; //Update error code

    std::cout << "Handled response.." << '\n';

    if (!e)
    {

        //Set the deadline timer
        //m_deadline_timer->expires_from_now(boost::posix_time::seconds(10));

        //Reset HTTP Status
        m_http_status = 0;

        /** Parse the response headers **/

        std::istream response_stream( m_response_buffer.get() );

        // eg.
        std::string http_version;
        response_stream >> http_version;
        response_stream >> m_http_status;

        std::string status_message;
        std::getline(response_stream, status_message);

        if (!response_stream || http_version.substr(0, 5) != "HTTP/")
        {
            m_log->add_message("There was an error processing the HTTP response headers: " + e.message(), "ASIO");
            m_response_ec = boost::asio::error::operation_aborted; //Handle status error
            return;
        }

        /** HTTP Redirects **/
        if ( m_http_status == 301 )
        {

            std::string new_location;

            //Find redirect location
            std::string tmp_response;
            while ( std::getline(response_stream, tmp_response) )
            {

                if ( tmp_response.find("Location:") != std::string::npos )
                {
                    new_location = tmp_response.substr(10);
                    break;
                }
            }

            m_log->add_message("HTTP 301 redirect detected: " + new_location, "ASIO");

            //Close Open Socket
            disconnect();

            //Try to reconnect to new host
            parse_url(new_location.c_str());
            connect();

            return;

        }

        // Read the response headers, which are terminated by a blank line.
        if ( !m_use_ssl )
            boost::asio::async_read_until(*m_socket, *m_response_buffer, "\r\n\r\n", boost::bind(&HttpClient::handle_read_headers, this, boost::asio::placeholders::error));
        else
            boost::asio::async_read_until(*m_ssl_socket, *m_response_buffer, "\r\n\r\n", boost::bind(&HttpClient::handle_read_headers, this, boost::asio::placeholders::error));

    }
    else if ( e != boost::asio::error::eof )
    {
        std::cout << "ASIO Response Error: " << e << "\n";
    }

}

void HttpClient::read_buffer_data()
{
    m_response_data.append( (std::istreambuf_iterator<char>( m_response_buffer.get() )), std::istreambuf_iterator<char>() );
}

void HttpClient::handle_read_headers( const boost::system::error_code& e )
{

    std::cout << "Handle read headers..." << '\n';

    if (!e)
    {
        //Read header data
        std::istream response_stream( m_response_buffer.get() );
        std::noskipws( response_stream ); //Don't skip white spaces
        std::string header;
        while (std::getline(response_stream, header) && header != "\r") {
            m_header_data += header.append("\n");
        }

        std::cout << "Read headers: " << '\n' << m_header_data << '\n';
        //std::cin.get();

        //There may be some data in the buffer to consume
        if (m_response_buffer->size() > 0) {

            read_buffer_data();

        }

        //Check for chunked transfer encoding
        if ( m_header_data.find("Transfer-Encoding: chunked") != std::string::npos )
        {
            m_chunked_encoding=true;
        }
        else
        {
            m_chunked_encoding=false;
        }

        if ( m_chunked_encoding )
        {
            if ( m_use_ssl )
            {
                boost::asio::async_read_until( *m_ssl_socket, *m_response_buffer, "\r\n", boost::bind(&HttpClient::read_chunked_content, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred) );
            }
            else
            {
                boost::asio::async_read_until( *m_socket, *m_response_buffer, "\r\n", boost::bind(&HttpClient::read_chunked_content, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred) );
            }

        }
        else
        {
            // Start reading remaining data until EOF
            if ( m_use_ssl )
            {
                boost::asio::async_read( *m_ssl_socket, *m_response_buffer, boost::asio::transfer_at_least(1), boost::bind(&HttpClient::handle_read_content, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred) );
            }
            else
            {
                boost::asio::async_read( *m_socket, *m_response_buffer, boost::asio::transfer_at_least(1), boost::bind(&HttpClient::handle_read_content, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred) );
            }

        }
    }
    else
    {
        std::cout << "ASIO Read Header Error: " << e << "\n";
        m_response_ec = e;
    }

}

void HttpClient::handle_read_content( const boost::system::error_code& e, size_t bytes_transferred )
{

    std::cout << "Read some content" << '\n';

    if (!e)
    {
        //Add to response data
        /*
        std::ostringstream oss;
        oss << m_response_buffer.get();
        m_response_data.append(oss.str());
        */

        //Append response data
        read_buffer_data();

        // Continue reading remaining data until EOF.
        if ( m_use_ssl )
        {
            boost::asio::async_read(*m_ssl_socket, *m_response_buffer, boost::asio::transfer_at_least(1), boost::bind(&HttpClient::handle_read_content, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
        }
        else
        {
            boost::asio::async_read(*m_socket, *m_response_buffer, boost::asio::transfer_at_least(1), boost::bind(&HttpClient::handle_read_content, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
        }

    }
    else if (e != boost::asio::error::eof)
    {
        /** Some other error occurred while reading the response **/
        m_log->add_message("Unknown error occurred while reading the HTTP response: " + e.message(), "ASIO");
        m_response_ec = boost::asio::error::eof; //Signal eof to start reading
    }
    else
    {
        /** Response should be read at this point **/
        m_response_ec = boost::asio::error::eof;
    }

}

void HttpClient::check_deadline()
{
    // Check whether the deadline has passed. We compare the deadline against
    // the current time since a new asynchronous operation may have moved the
    // deadline before this actor had a chance to run.
    if (m_deadline_timer->expires_at() <= boost::asio::deadline_timer::traits_type::now())
    {

        std::cout << "Deadline has expired!" << "\n";

        // The deadline has passed. The socket is closed so that any outstanding
        // asynchronous operations are cancelled. This allows the blocked
        // connect(), read_line() or write_line() functions to return.
        disconnect();

        // There is no longer an active deadline. The expiry is set to positive
        // infinity so that the actor takes no action until a new deadline is set.
        //m_deadline_timer->expires_at(boost::posix_time::pos_infin);
    }

    // Put the actor back to sleep.
    m_deadline_timer->async_wait(boost::bind(&HttpClient::check_deadline, this));

}

void HttpClient::set_error( const std::string& msg )
{
    m_error_message = msg;
}

std::string HttpClient::get_error()
{
    return m_error_message;
}

std::string HttpClient::get_response()
{
    return m_response_data;
}

std::string HttpClient::get_headers()
{
    return m_header_data;
}

unsigned int HttpClient::get_http_status()
{
    return m_http_status;
}

void HttpClient::clear_response()
{
    m_response_data.clear();
}

void HttpClient::clear_headers()
{
    m_header_data.clear();
}

void HttpClient::set_deadline(long seconds)
{
    m_deadline_timer->expires_from_now(boost::posix_time::seconds(seconds));
}

std::string HttpClient::get_hostname()
{
    return m_hostname;
}

void HttpClient::clear_error_code()
{
    m_response_ec.clear();
}

bool HttpClient::is_https()
{
    return m_use_ssl;
}

void HttpClient::write_socket( const std::string& http_request)
{

    //Cleanup buffers / data
    cleanup();

    m_deadline_timer->expires_at(boost::posix_time::pos_infin); //Transfer indefinitely

    std::cout << "Transferring " << http_request.size() << " bytes..." << '\n';

    //TODO: Implement bandwidth throttling, limit transfer speed somewhere in here

    if ( !m_use_ssl )
        boost::asio::async_write(*m_socket, boost::asio::buffer(http_request), boost::bind(&HttpClient::handle_write, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred )) ;
    else
        boost::asio::async_write(*m_ssl_socket, boost::asio::buffer(http_request), boost::bind(&HttpClient::handle_write, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred )) ;

}

void HttpClient::handle_write( const boost::system::error_code& e, size_t bytes_transferred )
{

    //std::cout << "Sent " << bytes_transferred << " bytes" << '\n';
    //std::cout << "Buffer size: " << m_request_buffer->size() << " bytes" << '\n';

    if (!e)
    {

        if ( !m_use_ssl )
            boost::asio::async_read_until(*m_socket, *m_response_buffer, "\r\n", boost::bind(&HttpClient::handle_response, this, boost::asio::placeholders::error));
        else
            boost::asio::async_read_until(*m_ssl_socket, *m_response_buffer, "\r\n", boost::bind(&HttpClient::handle_response, this, boost::asio::placeholders::error));

    }
    else
    {
        std::cout << "ASIO Write Error: " << e.message() << "\n";
        m_response_ec = e;
    }

}

void HttpClient::run_io_service()
{
    m_io_service.run_one();
}

boost::system::error_code HttpClient::get_error_code()
{
    return m_response_ec;
}

bool HttpClient::is_connected()
{
    return m_connected;
}

std::string HttpClient::get_uri_path()
{
    return m_uri;
}

void HttpClient::cleanup()
{
    //Refresh buffer(s)
    m_response_buffer.reset( new boost::asio::streambuf() );
    clear_response();
    clear_headers();
}

/*
 * Credit to: https://stackoverflow.com/questions/154536/encode-decode-urls-in-c
*/
std::string HttpClient::encode_uri(const std::string& uri)
{

    std::string uri_copy = uri;

    //Check if 1st char is "/" - don't encode
    if ( uri_copy[0] == '/' )
        uri_copy = uri_copy.substr(1);


    std::ostringstream escaped;
    //escaped << "/";
    escaped.fill('0');
    escaped << std::hex;

    for ( std::string::const_iterator i = uri_copy.begin(), n = uri_copy.end(); i != n; ++i )
    {

        std::string::value_type c = (*i);

        // Keep alphanumeric and other accepted characters intact
        if ( isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
            continue;
        }

        // Any other characters are percent-encoded
        escaped << std::uppercase;
        escaped << '%' << std::setw(2) << int((unsigned char) c);
        escaped << std::nouppercase;
    }

    return escaped.str();

}

int HttpClient::send_http_request( const HttpRequest& request )
{

    HttpRequestStream http_stream;
    std::string accept = request.get_accept();
    std::string authorization = request.get_auth();
    std::string http_method = request.get_method();
    std::string content_type = request.get_content_type();

    //Build the HTTP Request
    http_stream << http_method << " " << request.get_url() << " HTTP/1.0\r\n";
    http_stream << "Host: " << get_hostname() << ":" << std::to_string(m_port) << "\r\n";

    if ( !accept.empty() )
    {
        http_stream << "Accept: " << accept << "\r\n";
    }

    //Send Authorization Header
    if ( !authorization.empty() ) {
        http_stream << "Authorization: " << authorization << "\r\n";
    }

    bool send_body = false;

    //If POST or PUT, send content length and type headers
    if ( http_method == "POST" || http_method == "PUT" )
    {
        http_stream << "Content-Length: " << request.get_body_length() << "\r\n";

        if ( !content_type.empty() )
        {
            http_stream << "Content-Type: " << content_type << "\r\n";
        }

        if ( request.get_body_length() > 0 )
        {
            send_body=true;
        }

    }

    //Add any custom headers
    std::vector<std::string> headers = request.get_headers();
    for ( auto itr : headers ) {
        http_stream << itr << "\r\n";
    }

    //Close connection after response
    http_stream << "Connection: close\r\n\r\n";

    if ( send_body ) {
        http_stream << request.get_body();
    }

    std::cout << "Sending request:" << '\n' << http_stream.str() << '\n';

    //If client is already connected, disconnect before a new attempt
    if ( is_connected() ) {
        disconnect();
    }

    //Connect to server
    connect();

    //Write HTTP request to socket
    write_socket(http_stream.str());

    //Run the handlers until the response sets the status code or EOF
    do { run_io_service(); std::cout << "..." << '\n'; } while ( !get_error_code() );

    disconnect();

    unsigned int http_status = get_http_status();

    std::cout << "HTTP Status: " << http_status << "\n";
    std::cout << "HTTP Response: " << get_response() << '\n';

    return http_status;

}
