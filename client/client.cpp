#include "client.hpp"

using boost::asio::ip::tcp;
using boost::asio::deadline_timer;
using namespace Backup::Networking;

Client::Client( const std::string& host ) :
    m_socket(m_io_service),
    m_deadline_timer(m_io_service),
    m_use_ssl(false),
    m_ssl_ctx(ssl::context::tlsv12),
    m_ssl_socket(m_io_service,m_ssl_ctx),
    m_timeout(boost::posix_time::seconds(5)),
    m_verify_cert(true)
{

    //Set SSL Opts
    m_ssl_ctx.set_default_verify_paths();

    if ( m_verify_cert )
        m_ssl_socket.set_verify_mode(ssl::verify_peer);
    else
        m_ssl_socket.set_verify_mode(ssl::verify_none);

    //Determine protocol, hostname, etc
    this->parse_url(host);

    //Timer should not expire until a connection attempt is made
    m_deadline_timer.expires_at(boost::posix_time::pos_infin);
    this->check_deadline();

}

Client::~Client()
{

    if ( m_socket.is_open() )
        m_socket.close();

    if ( m_ssl_socket.lowest_layer().is_open() )
        m_ssl_socket.lowest_layer().close();

}

void Client::parse_url( const std::string& host )
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

        m_ssl_socket.set_verify_callback(ssl::rfc2818_verification(m_hostname));
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

    std::cout << "HTTP Protocol: " << m_protocol << '\n';
    std::cout << "Hostname: " << m_hostname << '\n';
    std::cout << "URI: " << m_uri << '\n';

}

void Client::set_timeout( boost::posix_time::time_duration t )
{
    m_timeout = t;
}

void Client::set_ssl(bool f)
{
    m_use_ssl = f;
}

bool Client::connect()
{

    m_conn_status.clear();

    //Set timeout
    m_deadline_timer.expires_from_now(m_timeout);

    try
    {

        tcp::resolver resolver(m_io_service);
        tcp::resolver::query query( m_hostname.c_str(), m_protocol.c_str() );
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

        //Block the call to ASYNC connect until the error code changes
        m_conn_status = boost::asio::error::would_block;

        if ( !m_use_ssl )
            boost::asio::async_connect(m_socket, endpoint_iterator, boost::bind(&Client::handle_connect, this, boost::asio::placeholders::error) );
        else
            boost::asio::async_connect(m_ssl_socket.lowest_layer(), endpoint_iterator, boost::bind(&Client::handle_connect, this, boost::asio::placeholders::error) );

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

void Client::handle_connect(const boost::system::error_code& e)
{

    if ( !e )
    {
        std::cout << "Connected to " << m_hostname << "!" << std::endl;
        std::cout << "Error Msg: " << e.message() << std::endl;

        if ( !m_use_ssl )
        {
            m_conn_status = e;
            return;
        }
        else //SSL client must perform handshake
        {
            m_ssl_socket.async_handshake(ssl::stream_base::client,boost::bind(&Client::handle_handshake, this, boost::asio::placeholders::error) );

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

void Client::handle_handshake(const boost::system::error_code& e )
{

    if (!e)
    {
        m_ssl_good=true;
        std::cout << "SSL Handshake successful" << "\n";
        m_conn_status = e;
        /*
        std::cout << "Enter message: ";
        std::cin.getline(request_, max_length);
        size_t request_length = strlen(request_);

        boost::asio::async_write(
            m_ssl_socket,
            boost::asio::buffer(***, request_length),
            boost::bind(&Client::handle_write, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)
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

void Client::handle_write( const boost::system::error_code& e )
{

    std::cout << "Handle write..." << '\n';

    if (!e)
    {
        // Read the response status line. The response_ streambuf will
        // automatically grow to accommodate the entire line. The growth may be
        // limited by passing a maximum size to the streambuf constructor.
        if ( !m_use_ssl )
            boost::asio::async_read_until(m_socket, m_response, "\r\n", boost::bind(&Client::handle_response, this, boost::asio::placeholders::error));
        else
            boost::asio::async_read_until(m_ssl_socket, m_response, "\r\n", boost::bind(&Client::handle_response, this, boost::asio::placeholders::error));

    }
    else
    {
        std::cout << "Error: " << e.message() << "\n";
    }

}

void Client::handle_response( const boost::system::error_code& e )
{

    m_response_ec = e; //Update error code

    std::cout << "Handled response.." << '\n';

    if (!e)
    {
        // Check that response is OK.
        std::istream response_stream(&m_response);
        std::string http_version;
        response_stream >> http_version;
        unsigned int status_code;
        response_stream >> status_code;
        std::string status_message;
        std::getline(response_stream, status_message);

        if (!response_stream || http_version.substr(0, 5) != "HTTP/")
        {
            std::cout << "Invalid response\n";
            m_response_ec = boost::asio::error::operation_aborted; //Handle statis error
            return;
        }

        //Handle a possible redirect
        if ( status_code == 301 )
        {
            //std::cout << "Response: \n" << response_stream.rdbuf() << '\n';

            std::string new_location;

            //Find redirect location
            std::string tmp_response;
            while ( std::getline(response_stream, tmp_response) )
            {

                if ( tmp_response.find("Location:") != std::string::npos )
                {
                    /*
                    response_stream.ignore(9, ' '); //Location:
                    response_stream >> new_location;
                    */
                    new_location = tmp_response.substr(10);
                    break;
                }
            }

            std::cout << "Request is being redirected to: " << new_location << '\n';

            //Close Open Socket
            this->disconnect();

            //Try to reconnect to new host
            this->parse_url(new_location.c_str());
            this->connect();

            return;
        }
        else if (status_code != 200)
        {
            std::cout << "Response returned with status code ";
            std::cout << status_code << "\n";
            m_response_ec = boost::asio::error::operation_aborted; //Set static error
            return;
        }

        // Read the response headers, which are terminated by a blank line.
        if ( !m_use_ssl )
            boost::asio::async_read_until(m_socket, m_response, "\r\n\r\n", boost::bind(&Client::handle_read_headers, this, boost::asio::placeholders::error));
        else
            boost::asio::async_read_until(m_ssl_socket, m_response, "\r\n\r\n", boost::bind(&Client::handle_read_headers, this, boost::asio::placeholders::error));

    }
    else
    {
        std::cout << "Error: " << e << "\n";
    }

}

void Client::handle_read_headers( const boost::system::error_code& e )
{

    std::cout << "Handle read headers..." << '\n';

    if (!e)
    {
        // Process the response headers.
        std::istream response_stream(&m_response);
        std::string header;
        while (std::getline(response_stream, header) && header != "\r")
            std::cout << header << "\n";

        std::cout << "\n";

        // Write whatever content we already have to output.
        if (m_response.size() > 0)
            std::cout << &m_response;

        // Start reading remaining data until EOF.
        if ( !m_use_ssl )
            boost::asio::async_read( m_socket, m_response, boost::asio::transfer_at_least(1), boost::bind(&Client::handle_read_content, this, boost::asio::placeholders::error) );
        else
            boost::asio::async_read( m_ssl_socket, m_response, boost::asio::transfer_at_least(1), boost::bind(&Client::handle_read_content, this, boost::asio::placeholders::error) );

    }
    else
    {
        std::cout << "Error: " << e << "\n";
    }

    m_response_ec = e;

}

void Client::handle_read_content( const boost::system::error_code& e )
{

    std::cout << "Read some content" << '\n';

    if (!e)
    {
        // Write all of the data that has been read so far.
        std::cout << &m_response;

        // Continue reading remaining data until EOF.
        if ( !m_use_ssl )
            boost::asio::async_read(m_socket, m_response, boost::asio::transfer_at_least(1), boost::bind(&Client::handle_read_content, this, boost::asio::placeholders::error));
        else
            boost::asio::async_read(m_ssl_socket, m_response, boost::asio::transfer_at_least(1), boost::bind(&Client::handle_read_content, this, boost::asio::placeholders::error));

    }
    else if (e != boost::asio::error::eof)
    {
        std::cout << "Error: " << e << "\n";
        m_response_ec = boost::asio::error::eof;
    }
    else
        m_response_ec = boost::asio::error::eof;

}

void Client::check_deadline()
{
    // Check whether the deadline has passed. We compare the deadline against
    // the current time since a new asynchronous operation may have moved the
    // deadline before this actor had a chance to run.
    if (m_deadline_timer.expires_at() <= deadline_timer::traits_type::now())
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
    m_deadline_timer.async_wait(boost::bind(&Client::check_deadline, this));

}

void Client::disconnect()
{
    if ( m_socket.is_open() ) {
        m_socket.shutdown( tcp::socket::shutdown_both );
        m_socket.close();
    }

    if ( m_ssl_socket.lowest_layer().is_open() ) {
        m_ssl_socket.lowest_layer().shutdown( tcp::socket::shutdown_both );
        m_ssl_socket.lowest_layer().close();
    }

}

void Client::send_request( const Backup::Networking::HttpRequest& r )
{

    m_deadline_timer.expires_from_now(boost::posix_time::seconds(15));

    //Build the HTTP Request
    std::ostringstream request_stream;
    request_stream << r.get_method() << " " << r.get_uri() << " HTTP/1.1\r\n";
    request_stream << "Host: " << m_hostname << "\r\n";
    request_stream << "Accept: */*\r\n";

    //If POST or PUT, send content length and type headers
    std::string http_method = r.get_method();
    bool do_send_data=false;
    if ( http_method == "POST" || http_method == "PUT" )
    {
        request_stream << "Content-Length: " << r.get_body_length() << "\r\n";
        request_stream << "Content-Type: " << r.get_content_type() << "\r\n";
        do_send_data=true;
    }

    request_stream << "Connection: close\r\n\r\n";

    if ( do_send_data )
        request_stream << r.get_body().c_str();

    //Clear status code before sending new data
    m_response_ec.clear();

    if ( !m_use_ssl )
        boost::asio::async_write(m_socket, boost::asio::buffer(request_stream.str().c_str(), request_stream.str().size()), boost::bind(&Client::handle_write, this, boost::asio::placeholders::error)) ;
    else
        boost::asio::async_write(m_ssl_socket, boost::asio::buffer(request_stream.str().c_str(), request_stream.str().size()), boost::bind(&Client::handle_write, this, boost::asio::placeholders::error)) ;

    //Run the handlers until the response sets the status code or EOF
    do { m_io_service.run_one(); std::cout << "..." << '\n'; } while ( !m_response_ec );

}

bool Client::upload_file_single( const Backup::Types::http_upload_file& f )
{

    //Create the raw JSON payload for the request
    std::string request = make_upload_json(f);

    //Create a new HTTP request
    HttpRequest r;
    r.set_content_type("application/json");
    r.set_body("");
    r.set_method("POST");
    r.set_uri("/api/v1/file");
    //r.add_header("");

    this->send_request(r);

}

std::string Client::make_upload_json( const Backup::Types::http_upload_file& f )
{

    rapidjson::Document doc;
    doc.SetObject();
    rapidjson::Document::AllocatorType& alloc = doc.GetAllocator();

    //Build backup_file object
    rapidjson::Value backup_file(rapidjson::kObjectType);
    {
        backup_file.AddMember("file_name", "test", alloc );
        backup_file.AddMember("file_size", 1000000, alloc );
        backup_file.AddMember("last_modified", 10000000, alloc );
        backup_file.AddMember("parent_path", "test", alloc );
    }

    //Add backup_file object to JSON
    doc.AddMember("backup_file", backup_file, alloc );

    rapidjson::StringBuffer strbuf;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(strbuf);

    doc.Accept(writer);

    std::cout << "Example JSON:\n" << strbuf.GetString() << '\n';

    return strbuf.GetString();

}

bool Client::heartbeat()
{

    using namespace Backup::Database;

    //Create JSON for client heartbeat
    rapidjson::Document doc;
    doc.SetObject();
    rapidjson::Document::AllocatorType& alloc = doc.GetAllocator();

    LocalDatabase* ldb = &LocalDatabase::getDatabase();

    std::map<std::string,std::string> jmap;
    jmap.insert( std::pair<std::string,std::string>("hostname", ldb->get_setting_str("hostname")) );
    jmap.insert( std::pair<std::string,std::string>("os", ldb->get_setting_str("host_os")) );
    jmap.insert( std::pair<std::string,std::string>("client_version", ldb->get_setting_str("client_version")) );
    jmap.insert( std::pair<std::string,std::string>("domain", ldb->get_setting_str("host_domain")) );

    for ( auto &kv : jmap )
    {
        rapidjson::Value key(kv.first, alloc);
        rapidjson::Value value(kv.second, alloc);

        doc.AddMember(key, value, alloc );
    }

    rapidjson::StringBuffer strbuf;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(strbuf);

    doc.Accept(writer);

    HttpRequest r;
    r.set_content_type("application/json");
    r.set_method("POST");
    r.set_uri("/api/v1/heartbeat");
    r.set_body( strbuf.GetString() );

    this->send_request(r);

    return true;
}
