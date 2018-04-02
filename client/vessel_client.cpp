#include "vessel_client.hpp"

using boost::asio::ip::tcp;
using boost::asio::deadline_timer;
using namespace Backup::Networking;

VesselClient::VesselClient( const std::string& host ) : HttpClient(host)
{

    //Set local database object
    m_ldb = &Backup::Database::LocalDatabase::get_database();

    //Get log
    m_log = new Backup::Logging::Log("vessel_cli");

    //Get user information
    this->m_auth_token = m_ldb->get_setting_str("auth_token");
    this->m_user_id = m_ldb->get_setting_int("user_id");

    //Create the authorization header used for API requests
    this->m_auth_header = get_auth_header(m_auth_token, m_user_id);

}

VesselClient::~VesselClient()
{

}

void VesselClient::send_request( Backup::Networking::HttpRequest* r )
{

    //If client is already connected, disconnect before a new attempt
    if ( is_connected() )
        disconnect();

    //Connect to server
    connect();

    if ( !is_connected() )
        return;

    //clear any existing data
    clear_response();
    clear_headers();

    //15 seconds until timeout
    set_deadline(15);

    //Build the HTTP Request
    std::stringstream request_stream(std::stringstream::out | std::stringstream::binary);
    request_stream << r->get_method() << " " << r->get_uri() << " HTTP/1.1\r\n";
    request_stream << "Host: " << get_hostname() << "\r\n";
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
    clear_error_code();

    //Write HTTP request to socket
    write_socket(&request_stream.str()[0]);

    //Run the handlers until the response sets the status code or EOF
    do { run_io_service(); std::cout << "..." << '\n'; } while ( !get_error_code() );

    disconnect();

    unsigned int http_status = get_http_status();

     std::cout << "HTTP Status: " << http_status << "\n";

    /**
     ** Handle HTTP Error codes
     ** The 401 handler is designed to perform a user activation or token refresh on the fly in the event of authorization failures
    **/
    if (http_status == 401)
        handle_auth_error();
    else if ( http_status > 200 ) {
        m_log->add_message("There was an error returned by the server. Status code: " + std::to_string(http_status), "ASIO" );
        handle_api_error();
    }

}

int VesselClient::init_upload ( Backup::File::BackupFile * bf )
{

    //Verify that file size is > 0
    if ( bf->get_file_size() <= 0 )
        return -1;

    Document doc;
    doc.SetObject();
    Document::AllocatorType& alloc = doc.GetAllocator();

    std::map<std::string,Value> jmap;

    std::string parent_path = Backup::File::BackupDirectory(bf->get_parent_path()).get_canonical_path();

    //Get Activation Code from DB
    jmap.insert( std::pair<std::string,Value>( "file_name", Value( bf->get_file_name().c_str(), alloc ) ) );
    jmap.insert( std::pair<std::string,Value>( "file_size", Value( bf->get_file_size() ) ) );
    jmap.insert( std::pair<std::string,Value>( "file_type", Value( bf->get_file_type().c_str(), alloc ) ) );
    jmap.insert( std::pair<std::string,Value>( "hash", Value( bf->get_hash_sha1().c_str(), alloc ) ) );
    jmap.insert( std::pair<std::string,Value>( "file_path", Value( parent_path.c_str(), alloc ) ) );
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
    std::cout << "Init upload response: " << get_response() << std::endl;

    //Reset Document
    doc.RemoveAllMembers();
    strbuf.Clear();
    writer.Reset(strbuf);

    ParseResult json_ok = doc.Parse( get_response().c_str() );

    if ( !json_ok )
    {
        handle_json_error(json_ok);
        return -1;
    }

    doc.Accept(writer);

    //Default to -1
    int upload_id = -1;

    std::cout << "Response: \n" << get_response() << std::endl;

    if (get_http_status() == 200 ) {

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

bool VesselClient::upload_file_part( Backup::File::BackupFile * bf, int part_number=1 )
{

    //Validate upload id
    if ( bf->get_upload_id() < 0 )
        return false;

    //Validate file size
    if ( bf->get_file_size() <= 0 )
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
    jmap.insert( std::pair<std::string,Value>( "hash", Value( bf->get_hash_sha1(file_part).c_str(), alloc ) ) );
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

    std::cout << "File part response: " << get_response() << std::endl;

    return true;

}

bool VesselClient::heartbeat()
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

std::string VesselClient::get_client_settings()
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

bool VesselClient::activate()
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

    std::cout << "Activation response: " << get_response() << std::endl;

    ParseResult json_ok = doc.Parse( get_response().c_str() );

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

bool VesselClient::is_activated()
{
    return m_activated;
}

void VesselClient::use_compression(bool flag)
{
    m_use_compression=flag;
}

bool VesselClient::refresh_token()
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
    ParseResult json_ok = doc.Parse( get_response().c_str() );

    if ( !json_ok )
    {
        handle_json_error(json_ok);
        return false;
    }

    doc.Accept(writer);

    //Handle errors
    if ( get_http_status() != 200 )
        return false;
    else if ( !doc.HasMember("response") )
        return false;

    const Value& response = doc["response"];

    //Invalid JSON
    if ( !response.HasMember("access_token") )
    {
        return false;
    }

    std::string token = response["access_token"].GetString();

    //Update LocalDatabase Settings
    m_ldb->update_setting("auth_token", token );
    m_ldb->update_setting("refresh_token", response["refresh_token"].GetString() );
    m_ldb->update_setting("token_expiry", response["token_expiry"].GetInt() );

    //Update Authorization header
    m_auth_header = get_auth_header(token, user_id );

    return true;

}

void VesselClient::handle_auth_error()
{

    if ( get_http_status() != 401 ) //http code should always be 401 for auth errors
        return;

    //Try to parse the 401 response
    Document doc;
    ParseResult json_ok = doc.Parse( get_response().c_str() );

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

std::string VesselClient::get_auth_header(const std::string& token, int user_id)
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

void VesselClient::handle_api_error()
{

    Document doc;
    ParseResult json_ok = doc.Parse( get_response().c_str() );

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
        ss << ". Status Code: " << std::to_string(get_http_status());
        m_log->add_message(ss.str(), "API");

    }

}

void VesselClient::handle_json_error(const ParseResult& res)
{
    std::stringstream ss;
    ss << "JSON parser error: ";
    ss << GetParseError_En(res.Code());
    m_log->add_message(ss.str(), "JSON");
}
