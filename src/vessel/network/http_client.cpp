#include <vessel/network/http_client.hpp>

using namespace Vessel::Networking;

bool HttpClient::m_http_logging = false;

HttpClient::HttpClient(const std::string& uri) : m_ssl_ctx(boost::asio::ssl::context::tlsv12)
{
     //Set local database object
    m_ldb = &LocalDatabase::get_database();

        //Create new log obj
    m_log = &Log::get_log();

    set_defaults();

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
    m_timeout = boost::posix_time::seconds(15);
    m_verify_cert = true;
    m_connected = false;
    m_content_length = 0;
    m_stopped=true;

    //Set Max Transfer Speed (if defined)
    size_t db_max_speed = m_ldb->get_setting_int("max_transfer_speed");
    m_max_transfer_speed = (db_max_speed >= MIN_TRANSFER_SPEED) ?  db_max_speed : -1;
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
        if ( m_protocol == "http" ) {
            m_port = 80;
        }
        else if ( m_protocol == "https" ) {
            m_port = 443;
        }
        else {
            m_port = 443; //Default
        }
    }

    if ( m_protocol == "https" || m_port == 443 )
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

void HttpClient::init_deadline_timer()
{
    //Reset deadline timer
    m_deadline_timer.reset ( new boost::asio::deadline_timer(m_io_service) );
    m_stopped=false;
}

bool HttpClient::connect()
{

    std::cout << "Trying to connect to " << m_hostname << " on port " << m_port << "..." << '\n';

    m_io_service.stop();
    m_io_service.reset();

    //Create work object
    m_work.reset( new boost::asio::io_service::work(m_io_service) );

    //Reset sockets
    m_socket.reset( new boost::asio::ip::tcp::socket(m_io_service) );
    m_ssl_socket.reset( new boost::asio::ssl::stream<boost::asio::ip::tcp::socket>(m_io_service,m_ssl_ctx) );

    //Init deadline timer for timeout
    init_deadline_timer();

    //Clear connection status code
    m_conn_status.clear();

    //Set timeout
    m_deadline_timer->expires_from_now(m_timeout); //m_deadline_timer.expires_at(boost::posix_time::pos_infin);
    check_deadline(); //Handle connect timeout

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
    m_io_service.run();

    if ( !m_conn_status )
    {
        m_connected=true;
    }
    else
    {
        //throw boost::system::system_error( m_conn_status ? m_conn_status : boost::asio::error::operation_aborted );
        m_connected=false;
        m_log->add_error("Failed to connect to " + m_hostname, "HttpClient");
        throw HttpException(HttpException::ConnectFailed, std::string("Failed to connect to " + m_hostname) );
    }

    m_io_service.stop();
    m_io_service.reset();

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

        if ( disconnect_ec ) {
            m_log->add_error("Failed to disconnect from " + m_hostname, "HttpClient");
        }
    }
    catch ( const boost::system::system_error & e )
    {
        m_log->add_error( "ASIO Socket error: " + std::string(e.what()), "HttpClient");
        m_conn_status = e.code();
    }

    cancel_deadline();
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
            cancel_deadline();
            m_work.reset();
            return;
        }
        else //SSL client must perform handshake
        {
            m_ssl_good=false; //Reset flag
            m_ssl_socket->async_handshake(boost::asio::ssl::stream_base::client,boost::bind(&HttpClient::handle_handshake, this, boost::asio::placeholders::error) );
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

        //std::cout << "SSL Handshake successful" << "\n";

    }
    else
    {
        m_ssl_good=false;
        m_conn_status = e;
        m_log->add_error("ASIO SSL Handshake failed: " + e.message(), "HttpClient" );
        throw HttpException(HttpException::HandshakeFailed, e.message() );
    }

    cancel_deadline();
    m_work.reset();

}

void HttpClient::read_chunked_content( const boost::system::error_code& e, size_t bytes_transferred )
{

    if ( bytes_transferred <= 0 )
    {
        m_response_ec = boost::asio::error::eof;
        m_work.reset();
        return;
    }

    //std::cout << "Buffer size: " << m_response_buffer->size() << "; Bytes transferred: " << bytes_transferred << '\n';

    std::string chunk_sz_s;

    std::istream chunk_stream( m_response_buffer.get() );

    //Read the chunk size
    std::getline(chunk_stream, chunk_sz_s);

    //std::cout << "Chunk_sz_s: " << chunk_sz_s << '\n';

    //Remove carriage return
    boost::replace_all(chunk_sz_s, "\r", "");

    int chunk_sz = std::stoul(chunk_sz_s, nullptr, 16);

    //std::cout << "Read chunk size: " << chunk_sz << '\n';

    if ( chunk_sz <= 0 ) //There is no more data to read
    {
        m_response_ec = boost::asio::error::eof;
        m_work.reset();
        return;
    }

    //std::cout << "Response buffer after read: " << m_response_buffer->size() << '\n';

    int buffer_sz = m_response_buffer->size();
    int bytes_to_read = ( chunk_sz - buffer_sz ) + 2; //Add two for "\r\n" delimiter

    //std::cout << "Bytes to read: " << bytes_to_read << '\n';

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

    std::cout << "Reading response from server.." << '\n';

    if (!e)
    {

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
            m_work.reset();
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
        m_log->add_error("ASIO Response error: " + e.message(), "HttpClient");
        m_work.reset();
    }

}

void HttpClient::read_buffer_data()
{
    m_response_data.append( (std::istreambuf_iterator<char>( m_response_buffer.get() )), std::istreambuf_iterator<char>() );
}

void HttpClient::handle_read_headers( const boost::system::error_code& e )
{

    std::cout << "Reading response headers..." << '\n';

    if (!e)
    {
        //Read header data
        std::istream response_stream( m_response_buffer.get() );
        std::noskipws( response_stream ); //Don't skip white spaces
        std::string header;
        while (std::getline(response_stream, header) && header != "\r")
        {
            m_header_data += header.append("\n");

            //Add to header map
            auto pos = header.find_first_of(":");
            if ( pos != std::string::npos )
            {

                m_response_headers.insert( std::pair<std::string,std::string>( header.substr(0, pos), header.substr( pos+2) ) );
            }


        }

        //std::cout << "Read headers: " << '\n' << m_header_data << '\n';

        //Check for chunked transfer encoding
        if ( m_header_data.find("Transfer-Encoding: chunked") != std::string::npos ) {
            m_chunked_encoding=true;
        }
        else {
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

            //There may be some data in the buffer to consume
            if (m_response_buffer->size() > 0)
            {
                read_buffer_data();
            }

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
        m_log->add_error("ASIO Read Header Error: " + e.message(), "HttpClient");
        m_response_ec = e;
        m_work.reset();
    }

}

void HttpClient::handle_read_content( const boost::system::error_code& e, size_t bytes_transferred )
{

    std::cout << "Reading response body..." << '\n';

    if (!e || e == boost::asio::ssl::error::stream_truncated )
    {
        //Append response data
        read_buffer_data();

        // Continue reading remaining data until EOF.
        if ( m_use_ssl )
        {
            boost::asio::async_read(*m_ssl_socket, *m_response_buffer, boost::asio::transfer_at_least(1), boost::bind(&HttpClient::handle_read_content, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
            if ( e == boost::asio::ssl::error::stream_truncated ) {
                m_response_ec = boost::asio::error::eof;
                m_work.reset();
            }
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
        m_work.reset();
    }
    else
    {
        /** Response should be read at this point **/
        m_response_ec = boost::asio::error::eof;
        m_work.reset();
    }

}

void HttpClient::check_deadline()
{

    if ( m_stopped )
    {
        return;
    }

    // Check whether the deadline has passed. We compare the deadline against
    // the current time since a new asynchronous operation may have moved the
    // deadline before this actor had a chance to run.
    if (m_deadline_timer->expires_at() <= boost::asio::deadline_timer::traits_type::now())
    {

        //std::cout << "Deadline has expired!" << "\n";

        // The deadline has passed. The socket is closed so that any outstanding
        // asynchronous operations are cancelled. this allows the blocked
        // connect(), read_line() or write_line() functions to return.
        disconnect();
        m_work.reset();

        // There is no longer an active deadline. The expiry is set to positive
        // infinity so that the actor takes no action until a new deadline is set.
        m_deadline_timer->expires_at(boost::posix_time::pos_infin);

        std::cout << "Connection timed out: " << m_timeout.total_seconds() << " seconds" << '\n';

        return;

    }

    // Put the actor back to sleep.
    m_deadline_timer->async_wait(boost::bind(&HttpClient::check_deadline, this));

}

void HttpClient::cancel_deadline()
{
    m_stopped=true;
    m_deadline_timer->cancel();
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
    m_response_headers.clear();
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

void HttpClient::write_socket( const std::string& http_request )
{

    //Cleanup buffers / data
    cleanup();

    //Reset IO Service for next operation
    m_io_service.reset();

    //Create work object
    m_work.reset( new boost::asio::io_service::work(m_io_service) );

    std::cout << "Transferring " << http_request.size() << " bytes..." << '\n';

    //If Maximum Transfer Speed is > 0, throttle the transfer
    //Multiple writes are required
    if ( m_max_transfer_speed > 0 && (m_max_transfer_speed < http_request.size()) )
    {
        size_t bytes_to_read = (m_max_transfer_speed > http_request.size()) ? http_request.size() : m_max_transfer_speed;

        //Create a shared pointer of the request data to ensure lifetime through async calls
        m_request_data.reset();
        m_request_data = std::make_shared<std::string>(http_request);

        //Create a new token bucket for the duration of the transfer
        m_token_bucket.reset( new TokenBucket( m_max_transfer_speed, m_request_data->size() ) );

        if ( !m_use_ssl )
        {
            boost::asio::async_write(*m_socket, boost::asio::buffer(m_request_data->substr(0, bytes_to_read), bytes_to_read), boost::bind(&HttpClient::handle_write_throttled, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred ));
        }
        else
        {
            boost::asio::async_write(*m_ssl_socket, boost::asio::buffer(m_request_data->substr(0, bytes_to_read), bytes_to_read), boost::bind(&HttpClient::handle_write_throttled, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred ));
        }
    }
    else
    {

        //Send the entire request in one write

        if ( !m_use_ssl )
        {
            boost::asio::async_write(*m_socket, boost::asio::buffer(http_request), boost::bind(&HttpClient::handle_write, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred )) ;
        }
        else
        {
            boost::asio::async_write(*m_ssl_socket, boost::asio::buffer(http_request), boost::bind(&HttpClient::handle_write, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred )) ;
        }

    }
}

void HttpClient::handle_write_throttled( const boost::system::error_code& e, size_t bytes_transferred )
{

    m_token_bucket->transfer(bytes_transferred);
    double transfer_rate = m_token_bucket->transfer_rate();
    size_t total_bytes_transferred = m_token_bucket->bytes_transferred();
    size_t bytes_remaining = m_request_data->size() - total_bytes_transferred;

    std::cout.precision(2);
    std::cout << "Wrote " << bytes_transferred << " bytes" << '\n';
    std::cout << "Total Request Size " << m_token_bucket->total_bytes() << " bytes" << '\n';
    std::cout << "Total Bytes Transferred " << total_bytes_transferred << " bytes" << '\n';
    std::cout << "Transfer rate: " << std::fixed << transfer_rate << "bps" << '\n';
    std::cout << "Bytes Remaining: " << bytes_remaining << '\n';
    std::cout << "Estimated Time Remaining: " << ((bytes_remaining*1.0)/transfer_rate) << " seconds" << '\n';

    if ( !e )
    {
        if ( m_token_bucket->bytes_transferred() >= m_token_bucket->total_bytes() )
        {
            //Request has been sent, read response
            m_request_data.reset(); //Free memory

            if ( !m_use_ssl )
            {
                boost::asio::async_read_until(*m_socket, *m_response_buffer, "\r\n", boost::bind(&HttpClient::handle_response, this, boost::asio::placeholders::error));
            }
            else
            {
                boost::asio::async_read_until(*m_ssl_socket, *m_response_buffer, "\r\n", boost::bind(&HttpClient::handle_response, this, boost::asio::placeholders::error));
            }

        }
        else
        {
            //Determine the amount of time we need to sleep to meet max_transfer_speed
            if ( transfer_rate > m_max_transfer_speed )
            {
                double sleep_time_secs = ( (transfer_rate*1.0) / (m_token_bucket->max_transfer_speed()*1.0) ); //Add some padding
                long sleep_time_ms = sleep_time_secs * 1000;
                std::cout << "Bandwidth has been exceeded - limiting transfer speed - waiting " << sleep_time_secs << " seconds..." << '\n';
                boost::this_thread::sleep_for( boost::chrono::milliseconds(sleep_time_ms) );
            }

            size_t bytes_to_read = ((total_bytes_transferred + m_max_transfer_speed) >= m_request_data->size()) ? bytes_remaining : m_max_transfer_speed;

            if ( !m_use_ssl )
            {
                boost::asio::async_write(*m_socket, boost::asio::buffer(m_request_data->substr(total_bytes_transferred, bytes_to_read), bytes_to_read), boost::bind(&HttpClient::handle_write_throttled, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred ));
            }
            else
            {
                boost::asio::async_write(*m_ssl_socket, boost::asio::buffer(m_request_data->substr(total_bytes_transferred, bytes_to_read), bytes_to_read), boost::bind(&HttpClient::handle_write_throttled, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred ));
            }

        }

    }
    else
    {
        m_log->add_error("ASIO Write Error: " + e.message(), "HttpClient");
        m_response_ec = e;
        m_work.reset(); //No more work
    }
}

void HttpClient::handle_write( const boost::system::error_code& e, size_t bytes_transferred )
{

    std::cout << "Sent " << bytes_transferred << " bytes..." << '\n';
    //std::cout << "Buffer size: " << m_request_buffer->size() << " bytes" << '\n';

    if (!e)
    {

        if ( !m_use_ssl )
        {
            boost::asio::async_read_until(*m_socket, *m_response_buffer, "\r\n", boost::bind(&HttpClient::handle_response, this, boost::asio::placeholders::error));
        }
        else
        {
            boost::asio::async_read_until(*m_ssl_socket, *m_response_buffer, "\r\n", boost::bind(&HttpClient::handle_response, this, boost::asio::placeholders::error));
        }

    }
    else
    {
        m_log->add_error("ASIO Write Error: " + e.message(), "HttpClient");
        m_response_ec = e;
        m_work.reset(); //No more work
    }

}

void HttpClient::run_io_service()
{
    m_io_service.run();
}

boost::system::error_code HttpClient::get_error_code()
{
    return m_response_ec;
}

bool HttpClient::is_connected()
{
    return m_connected;
}

void HttpClient::cleanup()
{
    //Refresh buffer(s)
    m_response_buffer.reset( new boost::asio::streambuf() );
    clear_response();
    clear_headers();
    m_content_length=0;
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
        if ( isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~' || c == '/') {
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
    m_content_length = request.get_body_length();

    //Build the HTTP Request
    http_stream << http_method << " " << request.get_url() << " HTTP/1.1\r\n";
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

    //std::cout << "Sending request:" << '\n' << http_stream.str() << '\n';

    if ( send_body ) {
        http_stream << request.get_body();
    }

    //If client is already connected, disconnect before a new attempt
    if ( is_connected() ) {
        disconnect();
    }

    //Connect to server
    connect();

    //Write HTTP request to socket
    write_socket(http_stream.str());

    //Run the handlers until the response sets the status code or EOF
    run_io_service();

    disconnect();

    unsigned int http_status = get_http_status();

    if ( m_http_logging )
    {
        //Truncate logs > 16kb
        m_log->add_http_message( ((http_stream.str().size() > 16000) ? http_stream.str().substr(0, 16000) : http_stream.str()), get_response(), http_status );
    }

    return http_status;

}

bool HttpClient::http_logging()
{
    return m_http_logging;
}

void HttpClient::http_logging(bool flag)
{
    m_http_logging = flag;
}

std::string HttpClient::get_header(const std::string& key)
{
    if ( m_response_headers.find(key) != m_response_headers.end() )
    {
        return m_response_headers[key];
    }

    return "";
}

size_t HttpClient::get_content_length()
{
    return m_content_length;
}

int HttpClient::get_port()
{
    return m_port;
}

std::string HttpClient::make_test_str(size_t length)
{

    std::string data;
    std::string pattern = "0";

    for ( size_t i=0; i < length; i++ )
    {
        data.append(pattern);
    }

    return data;

}

void HttpClient::max_transfer_speed(size_t limit)
{
    m_max_transfer_speed = (limit > 0) ? limit : -1;
}
