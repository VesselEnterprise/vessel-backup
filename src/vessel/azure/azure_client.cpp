#include <vessel/azure/azure_client.hpp>

AzureClient::AzureClient(const StorageProvider& provider) : HttpClient(provider.server), m_storage_provider(provider)
{
    m_ldb = &LocalDatabase::get_database();
    m_http_verb = "PUT";
    m_xms_version = "2018-03-28"; //Default
    m_xms_blob_type = "BlockBlob";
    m_xms_date = get_ms_date();
    m_user_id = m_ldb->get_setting_str("user_id");
    m_current_part = 0;
    m_chunk_size = BackupFile::get_chunk_size();
}

AzureClient::~AzureClient()
{

}

void AzureClient::build_headers()
{

    //Clear existing
    m_headers.clear();

    //Persistent headers
    m_headers.insert( std::pair<std::string,std::string>("x-ms-date", m_xms_date) );
    m_headers.insert( std::pair<std::string,std::string>("x-ms-version", m_xms_version) );

    //Optional headers

    if ( m_http_verb == "PUT")
    {

        if ( !m_content_type.empty() )
        {
            m_headers.insert( std::pair<std::string,std::string>("x-ms-blob-content-type", m_content_type ) );
        }

        if ( !m_xms_blob_type.empty() )
        {
            m_headers.insert( std::pair<std::string,std::string>("x-ms-blob-type", m_xms_blob_type) );
        }

        if ( !m_content_md5.empty() )
        {
            m_headers.insert( std::pair<std::string,std::string>("x-ms-blob-content-md5", m_content_md5 ) );
        }

    }

    //...

}

std::string AzureClient::get_canonical_headers()
{

    std::stringstream ss;

    for ( auto it=m_headers.begin(); it != m_headers.end(); ++it )
    {
        if ( it->first.find("x-ms") != std::string::npos )
        {
            ss << boost::to_lower_copy(it->first) << ":" << it->second << '\n';
        }
    }

    //std::cout << "Canonical Headers: " << '\n' << ss.str() << '\n';

    return ss.str();

}

std::string AzureClient::get_canonical_resources()
{
    std::stringstream ss;
    ss << "/" << m_storage_provider.access_id;
    ss << "/" << m_storage_provider.bucket_name;
    ss << "/" << encode_uri( m_file_uri_path );

    if ( m_query_params.find("blockid") != m_query_params.end() )
    {
        ss << "\n";
        ss << "blockid:" << m_block_id << "\ncomp:block";
    }
    else if ( m_query_params.find("comp") != m_query_params.end() )
    {

        if ( m_query_params.find("blocklisttype") != m_query_params.end() )
        {
            ss << "\n";
            ss << "blocklisttype:" << m_query_params["blocklisttype"];
        }

        if ( m_query_params["comp"] == "blocklist")
        {
            ss << "\n";
            ss << "comp:blocklist";
        }

    }

    //std::cout << "Canonical Resources: " << '\n' << ss.str() << '\n';

    return ss.str();
}

std::string AzureClient::get_ms_date()
{

    std::stringstream ss;
    boost::posix_time::ptime pt( boost::posix_time::second_clock::universal_time() );
    boost::posix_time::time_facet* input_facet = new boost::posix_time::time_facet(); //No memory leak here
    input_facet->format("%a, %d %b %Y %T GMT");
    ss.imbue( std::locale(ss.getloc(), input_facet ) );
    ss << pt;

    //std::cout << "MS Date: " << '\n' << ss.str() << '\n';

    return ss.str();

}

std::string AzureClient::get_string_to_sign()
{

    /*
     * https://docs.microsoft.com/en-us/rest/api/storageservices/authorize-with-shared-key

    StringToSign = VERB + "\n" +
        Content-Encoding + "\n" +
        Content-Language + "\n" +
        Content-Length + "\n" +
        Content-MD5 + "\n" +
        Content-Type + "\n" +
        Date + "\n" +
        If-Modified-Since + "\n" +
        If-Match + "\n" +
        If-None-Match + "\n" +
        If-Unmodified-Since + "\n" +
        Range + "\n" +
        CanonicalizedHeaders +
        CanonicalizedResource;

    */

    std::stringstream ss;
    ss << m_http_verb << '\n';
    ss << m_content_encoding << '\n';
    ss << m_content_language << '\n';
    ss << ((m_content_length > 0) ? std::to_string(m_content_length) : "") << '\n';
    ss << m_content_md5 << '\n'; //Content-MD5
    ss << m_content_type << '\n'; //Content-Type
    ss << '\n'; //Date
    ss << '\n'; //If-Modified-Since
    ss << '\n'; //If-Match
    ss << '\n'; //If-None-Match
    ss << '\n'; //If-Unmodified-Since
    ss << '\n'; //Range
    ss << get_canonical_headers();
    ss << get_canonical_resources();

    std::cout << "StringToSign: " << '\n' << ss.str() << '\n';

    return ss.str();

}

void AzureClient::remote_signing(bool flag)
{
    if ( !flag ) {
        read_key_file();
        m_remote_signing=false;
    }
    else {
        m_remote_signing=true;
    }
}

void AzureClient::read_key_file()
{
    std::ifstream infile("keys/azure.key", std::ios::in );
    if ( !infile.is_open() ) {
        throw AzureException(AzureException::BadSignature, "Unable to open azure.key file");
    }

    /**
     ** Line 1 = Server URL
     ** Line 2 = Account Name
     ** Line 3 = Container Name
     ** Line 4 = Shared Key
    */

    std::string tmp;

    if ( std::getline(infile, tmp) ) {
        m_storage_provider.server = tmp;
    }

    if ( std::getline(infile, tmp) ) {
        m_storage_provider.access_id = tmp;
    }

    if ( std::getline(infile, tmp) ) {
        m_storage_provider.bucket_name = tmp;
    }

    if ( std::getline(infile, tmp) ) {
        m_storage_provider.access_key = tmp;
    }

    infile.close();
}

std::string AzureClient::get_ms_signature()
{
    if ( m_remote_signing )
    {
        return api_get_signature();
    }

    return Hash::get_base64( Hash::get_hmac_256( Hash::base64_decode(m_storage_provider.access_key), get_string_to_sign(), false ) );

}

std::string AzureClient::get_file_uri_path()
{

    std::string path = m_user_id + m_file.get_parent_path() + "/" + m_file.get_file_name();

    #ifdef _WIN32
        boost::replace_all(path, "\\", "/");
    #endif

    return path;

}

void AzureClient::reset()
{
    //Reset vars to defaults
    m_http_verb = "PUT";
    m_xms_date = get_ms_date();
    m_current_part=0;
    m_content_body.reset();
    m_content_length = 0;
    m_content_md5.clear();
    m_content_type.clear();
    m_query_params.clear();
    m_xms_blob_type = "BlockBlob";

}

void AzureClient::init_upload(const BackupFile& file)
{

    //Reset vars
    reset();

    //Set file
    m_file = file;

    //Build the relative path on the cloud server
    m_file_uri_path = get_file_uri_path();

}

bool AzureClient::upload()
{
    if ( m_file.get_file_name().empty() ) {
        throw AzureException(AzureException::FileNotInitialized, "File has not been initialized");
    }

    //Reset vars
    reset();

    //Set the file content
    m_content_body = std::make_shared<std::string>( m_file.get_file_contents() );
    m_content_length = m_content_body->size();
    m_content_type = m_file.get_mime_type();
    m_content_md5 = Hash::get_md5_hash( *m_content_body, true ); //Get the MD% hash of the current payload, in this case - the entire file contents

    //Rebuild the request headers
    build_headers();

    //Create an HTTP Request
    HttpRequest request;

    request.set_method("PUT");
    request.set_url("/" + m_storage_provider.bucket_name + "/" + encode_uri( m_file_uri_path ) );
    if ( !m_content_type.empty() ) {
        request.add_header("Content-Type: " + m_content_type);
        request.add_header("x-ms-blob-content-type: " + m_content_type);
    }
    request.add_header("Content-MD5: " + m_content_md5);
    request.add_header("x-ms-date: " + m_xms_date);
    request.add_header("x-ms-version: " + m_xms_version);
    request.add_header("x-ms-blob-type: " + m_xms_blob_type);
    request.add_header("x-ms-blob-content-md5: " + m_content_md5);
    request.set_auth_header("SharedKey " + m_storage_provider.access_id + ":" + get_ms_signature());
    request.set_body( *m_content_body );
    request.accept("application/json");

    send_http_request(request);

    std::cout << "HTTP Status: " << get_http_status() << '\n';
    std::cout << get_response() << '\n';

    if ( get_http_status() != 200 || get_http_status() != 201 ) {
        return false;
    }

    return true;

}

bool AzureClient::upload_part(int part_number)
{

    reset();

    //Prepare the block blob
    m_current_part = part_number;
    m_content_body = std::make_shared<std::string>(m_file.get_file_part(part_number));
    m_content_md5 = Hash::get_md5_hash(*m_content_body, true);
    m_content_length = m_content_body->size();
    m_content_type.clear(); //Content-Type should not be passed with blocks
    m_block_id = Hash::get_base64( std::to_string(part_number) );
    m_xms_blob_type.clear(); //Do not pass when uploading a block chunk

    m_query_params.insert( std::pair<std::string,std::string>("comp", "block") );
    m_query_params.insert( std::pair<std::string,std::string>("blockid", m_block_id) );

    //Rebuild headers for the current block
    build_headers();

    //Create an HTTP Request
    HttpRequest request;

    request.set_method("PUT");
    request.set_url("/" + m_storage_provider.bucket_name + "/" + encode_uri( m_file_uri_path ) + "?comp=block&blockid=" + encode_uri(m_block_id) );
    request.add_header("Content-MD5: " + m_content_md5);
    request.add_header("x-ms-date: " + m_xms_date);
    request.add_header("x-ms-version: " + m_xms_version);
    request.add_header("x-ms-blob-content-md5: " + m_content_md5);
    request.set_auth_header("SharedKey " + m_storage_provider.access_id + ":" + get_ms_signature());
    request.set_body( *m_content_body );
    request.accept("application/json");

    send_http_request(request);

    if ( get_http_status() != 200 && get_http_status() != 201 ) {
        return false;
    }

    return true;

}

bool AzureClient::init_block()
{

    /*
     * Upload a zero length blob to initialize the multi block upload
    */

    //Reset vars
    reset();

    //Set the file content
    m_content_length = 0;
    m_content_type = m_file.get_mime_type();

    //Rebuild the request headers
    build_headers();

    //Create an HTTP Request
    HttpRequest request;

    request.set_method("PUT");
    request.set_url("/" + m_storage_provider.bucket_name + "/" + encode_uri( m_file_uri_path ) );
    if ( !m_content_type.empty() ) {
        request.add_header("Content-Type: " + m_content_type);
        request.add_header("x-ms-blob-content-type: " + m_content_type);
    }
    request.add_header("x-ms-date: " + m_xms_date);
    request.add_header("x-ms-version: " + m_xms_version);
    request.add_header("x-ms-blob-type: " + m_xms_blob_type);
    request.set_auth_header("SharedKey " + m_storage_provider.access_id + ":" + get_ms_signature());
    request.accept("application/json");

    send_http_request(request);

    if ( get_http_status() != 200 || get_http_status() != 201 ) {
        return false;
    }

    return true;

}

bool AzureClient::complete_multipart_upload(int total_parts)
{

    /*
    <?xml version="1.0" encoding="utf-8"?>
    <BlockList>
        <Committed>first-base64-encoded-block-id</Committed>
        <Uncommitted>second-base64-encoded-block-id</Uncommitted>
        <Latest>third-base64-encoded-block-id</Latest>
        ...
    </BlockList>
    */

    //Reset vars
    reset();

    //Prepare the list request

    //Build request payload
    std::string payload = "<?xml version=\"1.0\" encoding=\"utf-8\"?><BlockList>";

    for ( int i=1; i <= total_parts; i++ )
    {
        payload += "<Latest>" + Hash::get_base64( std::to_string(i) ) + "</Latest>";
    }

    payload += "</BlockList>";

    std::cout << "Payload:\n" << payload << '\n';

    m_content_body = std::make_shared<std::string>(payload);
    m_content_md5 = Hash::get_md5_hash(*m_content_body, true);
    m_content_length = m_content_body->size();
    m_content_type = m_file.get_mime_type();
    m_xms_blob_type.clear(); //Do not send when completing the request

    m_query_params.insert( std::pair<std::string,std::string>("comp", "blocklist") );

    //Rebuild headers for list complete request
    build_headers();

    //Create an HTTP Request
    HttpRequest request;

    request.set_method("PUT");
    request.set_url("/" + m_storage_provider.bucket_name + "/" + encode_uri( m_file_uri_path ) + "?comp=blocklist" );
    request.add_header("Content-MD5: " + m_content_md5);
    request.add_header("x-ms-date: " + m_xms_date);
    request.add_header("x-ms-version: " + m_xms_version);
    request.add_header("x-ms-blob-content-md5: " + m_content_md5);
    request.set_auth_header("SharedKey " + m_storage_provider.access_id + ":" + get_ms_signature());
    if ( !m_content_type.empty() ) {
        request.add_header("Content-Type: " + m_content_type);
        request.add_header("x-ms-blob-content-type: " + m_content_type);
    }
    request.set_body( *m_content_body );
    request.accept("application/json");

    send_http_request(request);

    if ( get_http_status() != 200 && get_http_status() != 201 ) {
        return false;
    }

    //return get_header("x-ms-request-id");

    return true;

}

std::string AzureClient::get_block_list()
{

    //Reset vars
    reset();

    m_http_verb = "GET";

    m_query_params.insert( std::pair<std::string,std::string>("comp", "blocklist") );
    m_query_params.insert( std::pair<std::string,std::string>("blocklisttype", "all") );

    build_headers();

    //Create an HTTP Request
    HttpRequest request;

    request.set_method("GET");
    request.set_url("/" + m_storage_provider.bucket_name + "/" + encode_uri( m_file_uri_path ) + "?comp=blocklist&blocklisttype=all" );
    request.add_header("x-ms-date: " + m_xms_date);
    request.add_header("x-ms-version: " + m_xms_version);
    request.set_auth_header("SharedKey " + m_storage_provider.access_id + ":" + get_ms_signature());
    request.accept("application/json");

    send_http_request(request);

    std::cout << "HTTP Status: " << get_http_status() << '\n';
    std::cout << get_response() << '\n';

    if ( get_http_status() != 200 && get_http_status() != 201 ) {
        return "";
    }

    return get_response();

}


std::string AzureClient::api_get_signature()
{

    std::unique_ptr<HttpClient> vessel = std::make_unique<HttpClient>( m_ldb->get_setting_str("master_server") );

    HttpRequest request;
    request.set_method("POST");
    request.set_auth_header( "Bearer " + m_ldb->get_setting_str("client_token") );
    request.set_content_type("application/json");
    request.accept("application/json");
    request.set_url( m_ldb->get_setting_str("vessel_api_path") + "/upload/azure/sign");
    request.set_body("{\"providerId\" : \"" + m_storage_provider.provider_id + "\", \"stringToSign\" : \"" + Hash::get_base64(get_string_to_sign()) + "\"}");

    //Send the API Request
    vessel->send_http_request(request);

    //Parse the signing key
    Document document;
    document.Parse( vessel->get_response().c_str() );

    Log::get_log().add_message( vessel->get_response(), "Azure");

    if ( !document.HasMember("signature") ) {
        throw AzureException( AzureException::BadSignature, "Unable to parse signature from API");
    }

    //Log::get_log().add_message("Obtained signing key: " + signing_key + " from API", "File Upload");

    return document["signature"].GetString();

}
