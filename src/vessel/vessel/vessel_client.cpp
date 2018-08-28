#include <vessel/vessel/vessel_client.hpp>

using boost::asio::ip::tcp;
using boost::asio::deadline_timer;
using namespace Vessel::Networking;

VesselClient::VesselClient( const std::string& host ) : HttpClient(host)
{

    //Set local database object
    m_ldb = &Vessel::Database::LocalDatabase::get_database();

    //Get log
    m_log = &Log::get_log();

    //Cache vars
    m_auth_token = m_ldb->get_setting_str("auth_token");
    m_client_token = m_ldb->get_setting_str("client_token");
    m_user_id = m_ldb->get_setting_str("user_id");
    m_api_path = m_ldb->get_setting_str("vessel_api_path");

    //Create the authorization header used for API requests
    m_auth_header = get_auth_header(m_auth_token, m_user_id);

}

VesselClient::~VesselClient()
{

}

std::string VesselClient::get_provider_endpoint(const std::string& provider_type )
{

    if ( provider_type == "aws_s3") return "aws";
    if ( provider_type == "azure") return "azure";
    if ( provider_type == "vessel") return "vessel";
    if ( provider_type == "local") return "local";

    return "";
}

std::string VesselClient::init_upload ( const BackupFile& bf )
{

    StorageProvider provider = get_storage_provider();

    //Write some JSON
    StringBuffer strbuf;
    Writer<StringBuffer> writer(strbuf);

    writer.StartObject();

    writer.Key("file_name");
    writer.String( bf.get_file_name().c_str() );

    writer.Key("file_path");
    writer.String( bf.get_parent_path().c_str() );

    writer.Key("file_type");
    writer.String( bf.get_file_type().c_str() );

    writer.Key("file_size");
    writer.Uint( bf.get_file_size() );

    writer.Key("hash");
    writer.String( bf.get_hash_sha1().c_str() );

    writer.Key("user_name");
    writer.String( "admin" /*m_ldb->get_setting_str("username").c_str()*/ );

    writer.Key("storage_provider_id");
    writer.String( provider.provider_id.c_str() );

    //
    writer.EndObject();

    std::string payload = strbuf.GetString();

    //Create a new HTTP request
    std::string endpoint = "/upload/" + get_provider_endpoint( provider.provider_type );

    HttpRequest r;
    r.set_auth_header("Bearer " + m_client_token);
    r.accept("application/json");
    r.set_content_type("application/json");
    r.set_body(strbuf.GetString());
    r.set_method("POST");
    r.set_url(m_api_path + endpoint);

    //Send the init request
    send_http_request(r);

    //ParseResult json_ok = doc.Parse( get_response().c_str() );

    return "";

}

bool VesselClient::upload_file_part( Vessel::File::BackupFile * bf, int part_number=1 )
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

    std::cout << "Body:\n" << body.str() << std::endl;

    //Create a new HTTP request
    HttpRequest r;
    r.add_header("Content-Disposition: multipart/form-data");
    r.add_header("Content-Type: multipart/form-data; boundary=BackupFile");
    r.set_body(body.str());
    r.set_method("POST");
    r.set_url(m_api_path + "/file");

    //std::cout << "Example body:\n" << r.get_body() << std::endl;

    send_http_request(r);

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
    r.set_url(m_api_path + "/heartbeat");
    r.set_body( strbuf.GetString() );

    send_http_request(r);

    if ( get_http_status != 200 ) {
        return false;
    }

    const std::string& response = get_response();

    //Parse Storage Providers from response
    parse_s

    return true;

}

std::string VesselClient::get_client_settings()
{

    if ( m_auth_token == "" ) //Don't try to get settings if no auth token is present
        return "{}";

    HttpRequest r;
    r.set_content_type("application/json");
    r.set_method("GET");
    r.set_url(m_api_path + "/settings");

    send_http_request(r);

    return get_response();

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
    r.set_url(m_api_path + "/activate");
    r.set_content_type("application/json");
    r.set_body( strbuf.GetString() );

    send_http_request( r );

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
            std::string user_id = response["user_id"].GetString();
            if ( !user_id.empty() )
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
    std::string user_id = m_ldb->get_setting_str("user_id");

    //Prepare JSON request
    Value refresh_token_v( refresh_token.c_str(), refresh_token.size(), alloc );
    Value user_id_v( user_id.c_str(), user_id.size(), alloc );

    doc.AddMember( "refresh_token", refresh_token_v, alloc );
    doc.AddMember( "user_id", user_id_v, alloc );

    //Write JSON
    StringBuffer strbuf;
    PrettyWriter<StringBuffer> writer(strbuf);
    doc.Accept(writer);

    //Send Http Request
    HttpRequest r;
    r.set_method("POST");
    r.set_url(m_api_path + "/refresh_token");
    r.set_content_type("application/json");
    r.set_body( strbuf.GetString() );

    send_http_request( r );

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

std::string VesselClient::get_auth_header(const std::string& token, const std::string& user_id)
{

    using namespace CryptoPP;

    std::string header = user_id + ":" + token;
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

bool VesselClient::has_deployment_key()
{
    return (m_ldb->get_setting_str("deployment_key") != "") ? true : false;
}

bool VesselClient::has_client_token()
{
    return (m_ldb->get_setting_str("client_token") != "") ? true : false;
}

void VesselClient::install_client()
{

    std::string deployment_key = m_ldb->get_setting_str("deployment_key");
    std::string auth_token = m_ldb->get_setting_str("auth_token");

    //Write some JSON
    StringBuffer strbuf;
    Writer<StringBuffer> writer(strbuf);

    writer.StartObject();

    writer.Key("deployment_key");
    writer.String(deployment_key.c_str());

    writer.Key("client_name");
    writer.String(m_ldb->get_setting_str("hostname").c_str());

    writer.Key("os");
    writer.String(m_ldb->get_setting_str("host_os").c_str());

    writer.Key("ip_address");
    writer.String("");

    writer.Key("mac_address");
    writer.String("");

    writer.Key("domain");
    writer.String(m_ldb->get_setting_str("host_domain").c_str());

    writer.Key("client_version");
    writer.String(m_ldb->get_setting_str("client_version").c_str());

    //
    writer.EndObject();

    std::string payload = strbuf.GetString();

    std::cout << payload << '\n';

    HttpRequestStream http_request;
    http_request << "POST " << m_api_path << "/client/install HTTP/1.1" << "\r\n";
    http_request << "Host: " << get_hostname() << ":8000\r\n";
    http_request << "Content-Type: application/json" << "\r\n";
    http_request << "Authorization: Bearer " << deployment_key << "\r\n";
    http_request << "Accept: application/json" << "\r\n";
    http_request << "Content-Length: " << payload.size() << "\r\n";
    http_request << "\r\n";
    http_request << payload;

    std::cout << http_request.str() << '\n';

    //Connect to socket
    connect();

    //Write to socket
    write_socket(http_request.str());

    do { run_io_service(); std::cout << "..." << '\n'; } while ( !get_error_code() );

    disconnect();

    if ( get_http_status() != 200 ) //http code should always be 200
    {
        throw VesselException(VesselException::NotInstalled, "Failed to install the client application. Please check the auth token and/or deployment key");
    }

    std::string response = get_response();

    //Parse the response, Save Storage Providers, Update settings, etc

    Document document;
    document.Parse(response.c_str());

    if ( !document.HasMember("app_client" ) )
    {
        throw VesselException(VesselException::NotInstalled, "Invalid JSON response for client install");
    }

    const Value& appClientObj = document["app_client"];

    //Update client settings
    m_ldb->update_setting("client_id", appClientObj["client_id"].GetString() );
    m_ldb->update_setting("client_token", appClientObj["token"].GetString() );

    //Create vector to store current providers, and remove ones that no longer exist
    std::vector<std::string> provider_ids;

    //Parse Storage Providers
    if ( document.HasMember("storage_providers") )
    {
        const Value& storageObj = document["storage_providers"];

        if ( storageObj.IsArray() )
        {
            for ( auto& providerObj : storageObj.GetArray() )
            {
                std::string provider_id = providerObj["provider_id"].GetString();
                if ( !sync_storage_provider(providerObj) )
                {
                    throw VesselException(VesselException::ProviderError, "Storage provider " + provider_id + " failed to sync" );
                }
                provider_ids.push_back( provider_id );
            }

            //Remove any obsolete providers
            sync_storage_provider_all(provider_ids);

        }

    }

}

bool VesselClient::sync_storage_provider(const Value& obj)
{

    StorageProvider providerObj;
    providerObj.provider_id = obj["provider_id"].GetString();
    providerObj.provider_name = obj["provider_name"].GetString();
    providerObj.provider_type = obj["provider_type"].GetString();
    providerObj.bucket_name = obj["bucket_name"].GetString();
    providerObj.description = obj["description"].GetString();
    providerObj.server = obj["server"].GetString();
    providerObj.storage_path = obj["storage_path"].GetString();
    providerObj.region = obj["region"].GetString();
    providerObj.priority = obj["priority"].GetInt();

    sqlite3_stmt* stmt;
    std::string query = "REPLACE INTO backup_provider (provider_id,name,server,type,storage_path,priority,bucket_name,region) VALUES(?1,?2,?3,?4,?5,?6,?7,?8)";

    if ( sqlite3_prepare_v2(m_ldb->get_handle(), query.c_str(), query.size(), &stmt, NULL ) != SQLITE_OK )
        return false;

    sqlite3_bind_text(stmt, 1, providerObj.provider_id.c_str(), providerObj.provider_id.size(), 0 );
    sqlite3_bind_text(stmt, 2, providerObj.provider_name.c_str(), providerObj.provider_name.size(), 0 );
    sqlite3_bind_text(stmt, 3, providerObj.server.c_str(), providerObj.server.size(), 0 );
    sqlite3_bind_text(stmt, 4, providerObj.provider_type.c_str(), providerObj.provider_type.size(), 0 );
    sqlite3_bind_text(stmt, 5, providerObj.storage_path.c_str(), providerObj.storage_path.size(), 0 );
    sqlite3_bind_int(stmt, 6, providerObj.priority );
    sqlite3_bind_text(stmt, 7, providerObj.bucket_name.c_str(), providerObj.bucket_name.size(), 0 );
    sqlite3_bind_text(stmt, 8, providerObj.region.c_str(), providerObj.region.size(), 0 );

    int rc = sqlite3_step(stmt);

    //Cleanup
    sqlite3_finalize(stmt);

    if ( rc != SQLITE_DONE )
        return false;

    return true;

}

void VesselClient::sync_storage_provider_all(const std::vector<std::string>& provider_ids)
{

    //Get current storage providers
    sqlite3_stmt* stmt;
    std::string query = "SELECT provider_id FROM backup_provider";

    if ( sqlite3_prepare_v2(m_ldb->get_handle(), query.c_str(), query.size(), &stmt, NULL ) != SQLITE_OK )
        return;

    while ( sqlite3_step(stmt) == SQLITE_ROW )
    {
        std::string id = (char*)sqlite3_column_text(stmt, 0);

        //If not in the provider_ids vector, delete from db
        if ( std::find(provider_ids.begin(), provider_ids.end(), id) == provider_ids.end() )
        {
            delete_storage_provider(id);
        }
    }

    //Cleanup
    sqlite3_finalize(stmt);

}

bool VesselClient::delete_storage_provider(const std::string& id)
{

    sqlite3_stmt* stmt;
    std::string query = "DELETE FROM backup_provider WHERE provider_id=?1";

    if ( sqlite3_prepare_v2(m_ldb->get_handle(), query.c_str(), query.size(), &stmt, NULL ) != SQLITE_OK )
        return false;

    sqlite3_bind_text(stmt, 1, id.c_str(), id.size(), 0 );

    int rc = sqlite3_step(stmt);

    //Cleanup
    sqlite3_finalize(stmt);

    if ( rc != SQLITE_DONE )
        return false;

    return true;

}

StorageProvider VesselClient::get_storage_provider()
{

    sqlite3_stmt* stmt;
    std::string query = "SELECT provider_id,name,description,server,type,bucket_name,region,storage_path,priority FROM backup_provider ORDER BY priority ASC LIMIT 1";

    if ( sqlite3_prepare_v2(m_ldb->get_handle(), query.c_str(), query.size(), &stmt, NULL ) != SQLITE_OK )
    {
        throw DatabaseException(DatabaseException::InvalidStatement, "Bad statement. Failed to get storage provider");
    }

    if ( sqlite3_step(stmt) != SQLITE_ROW )
    {
        throw VesselException(VesselException::ProviderError, "Unable to find a storage provider");
    }

    StorageProvider provider;
    provider.provider_id = LocalDatabase::get_sqlite_str( sqlite3_column_text(stmt, 0) );
    provider.provider_name = (char*)sqlite3_column_text(stmt, 1);
    provider.description = LocalDatabase::get_sqlite_str( sqlite3_column_text(stmt, 2) );
    provider.server = (char*)sqlite3_column_text(stmt, 3);
    provider.provider_type = (char*)sqlite3_column_text(stmt, 4);
    provider.bucket_name = (char*)sqlite3_column_text(stmt, 5);
    provider.region = (char*)sqlite3_column_text(stmt, 6);
    provider.storage_path = (char*)sqlite3_column_text(stmt, 7);
    provider.priority = (int)sqlite3_column_int(stmt, 8);

    //Cleanup
    sqlite3_finalize(stmt);

    return provider;

}
