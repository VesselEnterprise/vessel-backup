#include "http_client.hpp"

using namespace Backup::Networking;

HttpClient::HttpClient(const std::string& uri) :
    m_socket(m_io_service),
    m_deadline_timer(m_io_service),
    m_use_ssl(false),
    m_ssl_ctx(boost::asio::ssl::context::tlsv12),
    m_ssl_socket(m_io_service,m_ssl_ctx),
    m_timeout(boost::posix_time::seconds(5)),
    m_verify_cert(true)
{

     //Set local database object
    m_ldb = &Backup::Database::LocalDatabase::get_database();

        //Create new log obj
    m_log = new Backup::Logging::Log("asio");

    //Set SSL Opts
    m_ssl_ctx.set_default_verify_paths();

    if ( m_verify_cert )
        m_ssl_socket.set_verify_mode(boost::asio::ssl::verify_peer);
    else
        m_ssl_socket.set_verify_mode(boost::asio::ssl::verify_none);

    //Determine protocol, hostname, etc
    this->parse_url(uri);

    //Timer should not expire until a connection attempt is made
    m_deadline_timer.expires_at(boost::posix_time::pos_infin);
    this->check_deadline();

}

HttpClient::~HttpClient()
{
    if ( m_socket.is_open() )
        m_socket.close();

    if ( m_ssl_socket.lowest_layer().is_open() )
        m_ssl_socket.lowest_layer().close();
}

void HttpClient::parse_url( const std::string& host )
{

    //Detect HTTP protocol
    m_protocol = host.substr( 0, host.find_first_of(':') );

    if ( m_protocol == "https" )
    {
        m_use_ssl = true;
        m_port=443;
        unsigned int proto_len = host.find_first_of(':') + 3; //Len of protocol part
        m_hostname = host.substr( proto_len );
        m_hostname = m_hostname.substr(0, m_hostname.find_first_of('/') );
        m_uri = host.substr( proto_len + m_hostname.length() );

        m_ssl_socket.set_verify_callback(boost::asio::ssl::rfc2818_verification(m_hostname));
    }
    else if ( m_protocol == "http" )
    {
        m_use_ssl=false;
        m_port=80;
        unsigned int proto_len = host.find_first_of(':') + 3; //Len of protocol part
        m_hostname = host.substr( proto_len );
        m_hostname = m_hostname.substr(0, m_hostname.find_first_of('/') );
        m_uri = host.substr( proto_len + m_hostname.length() );
    }
    else //Invalid Protocol or none specified?
    {
        m_protocol = "http"; //use HTTP by default
        m_use_ssl=false;
        m_hostname = host.substr( 0, host.find_last_of('/') );
        m_uri = host.substr( m_hostname.length() );
    }

    unsigned int hostlen = m_hostname.length();
    if ( m_hostname[hostlen-1] == '/' )
        m_hostname[hostlen-1] = 0; //terminate string

    /*
    std::cout << "HTTP Protocol: " << m_protocol << '\n';
    std::cout << "Hostname: " << m_hostname << '\n';
    std::cout << "URI: " << m_uri << '\n';
    */

}

void HttpClient::set_timeout( boost::posix_time::time_duration t )
{
    m_timeout = t;
}

void HttpClient::set_ssl(bool f)
{
    m_use_ssl = f;
}

bool HttpClient::connect()
{

    m_conn_status.clear();

    //Set timeout
    m_deadline_timer.expires_from_now(m_timeout);

    try
    {

        boost::asio::ip::tcp::resolver resolver(m_io_service);
        boost::asio::ip::tcp::resolver::query query( m_hostname.c_str(), m_protocol.c_str() );
        boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

        //Block the call to ASYNC connect until the error code changes
        m_conn_status = boost::asio::error::would_block;

        if ( !m_use_ssl )
            boost::asio::async_connect(m_socket, endpoint_iterator, boost::bind(&HttpClient::handle_connect, this, boost::asio::placeholders::error) );
        else
            boost::asio::async_connect(m_ssl_socket.lowest_layer(), endpoint_iterator, boost::bind(&HttpClient::handle_connect, this, boost::asio::placeholders::error) );

        //Run ASYNC connect until operation completed
        do m_io_service.run_one(); while (m_conn_status == boost::asio::error::would_block);

        if ( !m_conn_status )
        {
            m_connected=true;
        }
        else
        {
            throw boost::system::system_error( m_conn_status ? m_conn_status : boost::asio::error::operation_aborted );
            m_connected=false;
        }

        /*
        for (;;)
        {
            boost::array<char, 128> buf;
            boost::system::error_code error;

            size_t len = m_socket.read_some(boost::asio::buffer(buf), error);

            if (error == boost::asio::error::eof)
                break; // Connection closed cleanly by peer.
            else if (error)
                throw boost::system::system_error(error); // Some other error.

            std::cout.write(buf.data(), len);

            std::cout << "Reading data..." << std::endl;
        }
        */

    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        m_connected=false;
    }

    return m_connected;
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
            m_ssl_socket.async_handshake(boost::asio::ssl::stream_base::client,boost::bind(&HttpClient::handle_handshake, this, boost::asio::placeholders::error) );

            //Do not return until handshake is complete
            do m_io_service.run_one(); while (m_conn_status == boost::asio::error::would_block && !m_ssl_good);
        }

    }
    else
    {
        std::cout << "Connect failed: " << e.message() << "\n";
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

        /*
        std::cout << "Enter message: ";
        std::cin.getline(request_, max_length);
        size_t request_length = strlen(request_);

        boost::asio::async_write(
            m_ssl_socket,
            boost::asio::buffer(***, request_length),
            boost::bind(&BackupClient::handle_write, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)
        );
        */
    }
    else
    {
        m_ssl_good=false;
        m_conn_status = e;
        std::cout << "Handshake failed: " << e.message() << "\n";
    }

}

void HttpClient::handle_write( const boost::system::error_code& e )
{

    std::cout << "Handle write..." << '\n';

    if (!e)
    {

        //Set the deadline timer
        m_deadline_timer.expires_from_now(boost::posix_time::seconds(10));

        // Read the response status line. The response_ streambuf will
        // automatically grow to accommodate the entire line. The growth may be
        // limited by passing a maximum size to the streambuf constructor.
        if ( !m_use_ssl )
            boost::asio::async_read_until(m_socket, m_response_buffer, "\r\n", boost::bind(&HttpClient::handle_response, this, boost::asio::placeholders::error));
        else
            boost::asio::async_read_until(m_ssl_socket, m_response_buffer, "\r\n", boost::bind(&HttpClient::handle_response, this, boost::asio::placeholders::error));

    }
    else
    {
        std::cout << "ASIO Write Error: " << e.message() << "\n";
    }

}

void HttpClient::handle_response( const boost::system::error_code& e )
{

    m_response_ec = e; //Update error code

    //std::cout << "Handled response.." << '\n';

    if (!e)
    {

        //Set the deadline timer
        //m_deadline_timer.expires_from_now(boost::posix_time::seconds(10));

        //Reset HTTP Status
        m_http_status = 0;

        /** Parse the response headers **/

        std::istream response_stream(&m_response_buffer);

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
            boost::asio::async_read_until(m_socket, m_response_buffer, "\r\n\r\n", boost::bind(&HttpClient::handle_read_headers, this, boost::asio::placeholders::error));
        else
            boost::asio::async_read_until(m_ssl_socket, m_response_buffer, "\r\n\r\n", boost::bind(&HttpClient::handle_read_headers, this, boost::asio::placeholders::error));

    }
    else
    {
        std::cout << "ASIO Response Error: " << e << "\n";
    }

}

void HttpClient::handle_read_headers( const boost::system::error_code& e )
{

    std::cout << "Handle read headers..." << '\n';

    if (!e)
    {
        //Read header data
        std::istream response_stream(&m_response_buffer);
        std::noskipws( response_stream ); //Don't skip white spaces
        std::string header;
        while (std::getline(response_stream, header) && header != "\r")
            m_header_data += header.append("\n");

        //There may be some data in the buffer to consume
        if (m_response_buffer.size() > 0) {
            m_response_data.append( std::istream_iterator<char>(response_stream), std::istream_iterator<char>() );
        }

        // Start reading remaining data until EOF.
        if ( !m_use_ssl )
            boost::asio::async_read( m_socket, m_response_buffer, boost::asio::transfer_at_least(1), boost::bind(&HttpClient::handle_read_content, this, boost::asio::placeholders::error) );
        else
            boost::asio::async_read( m_ssl_socket, m_response_buffer, boost::asio::transfer_at_least(1), boost::bind(&HttpClient::handle_read_content, this, boost::asio::placeholders::error) );

    }
    else
    {
        std::cout << "ASIO Read Header Error: " << e << "\n";
    }

    m_response_ec = e;

}

void HttpClient::handle_read_content( const boost::system::error_code& e )
{

    std::cout << "Read some content" << '\n';

    if (!e)
    {

        //Read data from buffer into a stream
        std::istream response_data(&m_response_buffer);
        std::noskipws( response_data ); //Don't skip white spaces

        m_response_data.append( std::string(std::istream_iterator<char>(response_data), std::istream_iterator<char>()) );

        // Continue reading remaining data until EOF.
        if ( !m_use_ssl )
            boost::asio::async_read(m_socket, m_response_buffer, boost::asio::transfer_at_least(1), boost::bind(&HttpClient::handle_read_content, this, boost::asio::placeholders::error));
        else
            boost::asio::async_read(m_ssl_socket, m_response_buffer, boost::asio::transfer_at_least(1), boost::bind(&HttpClient::handle_read_content, this, boost::asio::placeholders::error));

    }
    else if (e != boost::asio::error::eof)
    {
        /** Some other error occurred while reading the response **/
        m_log->add_message("Unknown error occurred while reading the HTTP response: " + e.message(), "ASIO");
        m_response_ec = boost::asio::error::eof; //Signal eof to start reading
    }
    else
    {

        std::cout << m_response_data << "\n";

        /** Response should be read at this point **/
        m_response_ec = boost::asio::error::eof;

    }

}

void HttpClient::check_deadline()
{
    // Check whether the deadline has passed. We compare the deadline against
    // the current time since a new asynchronous operation may have moved the
    // deadline before this actor had a chance to run.
    if (m_deadline_timer.expires_at() <= boost::asio::deadline_timer::traits_type::now())
    {
        // The deadline has passed. The socket is closed so that any outstanding
        // asynchronous operations are cancelled. This allows the blocked
        // connect(), read_line() or write_line() functions to return.
        this->disconnect();

        // There is no longer an active deadline. The expiry is set to positive
        // infinity so that the actor takes no action until a new deadline is set.
        m_deadline_timer.expires_at(boost::posix_time::pos_infin);
    }

    // Put the actor back to sleep.
    m_deadline_timer.async_wait(boost::bind(&HttpClient::check_deadline, this));

}

void HttpClient::disconnect()
{
    try
    {
        if ( m_socket.is_open() ) {
            //m_socket.shutdown( tcp::socket::shutdown_both );
            m_socket.close();
        }

        if ( m_ssl_socket.lowest_layer().is_open() ) {
            //m_ssl_socket.lowest_layer().shutdown( tcp::socket::shutdown_both );
            m_ssl_socket.lowest_layer().close();
        }
    }
    catch ( boost::system::system_error& ec )
    {
        std::cout << "There was some error" << std::endl;
    }

    m_connected=false;

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
    m_deadline_timer.expires_from_now(boost::posix_time::seconds(seconds));
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

void HttpClient::write_socket(const std::string& data)
{
    if ( !m_use_ssl )
        boost::asio::async_write(m_socket, boost::asio::buffer( data, data.size() ), boost::bind(&HttpClient::handle_write, this, boost::asio::placeholders::error)) ;
    else
        boost::asio::async_write(m_ssl_socket, boost::asio::buffer( data, data.size() ), boost::bind(&HttpClient::handle_write, this, boost::asio::placeholders::error)) ;
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
    escaped << "/";
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
