#include "client.hpp"

using boost::asio::ip::tcp;
using boost::asio::deadline_timer;
using namespace Backup::Networking;

BackupClient::BackupClient( const std::string& host ) :
    m_socket(m_io_service),
    m_deadline_timer(m_io_service),
    m_use_ssl(false),
    m_ssl_ctx(ssl::context::tlsv12),
    m_ssl_socket(m_io_service,m_ssl_ctx),
    m_timeout(boost::posix_time::seconds(5)),
    m_verify_cert(true)
{

    //Create new log obj
    m_log = new Log("asio");

    //Set local database object
    m_ldb = &Backup::Database::LocalDatabase::get_database();

    //Set SSL Opts
    m_ssl_ctx.set_default_verify_paths();

    if ( m_verify_cert )
        m_ssl_socket.set_verify_mode(ssl::verify_peer);
    else
        m_ssl_socket.set_verify_mode(ssl::verify_none);

    //Determine protocol, hostname, etc
    this->parse_url(host);

    //Get user information
    this->m_auth_token = m_ldb->get_setting_str("auth_token");
    this->m_user_id = m_ldb->get_setting_int("user_id");

    //Create the authorization header used for API requests
    this->m_auth_header = get_auth_header(m_auth_token, m_user_id);

    //Timer should not expire until a connection attempt is made
    m_deadline_timer.expires_at(boost::posix_time::pos_infin);
    this->check_deadline();

}

BackupClient::~BackupClient()
{

    if ( m_socket.is_open() )
        m_socket.close();

    if ( m_ssl_socket.lowest_layer().is_open() )
        m_ssl_socket.lowest_layer().close();

}

void BackupClient::parse_url( const std::string& host )
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

    /*
    std::cout << "HTTP Protocol: " << m_protocol << '\n';
    std::cout << "Hostname: " << m_hostname << '\n';
    std::cout << "URI: " << m_uri << '\n';
    */

}

void BackupClient::set_timeout( boost::posix_time::time_duration t )
{
    m_timeout = t;
}

void BackupClient::set_ssl(bool f)
{
    m_use_ssl = f;
}

bool BackupClient::connect()
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
            boost::asio::async_connect(m_socket, endpoint_iterator, boost::bind(&BackupClient::handle_connect, this, boost::asio::placeholders::error) );
        else
            boost::asio::async_connect(m_ssl_socket.lowest_layer(), endpoint_iterator, boost::bind(&BackupClient::handle_connect, this, boost::asio::placeholders::error) );

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

void BackupClient::handle_connect(const boost::system::error_code& e)
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
            m_ssl_socket.async_handshake(ssl::stream_base::client,boost::bind(&BackupClient::handle_handshake, this, boost::asio::placeholders::error) );

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

void BackupClient::handle_handshake(const boost::system::error_code& e )
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

void BackupClient::handle_write( const boost::system::error_code& e )
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
            boost::asio::async_read_until(m_socket, m_response_buffer, "\r\n", boost::bind(&BackupClient::handle_response, this, boost::asio::placeholders::error));
        else
            boost::asio::async_read_until(m_ssl_socket, m_response_buffer, "\r\n", boost::bind(&BackupClient::handle_response, this, boost::asio::placeholders::error));

    }
    else
    {
        std::cout << "ASIO Write Error: " << e.message() << "\n";
    }

}

void BackupClient::handle_response( const boost::system::error_code& e )
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
            boost::asio::async_read_until(m_socket, m_response_buffer, "\r\n\r\n", boost::bind(&BackupClient::handle_read_headers, this, boost::asio::placeholders::error));
        else
            boost::asio::async_read_until(m_ssl_socket, m_response_buffer, "\r\n\r\n", boost::bind(&BackupClient::handle_read_headers, this, boost::asio::placeholders::error));

    }
    else
    {
        std::cout << "ASIO Response Error: " << e << "\n";
    }

}

void BackupClient::handle_read_headers( const boost::system::error_code& e )
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
            boost::asio::async_read( m_socket, m_response_buffer, boost::asio::transfer_at_least(1), boost::bind(&BackupClient::handle_read_content, this, boost::asio::placeholders::error) );
        else
            boost::asio::async_read( m_ssl_socket, m_response_buffer, boost::asio::transfer_at_least(1), boost::bind(&BackupClient::handle_read_content, this, boost::asio::placeholders::error) );

    }
    else
    {
        std::cout << "ASIO Read Header Error: " << e << "\n";
    }

    m_response_ec = e;

}

void BackupClient::handle_read_content( const boost::system::error_code& e )
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
            boost::asio::async_read(m_socket, m_response_buffer, boost::asio::transfer_at_least(1), boost::bind(&BackupClient::handle_read_content, this, boost::asio::placeholders::error));
        else
            boost::asio::async_read(m_ssl_socket, m_response_buffer, boost::asio::transfer_at_least(1), boost::bind(&BackupClient::handle_read_content, this, boost::asio::placeholders::error));

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

void BackupClient::check_deadline()
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
    m_deadline_timer.async_wait(boost::bind(&BackupClient::check_deadline, this));

}

void BackupClient::disconnect()
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

void BackupClient::send_request( Backup::Networking::HttpRequest* r )
{

    //If client is already connected, disconnect before a new attempt
    if ( m_connected )
        this->disconnect();

    //Connect to server
    this->connect();

    if ( !m_connected )
        return;

    //clear any existing data
    m_response_data.clear();
    m_header_data.clear();

    m_deadline_timer.expires_from_now(boost::posix_time::seconds(15));

    //Build the HTTP Request
    std::stringstream request_stream(std::stringstream::out | std::stringstream::binary);
    request_stream << r->get_method() << " " << r->get_uri() << " HTTP/1.1\r\n";
    request_stream << "Host: " << m_hostname << "\r\n";
    request_stream << "Accept: */*\r\n";

    //Send Authorization Header
    if ( this->m_auth_token != "" ) {
        request_stream << "Authorization: " << this->m_auth_header << "\r\n";
    }

    std::string http_method = r->get_method();
    std::string content_type = r->get_content_type();

    bool do_send_data=false; /* Flag which indicates we want to send raw data */

    //If POST or PUT, send content length and type headers
    if ( http_method == "POST" || http_method == "PUT" )
    {
        request_stream << "Content-Length: " << r->get_body_length() << "\r\n";

        if ( !content_type.empty() )
            request_stream << "Content-Type: " << content_type << "\r\n";

        do_send_data=true;
    }

    //Add any custom headers
    std::vector<std::string> headers = r->get_headers();
    for ( std::vector<std::string>::iterator itr = headers.begin(); itr != headers.end(); ++itr )
        request_stream << *itr << "\r\n";

    //Close connection after response
    request_stream << "Connection: close\r\n\r\n";

    //request_stream << "\r\n";

    if ( do_send_data )
        request_stream.write( &r->get_body()[0], r->get_body_length() );

    //Clear status code before sending new data
    m_response_ec.clear();

    std::string data = request_stream.str();

    if ( !m_use_ssl )
        boost::asio::async_write(m_socket, boost::asio::buffer( data, request_stream.str().size() ), boost::bind(&BackupClient::handle_write, this, boost::asio::placeholders::error)) ;
    else
        boost::asio::async_write(m_ssl_socket, boost::asio::buffer( &request_stream.str()[0], request_stream.str().size() ), boost::bind(&BackupClient::handle_write, this, boost::asio::placeholders::error)) ;

    //Run the handlers until the response sets the status code or EOF
    do { m_io_service.run_one(); std::cout << "..." << '\n'; } while ( !m_response_ec );

    this->disconnect();

    /**
     ** Handle HTTP Error codes
     ** The 401 handler is designed to perform a user activation or token refresh on the fly in the event of authorization failures
    **/
    if (m_http_status == 401)
        handle_auth_error();
    else if ( m_http_status > 200 ) {
        m_log->add_message("There was an error returned by the server. Status code: " + std::to_string(m_http_status), "ASIO" );
        handle_api_error();
    }

}

int BackupClient::init_upload ( Backup::File::BackupFile * bf )
{

    Document doc;
    doc.SetObject();
    Document::AllocatorType& alloc = doc.GetAllocator();

    std::map<std::string,Value> jmap;

    //Get Activation Code from DB
    jmap.insert( std::pair<std::string,Value>( "file_name", Value( bf->get_file_name().c_str(), alloc ) ) );
    jmap.insert( std::pair<std::string,Value>( "file_size", Value( bf->get_file_size() ) ) );
    jmap.insert( std::pair<std::string,Value>( "file_type", Value( bf->get_file_type().c_str(), alloc ) ) );
    jmap.insert( std::pair<std::string,Value>( "hash", Value( bf->get_hash().c_str(), alloc ) ) );
    jmap.insert( std::pair<std::string,Value>( "file_path", Value( bf->get_parent_path().c_str(), alloc ) ) );
    jmap.insert( std::pair<std::string,Value>( "last_modified", Value( (uint64_t)bf->get_last_modified() ) ) );
    jmap.insert( std::pair<std::string,Value>( "parts", Value( bf->get_total_parts() ) ) );
    jmap.insert( std::pair<std::string,Value>( "compressed", Value( m_use_compression ) ) );

        //Add values to document object
    for ( auto &kv : jmap )
    {
        doc.AddMember(Value().SetString(kv.first.c_str(),alloc), kv.second, alloc );
    }

    //Write object to buffer
    StringBuffer strbuf;
    PrettyWriter<StringBuffer> writer(strbuf);

    doc.Accept(writer);

    //Create a new HTTP request
    HttpRequest r;
    r.add_header("Content-Type: application/json");
    r.set_body(strbuf.GetString());
    r.set_method("POST");
    r.set_uri("/backup/api/v1/file?action=init");

    //std::cout << "Example body:\n" << r.get_body() << std::endl;

    this->send_request(&r);

    //Parse response and get upload id
    std::cout << "Init upload response: " << m_response_data << std::endl;

    //Reset Document
    doc.RemoveAllMembers();
    strbuf.Clear();
    writer.Reset(strbuf);

    ParseResult json_ok = doc.Parse( m_response_data.c_str() );

    if ( !json_ok )
    {
        handle_json_error(json_ok);
        return -1;
    }

    doc.Accept(writer);

    //Default to -1
    int upload_id = -1;

    std::cout << "Response: \n" << m_response_data << std::endl;

    if ( m_http_status == 200 ) {

        if ( !doc.HasMember("response") ) {
            set_error("There was an error parsing the JSON response");
            return -1;
        }

        //Check if access token is present
        const Value& response = doc["response"];

        if ( response.HasMember("upload_id") )
            upload_id = response["upload_id"].GetInt();
        else
            set_error("There was an error parsing the JSON response");

    }

    return upload_id;

}

void BackupClient::set_error( const std::string& msg )
{
    m_error_message = msg;
}

std::string BackupClient::get_error()
{
    return m_error_message;
}

bool BackupClient::upload_file_part( Backup::File::BackupFile * bf, int part_number=1 )
{

    //Validate upload id
    if ( bf->get_upload_id() < 0 )
        return false;

    //Build a JSON payload of the file properties/metadata
    Document doc;
    doc.SetObject();
    Document::AllocatorType& alloc = doc.GetAllocator();

    std::map<std::string,Value> jmap;

    std::string file_part;

    if ( m_use_compression )
    {
        auto bfc = bf->get_compressed_copy();
        bfc->set_chunk_size( bf->get_chunk_size() );
        file_part = bfc->get_file_part(part_number);
    }
    else
    {
        file_part = bf->get_file_part(part_number);
    }

    //Get Activation Code from DB
    jmap.insert( std::pair<std::string,Value>( "file_id", Value( bf->get_unique_id().c_str(), alloc ) ) );
    jmap.insert( std::pair<std::string,Value>( "part_number", Value( part_number) ) );
    jmap.insert( std::pair<std::string,Value>( "part_size", Value( file_part.size() ) ) );
    jmap.insert( std::pair<std::string,Value>( "hash", Value( bf->get_hash(file_part).c_str(), alloc ) ) );
    jmap.insert( std::pair<std::string,Value>( "compressed", Value( m_use_compression ) ) );

    //Add values to document object
    for ( auto &kv : jmap )
    {
        doc.AddMember(Value().SetString(kv.first.c_str(),alloc), kv.second, alloc );
    }

    //Write object to buffer
    StringBuffer strbuf;
    PrettyWriter<StringBuffer> writer(strbuf);

    doc.Accept(writer);

    //Build the request body
    std::stringstream body;

    //Write a boundary
    body << "\r\n--BackupFile\r\n";
    body << "Content-Disposition: form-data; name=\"metadata\"\r\n\r\n";

    //Write JSON payload
    body << strbuf.GetString();

    body << "\r\n";

    //Write a boundary
    body << "--BackupFile\r\n";
    body << "Content-Disposition: form-data; name=\"fileContent\"\r\n\r\n";

    //Write file data

    body.write( &file_part[0], file_part.size() );

    body << "\r\n";

    //Write a boundary
    body << "--BackupFile--\r\n\r\n";

    //std::cout << "Body:\n" << body.str() << std::endl;

    //Create a new HTTP request
    HttpRequest r;
    r.add_header("Content-Disposition: multipart/form-data");
    r.add_header("Content-Type: multipart/form-data; boundary=BackupFile");
    r.set_body(body.str());
    r.set_method("POST");
    r.set_uri("/backup/api/v1/file");

    //std::cout << "Example body:\n" << r.get_body() << std::endl;

    this->send_request(&r);

    std::cout << "File part response: " << m_response_data << std::endl;

    return true;

}

bool BackupClient::heartbeat()
{

    //Create JSON for client heartbeat
    rapidjson::Document doc;
    doc.SetObject();
    rapidjson::Document::AllocatorType& alloc = doc.GetAllocator();

    std::map<std::string,std::string> jmap;
    jmap.insert( std::pair<std::string,std::string>("host_name", m_ldb->get_setting_str("hostname")) );
    jmap.insert( std::pair<std::string,std::string>("os", m_ldb->get_setting_str("host_os")) );
    jmap.insert( std::pair<std::string,std::string>("client_version", m_ldb->get_setting_str("client_version")) );
    jmap.insert( std::pair<std::string,std::string>("domain", m_ldb->get_setting_str("host_domain")) );

    for ( auto &kv : jmap )
    {
        rapidjson::Value key(kv.first.c_str(), alloc);
        rapidjson::Value value(kv.second.c_str(), alloc);
        doc.AddMember(key, value, alloc );
    }

    rapidjson::StringBuffer strbuf;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(strbuf);

    doc.Accept(writer);

    HttpRequest r;
    r.set_content_type("application/json");
    r.set_method("POST");
    r.set_uri("/backup/api/v1/heartbeat");
    r.set_body( strbuf.GetString() );

    this->send_request(&r);

    return true;

}

std::string BackupClient::get_response()
{
    return m_response_data;
}

std::string BackupClient::get_headers()
{
    return m_header_data;
}

std::string BackupClient::get_client_settings()
{

    if ( m_auth_token == "" ) //Don't try to get settings if no auth token is present
        return "{}";

    HttpRequest r;
    r.set_content_type("application/json");
    r.set_method("GET");
    r.set_uri("/backup/api/v1/settings");

    this->send_request(&r);

    return this->get_response();

}

bool BackupClient::activate()
{

    Document doc;
    doc.SetObject();
    Document::AllocatorType& alloc = doc.GetAllocator();

    //Get Activation Code from DB
    Value activation_code( m_ldb->get_setting_str("activation_code").c_str(), alloc );
    Value user_name( m_ldb->get_setting_str("username").c_str(), alloc );

    doc.AddMember("user_name", user_name, alloc );
    doc.AddMember("activation_code", activation_code, alloc );

    StringBuffer strbuf;
    PrettyWriter<StringBuffer> writer(strbuf);

    doc.Accept(writer);

    HttpRequest r;
    r.set_method("POST");
    r.set_uri("/backup/api/v1/activate");
    r.set_content_type("application/json");
    r.set_body( strbuf.GetString() );

    this->send_request( &r );

    //Reset Document
    strbuf.Clear();
    doc.RemoveAllMembers();
    writer.Reset(strbuf);

    std::cout << "Activation response: " << m_response_data << std::endl;

    ParseResult json_ok = doc.Parse( m_response_data.c_str() );

    if ( !json_ok )
    {
        handle_json_error(json_ok);
        return false;
    }

    doc.Accept(writer);

    //Check if access token is present
    const Value& response = doc["response"];

    if ( !response.HasMember("access_token") ) {
        m_activated=false;
        return false;
    }

    //Set auth token
    std::string token = response["access_token"].GetString();

    //If the access token is empty, and the flag is false, activation failed
    if ( response["is_activated"].GetBool() ) {
        m_activated=true;
    }

    //Set Auth Token?
    if ( token != "" )
    {
        //Update DB w/ new token and set member var
        m_ldb->update_setting("auth_token", token );
        m_auth_token = token;

         //Set DB User ID provided w/ user activation
        if ( response.HasMember("user_id") )
        {
            int user_id = response["user_id"].GetInt();
            if ( user_id > 0 )
            {
                m_ldb->update_setting("user_id", user_id );
                m_user_id = user_id;
            }
        }

        //Generate a new authorization header for subsequent requests
        m_auth_header = get_auth_header(token, m_user_id);

    }

    //Set Refresh token
    if ( response.HasMember("refresh_token") )
    {
        m_ldb->update_setting("refresh_token", response["refresh_token"].GetString() );
    }

    return m_activated;

}

bool BackupClient::is_activated()
{
    return m_activated;
}

unsigned int BackupClient::get_http_status()
{
    return m_http_status;
}

void BackupClient::use_compression(bool flag)
{
    m_use_compression=flag;
}

bool BackupClient::refresh_token()
{

    //Create new JSON document
    Document doc;
    doc.SetObject();

    //Get document allocator
    Document::AllocatorType& alloc = doc.GetAllocator();

    //Get refresh token from LocalDatabase
    std::string refresh_token = m_ldb->get_setting_str("refresh_token");

    //Get user ID from LocalDatabase
    int user_id = m_ldb->get_setting_int("user_id");

    //Prepare JSON request
    Value refresh_token_v( refresh_token.c_str(), refresh_token.size(), alloc );
    Value user_id_v( user_id );

    doc.AddMember( "refresh_token", refresh_token_v, alloc );
    doc.AddMember( "user_id", user_id_v, alloc );

    //Write JSON
    StringBuffer strbuf;
    PrettyWriter<StringBuffer> writer(strbuf);
    doc.Accept(writer);

    //Send Http Request
    HttpRequest r;
    r.set_method("POST");
    r.set_uri("/backup/api/v1/refresh_token");
    r.set_content_type("application/json");
    r.set_body( strbuf.GetString() );

    this->send_request( &r );

    //Clear existing JSON document
    strbuf.Clear();
    doc.RemoveAllMembers();
    writer.Reset(strbuf);

    //std::cout << "Refresh response: " << m_response_data << std::endl;

    //Parse response and update database
    ParseResult json_ok = doc.Parse( m_response_data.c_str() );

    if ( !json_ok )
    {
        handle_json_error(json_ok);
        return false;
    }

    doc.Accept(writer);

    std::cout << "Check 1" << std::endl;

    //Handle errors
    if ( m_http_status != 200 )
        return false;
    else if ( !doc.HasMember("response") )
        return false;

    std::cout << "Check 2" << std::endl;

    const Value& response = doc["response"];

    //Invalid JSON
    if ( !response.HasMember("access_token") )
    {
        return false;
    }

    std::cout << "Check 3" << std::endl;

    std::string token = response["access_token"].GetString();

    std::cout << "Check 3.5" << std::endl;

    //Update LocalDatabase Settings
    m_ldb->update_setting("auth_token", token );
    m_ldb->update_setting("refresh_token", response["refresh_token"].GetString() );
    m_ldb->update_setting("token_expiry", response["token_expiry"].GetInt() );

    std::cout << "Check 4" << std::endl;

    //Update Authorization header
    m_auth_header = get_auth_header(token, user_id );

    return true;

}

void BackupClient::handle_auth_error()
{

    if ( m_http_status != 401 ) //http code should always be 401 for auth errors
        return;

    //Try to parse the 401 response
    Document doc;
    ParseResult json_ok = doc.Parse( m_response_data.c_str() );

    if ( !json_ok )
    {
        handle_json_error(json_ok);
        return;
    }

    if ( !doc.HasMember("error") )
    {
        m_log->add_message("Unable to parse 401 authorization response JSON", "Authentication failure");
        return;
    }

    const Value& auth_error = doc["error"];

    //Log an error message if it was returned
    if ( auth_error.HasMember("message") )
    {
        m_log->add_message(auth_error["message"].GetString(), "Authentication failure");
    }

    if ( auth_error.HasMember("token_expired") )
    {
        if ( auth_error["token_expired"].GetBool() == true )
        {
            //Try to refresh the token
            m_log->add_message("Access token is expired. Trying to refresh", "Authentication failure");
            refresh_token();
        }
    }
    else {
        //Try to activate the client?
        //activate();
    }

}

std::string BackupClient::get_auth_header(const std::string& token, int user_id)
{

    using namespace CryptoPP;

    std::string header = std::to_string(user_id) + ":" + token;
    std::string encoded;

    StringSource ss(header, true,
        new Base64Encoder(
            new StringSink(encoded), false
        )
    );

    return encoded;

}

void BackupClient::handle_api_error()
{

    Document doc;
    ParseResult json_ok = doc.Parse( m_response_data.c_str() );

    if ( !json_ok )
    {
        handle_json_error(json_ok);
        return;
    }

    if ( !doc.HasMember("error") )
        return; //Nothing to do

    const Value& error = doc["error"];

    if ( error.HasMember("message") )
    {
        std::stringstream ss;
        ss << "API Error: ";
        ss << error["message"].GetString();
        ss << ". Status Code: " << std::to_string(m_http_status);
        m_log->add_message(ss.str(), "API");

    }

}

void BackupClient::handle_json_error(const ParseResult& res)
{
    std::stringstream ss;
    ss << "JSON parser error: ";
    ss << GetParseError_En(res.Code());
    m_log->add_message(ss.str(), "JSON");
}
