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

    //Set SSL Opts
    m_ssl_ctx.set_default_verify_paths();

    if ( m_verify_cert )
        m_ssl_socket.set_verify_mode(ssl::verify_peer);
    else
        m_ssl_socket.set_verify_mode(ssl::verify_none);

    //Determine protocol, hostname, etc
    this->parse_url(host);

    //Set auth token from local database
    Backup::Database::LocalDatabase* ldb = &Backup::Database::LocalDatabase::get_database();

    this->m_auth_token = ldb->get_setting_str("auth_token");

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
        std::cout << "Error: " << e.message() << "\n";
    }

}

void BackupClient::handle_response( const boost::system::error_code& e )
{

    m_response_ec = e; //Update error code

    std::cout << "Handled response.." << '\n';

    if (!e)
    {

        //Set the deadline timer
        m_deadline_timer.expires_from_now(boost::posix_time::seconds(10));

        //Reset HTTP Status
        m_http_status = 0;

        // Check that response is OK.
        std::istream response_stream(&m_response_buffer);

        std::string http_version;
        response_stream >> http_version;
        response_stream >> m_http_status;
        std::string status_message;
        std::getline(response_stream, status_message);

        if (!response_stream || http_version.substr(0, 5) != "HTTP/")
        {
            std::cout << "Invalid response\n";
            m_response_ec = boost::asio::error::operation_aborted; //Handle status error
            return;
        }

        //Handle a possible redirect
        if ( m_http_status == 301 )
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
        else if (m_http_status != 200)
        {
            std::cout << "Response returned with status code ";
            std::cout << m_http_status << "\n";
            //m_response_ec = boost::asio::error::operation_aborted; //Set static error
            //return;
        }

        // Read the response headers, which are terminated by a blank line.
        if ( !m_use_ssl )
            boost::asio::async_read_until(m_socket, m_response_buffer, "\r\n\r\n", boost::bind(&BackupClient::handle_read_headers, this, boost::asio::placeholders::error));
        else
            boost::asio::async_read_until(m_ssl_socket, m_response_buffer, "\r\n\r\n", boost::bind(&BackupClient::handle_read_headers, this, boost::asio::placeholders::error));

    }
    else
    {
        std::cout << "Error: " << e << "\n";
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
        std::cout << "Error: " << e << "\n";
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
        std::cout << "Error: " << e << "\n";
        m_response_ec = boost::asio::error::eof;
    }
    else
        m_response_ec = boost::asio::error::eof;

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

    if ( !m_connected ) {
        this->connect();
    }

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
        request_stream << "Authorization: " << this->m_auth_token << "\r\n";
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

}

int BackupClient::init_upload ( Backup::File::BackupFile * bf )
{

    using namespace rapidjson;

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

    //Reset Document
    doc.RemoveAllMembers();
    strbuf.Clear();
    writer.Reset(strbuf);

    doc.Parse( m_response_data.c_str() );
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

    using namespace rapidjson;

    //Build a JSON payload of the file properties/metadata
    Document doc;
    doc.SetObject();
    Document::AllocatorType& alloc = doc.GetAllocator();

    std::map<std::string,Value> jmap;

    std::string file_part = bf->get_file_part(part_number);

    //Get Activation Code from DB
    jmap.insert( std::pair<std::string,Value>( "upload_id", Value( bf->get_upload_id() ) ) );
    jmap.insert( std::pair<std::string,Value>( "part_number", Value( part_number) ) );
    jmap.insert( std::pair<std::string,Value>( "part_size", Value( file_part.size() ) ) );
    jmap.insert( std::pair<std::string,Value>( "hash", Value( bf->get_hash(file_part).c_str(), alloc ) ) );

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

    this->connect();

    using namespace Backup::Database;

    //Create JSON for client heartbeat
    rapidjson::Document doc;
    doc.SetObject();
    rapidjson::Document::AllocatorType& alloc = doc.GetAllocator();

    LocalDatabase* ldb = &LocalDatabase::get_database();

    std::map<std::string,std::string> jmap;
    jmap.insert( std::pair<std::string,std::string>("host_name", ldb->get_setting_str("hostname")) );
    jmap.insert( std::pair<std::string,std::string>("os", ldb->get_setting_str("host_os")) );
    jmap.insert( std::pair<std::string,std::string>("client_version", ldb->get_setting_str("client_version")) );
    jmap.insert( std::pair<std::string,std::string>("domain", ldb->get_setting_str("host_domain")) );

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

    using namespace rapidjson;
    using Backup::Database::LocalDatabase;

    LocalDatabase* ldb = &LocalDatabase::get_database();

    Document doc;
    doc.SetObject();
    Document::AllocatorType& alloc = doc.GetAllocator();

    //Get Activation Code from DB
    Value activation_code( ldb->get_setting_str("activation_code").c_str(), alloc );
    Value user_name( ldb->get_setting_str("username").c_str(), alloc );

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

    std::cout << strbuf.GetString() << std::endl;

    //Reset Document
    doc.RemoveAllMembers();
    writer.Reset(strbuf);

    doc.Parse( m_response_data.c_str() );

    doc.Accept(writer);

    std::cout << strbuf.GetString() << std::endl;

    //Check if access token is present
    const Value& response = doc["response"];

    if ( !response.HasMember("access_token") ) {
        m_activated=false;
        return false;
    }

    //Set auth token
    std::string token = response["access_token"].GetString();

    if ( token == "" && !response["is_activated"].GetBool() ) {
        m_activated=false;
        return false;
    }

    //Set Auth Token?
    if ( token != "" ) {
        ldb->update_setting("auth_token", token );
        m_auth_token = token;
    }

    m_activated=true;

    return true;

}

bool BackupClient::is_activated()
{
    return m_activated;
}

unsigned int BackupClient::get_http_status()
{
    return m_http_status;
}
