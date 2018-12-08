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

std::string VesselClient::get_provider_endpoint(const std::string& provider_type )
{

    if ( provider_type == "aws_s3") return "aws";
    if ( provider_type == "azure_blob") return "azure";
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

    writer.Key("total_parts");
    writer.Uint( bf.get_total_parts() );

    writer.Key("user_name");
    writer.String( m_ldb->get_setting_str("username").c_str() );

    writer.Key("storage_provider_id");
    writer.String( provider.provider_id.c_str() );

    //
    writer.EndObject();

    //Create a new HTTP request
    std::string endpoint = "/upload/" + get_provider_endpoint( provider.provider_type );

    HttpRequest r;
    r.set_auth_header("Bearer " + m_client_token);
    r.accept("application/json");
    r.set_content_type("application/json");
    r.set_body( strbuf.GetString() );
    r.set_method("POST");
    r.set_url(m_api_path + endpoint);

    //Send the init request
    send_http_request(r);

    //Parse the upload ID from the response
    if ( get_http_status() != 200 ) {
        throw VesselException( VesselException::BadUpload, "Bad payload or invalid upload id");
    }

    Document document;
    document.Parse( get_response().c_str() ) ;

    if ( !document.HasMember("upload") ) {
        throw VesselException( VesselException::BadUpload, "Bad payload or invalid upload id");
    }

    const Value& upload = document["upload"];

    std::string upload_id = upload["upload_id"].GetString();

    m_log->add_message("Initialized upload: " + upload_id, "File Upload");

    return upload_id;

}

void VesselClient::complete_upload( const std::string& upload_id )
{

    //Write some JSON
    StringBuffer strbuf;
    Writer<StringBuffer> writer(strbuf);

    writer.StartObject();
    writer.Key("uploaded");
    writer.Bool(true);
    writer.EndObject();

    //Create a new HTTP request
    HttpRequest r;
    r.set_auth_header("Bearer " + m_client_token);
    r.accept("application/json");
    r.set_content_type("application/json");
    r.set_body( strbuf.GetString() );
    r.set_method("PUT");
    r.set_url(m_api_path + "/upload/" + upload_id);

    //Send the init request
    send_http_request(r);

    //Parse the upload ID from the response
    if ( get_http_status() != 200 ) {
        m_log->add_error("Failed to mark file as uploaded: (Upload Id=" + upload_id + ")", "File Upload");
    }

}

bool VesselClient::upload_file_part( Vessel::File::BackupFile * bf, int part_number=1 )
{
    //TODO
    return true;
}

void VesselClient::heartbeat()
{

    //UNIX Timestamp of Last Heartbeat
    unsigned long last_heartbeat = m_ldb->get_setting_int("last_heartbeat");

    //Write some JSON
    StringBuffer strbuf;
    PrettyWriter<StringBuffer> writer(strbuf);

    writer.StartObject();

    writer.Key("user_id");
    writer.String( m_ldb->get_setting_str("user_id").c_str() );

    writer.Key("host_name");
    writer.String( m_ldb->get_setting_str("hostname").c_str() );

    writer.Key("os");
    writer.String( m_ldb->get_setting_str("host_os").c_str() );

    writer.Key("client_version");
    writer.String( m_ldb->get_setting_str("client_version").c_str() );

    writer.Key("domain");
    writer.String( m_ldb->get_setting_str("host_domain").c_str() );

    //
    writer.Key("stats");
    writer.StartObject();

    std::map<std::string,int> stats = m_ldb->get_stats();
    for ( auto itr = stats.begin(); itr != stats.end(); ++itr )
    {
        writer.Key(itr->first.c_str());
        writer.Int(itr->second);
    }

    writer.EndObject();

    //Sent recent log output to server
    writer.Key("logs");
    writer.StartArray();

    std::string query = "SELECT message,type,exception,payload,code,error,logged_at FROM backup_log WHERE logged_at >= ?1";
    sqlite3_stmt* stmt;

    if ( sqlite3_prepare_v2(m_ldb->get_handle(), query.c_str(), -1, &stmt, NULL ) != SQLITE_OK ) {
        m_log->add_error("Failed to retrieve recent logs for heartbeat: " + m_ldb->get_last_err(), "Heartbeat");
    }
    else
    {

        sqlite3_bind_int(stmt, 1, last_heartbeat);

        while ( sqlite3_step(stmt) == SQLITE_ROW )
        {
            writer.StartObject();
            //
            writer.Key("type");
            writer.String( LocalDatabase::get_sqlite_str( sqlite3_column_text(stmt, 1) ).c_str() );
            //
            writer.Key("message");
            writer.String( LocalDatabase::get_sqlite_str( sqlite3_column_text(stmt, 0) ).c_str() );
            //
            writer.Key("exception");
            writer.String( LocalDatabase::get_sqlite_str( sqlite3_column_text(stmt, 2) ).c_str() );
            //
            writer.Key("payload");
            writer.String( LocalDatabase::get_sqlite_str( sqlite3_column_text(stmt, 3) ).c_str() );
            //
            writer.Key("code");
            writer.Int( (int)sqlite3_column_int(stmt, 4) );
            //
            writer.Key("error");
            writer.Int( (int)sqlite3_column_int(stmt, 5) );
            //
            writer.Key("logged_at");
            writer.Uint( (unsigned long)sqlite3_column_int(stmt, 6) );
            //
            writer.EndObject();
        }

    }

    //Cleanup
    sqlite3_finalize(stmt);

    writer.EndArray();

    //
    writer.EndObject();

    std::string payload = strbuf.GetString();

    //std::cout << "Sending payload: " << '\n' << payload << '\n';

    //Send the Heartbeat Request

    HttpRequest r;
    r.set_auth_header("Bearer " + m_client_token);
    r.accept("application/json");
    r.set_content_type("application/json");
    r.set_method("POST");
    r.set_url(m_api_path + "/heartbeat");
    r.set_body( payload );

    send_http_request(r);

    std::time_t response_time = (unsigned long)std::time(nullptr);

    int status_code = get_http_status();

    if ( status_code != 200 ) {
        Log::get_log().add_error("Heartbeat failed with status code: " + std::to_string(status_code), "Heartbeat");
    }

    //Prune logs if they were sent successfully
    if ( status_code == 200 )
    {
        m_ldb->prune_logs(last_heartbeat, response_time-1 );
    }

    //Update Last Heartbeat
    m_ldb->update_setting<unsigned long>("last_heartbeat", (unsigned long)std::time(nullptr) );

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

    writer.Key("user_name");
    writer.String(m_ldb->get_setting_str("username").c_str());

    //
    writer.EndObject();

    std::string payload = strbuf.GetString();

    //std::cout << payload << '\n';

    HttpRequest request;
    request.set_method("POST");
    request.set_url(m_api_path + "/client/install");
    request.add_header("Content-Type: application/json");
    request.add_header("Accept: application/json");
    request.set_auth_header("Bearer " + deployment_key);
    request.set_body(payload);

    //Send the request
    int status = send_http_request(request);

    if ( status != 200 ) //HTTP code should always be 200
    {
        m_log->add_error("Failed to install the client application. Please check the auth token and/or deployment key", "Client");
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


    if ( document.HasMember("user") )
    {
        const Value& user = document["user"];
        m_ldb->update_setting<std::string>( "user_id", user["user_id"].GetString() );
    }

}

bool VesselClient::sync_storage_provider(const Value& obj)
{

    StorageProvider providerObj;
    providerObj.provider_id = obj["provider_id"].GetString();
    providerObj.provider_name = obj["provider_name"].GetString();
    providerObj.provider_type = obj["provider_type"].GetString();
    providerObj.bucket_name = obj["bucket_name"].IsNull() ? "" : obj["bucket_name"].GetString();
    providerObj.description = obj["description"].IsNull() ? "" : obj["description"].GetString();
    providerObj.server = obj["server"].IsNull() ? "" : obj["server"].GetString();
    providerObj.access_id = obj["access_id"].IsNull() ? "" : obj["access_id"].GetString();
    providerObj.storage_path = obj["storage_path"].IsNull() ? "" : obj["storage_path"].GetString();
    providerObj.region = obj["region"].IsNull() ? "" : obj["region"].GetString();
    providerObj.priority = obj["priority"].IsNull() ? 0 : obj["priority"].GetInt();

    sqlite3_stmt* stmt;
    std::string query = "REPLACE INTO backup_provider (provider_id,name,server,type,storage_path,priority,bucket_name,region,access_id) VALUES(?1,?2,?3,?4,?5,?6,?7,?8,?9)";

    if ( sqlite3_prepare_v2(m_ldb->get_handle(), query.c_str(), -1, &stmt, NULL ) != SQLITE_OK )
        return false;

    sqlite3_bind_text(stmt, 1, providerObj.provider_id.c_str(), providerObj.provider_id.size(), 0 );
    sqlite3_bind_text(stmt, 2, providerObj.provider_name.c_str(), providerObj.provider_name.size(), 0 );
    sqlite3_bind_text(stmt, 3, providerObj.server.c_str(), providerObj.server.size(), 0 );
    sqlite3_bind_text(stmt, 4, providerObj.provider_type.c_str(), providerObj.provider_type.size(), 0 );
    sqlite3_bind_text(stmt, 5, providerObj.storage_path.c_str(), providerObj.storage_path.size(), 0 );
    sqlite3_bind_int(stmt, 6, providerObj.priority );
    sqlite3_bind_text(stmt, 7, providerObj.bucket_name.c_str(), providerObj.bucket_name.size(), 0 );
    sqlite3_bind_text(stmt, 8, providerObj.region.c_str(), providerObj.region.size(), 0 );
    sqlite3_bind_text(stmt, 9, providerObj.access_id.c_str(), providerObj.access_id.size(), 0 );

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

    if ( sqlite3_prepare_v2(m_ldb->get_handle(), query.c_str(), -1, &stmt, NULL ) != SQLITE_OK )
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

    if ( sqlite3_prepare_v2(m_ldb->get_handle(), query.c_str(), -1, &stmt, NULL ) != SQLITE_OK )
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
    std::string query = "SELECT provider_id,name,description,server,type,bucket_name,region,storage_path,priority,access_id FROM backup_provider ORDER BY priority ASC LIMIT 1";

    if ( sqlite3_prepare_v2(m_ldb->get_handle(), query.c_str(), -1, &stmt, NULL ) != SQLITE_OK )
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
    provider.access_id = (char*)sqlite3_column_text(stmt, 9);

    //Cleanup
    sqlite3_finalize(stmt);

    return provider;

}

void VesselClient::refresh_client_token()
{
    m_client_token = m_ldb->get_setting_str("client_token");
}
