#include <vessel/aws/aws_s3_client.hpp>

AwsS3Client::AwsS3Client(const StorageProvider& provider) : HttpClient(provider.server), m_storage_provider(provider), m_reduced_redundancy(true)
{
    m_ldb = &LocalDatabase::get_database();
    m_http_verb = "PUT"; //Default for uploading new files
    m_part_size = BackupFile::get_chunk_size();
    m_remote_signing=true;
    m_user_id = m_ldb->get_setting_str("user_id");
    init_amz_date();
}

std::string AwsS3Client::get_canonical_request()
{

    std::stringstream ss;

    ss << m_http_verb << "\n";
    ss << "/" << encode_uri( get_file_uri_path() ) << "\n";
    ss << m_query_str << "\n";
    ss << get_amz_headers() << "\n";
    ss << get_signed_headers() << "\n";
    ss << m_content_sha256;

    //std::cout << "Canonical request: " << ss.str() << "\n";

    return ss.str();
}

std::string AwsS3Client::get_string_to_sign()
{

    //Get SHA256 hash of canonical request
    std::string cr_hash = Hash::get_sha256_hash(get_canonical_request());

    std::ostringstream oss;
    oss << "AWS4-HMAC-SHA256\n";
    oss << m_amzdate << "\n";
    oss << m_amzdate_short << "/" << m_storage_provider.region << "/s3/aws4_request\n";
    oss << cr_hash;

    //std::cout << "StringToSign:" << oss.str() << '\n';

    return oss.str();

}

std::string AwsS3Client::get_signing_key()
{

    if ( m_remote_signing )
    {
        return api_get_signing_key();
    }

    std::string secret = m_storage_provider.access_key;
    std::string key_date = Hash::get_hmac_256("AWS4" + secret, m_amzdate_short, false);
    std::string key_region = Hash::get_hmac_256(key_date, m_storage_provider.region, false);
    std::string key_service = Hash::get_hmac_256(key_region, "s3", false);
    std::string key_signing = Hash::get_hmac_256(key_service, "aws4_request", false);

    return key_signing;
}

std::string AwsS3Client::api_get_signing_key()
{

    std::unique_ptr<HttpClient> vessel = std::make_unique<HttpClient>( m_ldb->get_setting_str("master_server") );

    HttpRequest request;
    request.set_method("POST");
    request.set_auth_header( "Bearer " + m_ldb->get_setting_str("client_token") );
    request.set_content_type("application/json");
    request.accept("application/json");
    request.set_url( m_ldb->get_setting_str("vessel_api_path") + "/upload/aws/sign");
    request.set_body("{\"providerId\" : \"" + m_storage_provider.provider_id + "\", \"amzDate\" : \"" + m_amzdate_short + "\"}");

    //Send the API Request
    vessel->send_http_request(request);

    //Parse the signing key
    Document document;
    document.Parse( vessel->get_response().c_str() );

    Log::get_log().add_message( vessel->get_response(), "AWS");

    if ( !document.HasMember("key") ) {
        throw AwsException( AwsException::BadSigningKey, "Unable to parse signing key from API");
    }

    std::string signing_key = document["key"].GetString();

    //Log::get_log().add_message("Obtained signing key: " + signing_key + " from API", "File Upload");

    return Hash::base64_decode(signing_key);
}

std::string AwsS3Client::get_signature_v4()
{
    //Signature calculation from signing key
    return Hash::get_hmac_256(get_signing_key(), get_string_to_sign());

}

std::string AwsS3Client::get_stream_signature_v4(const std::string& prev_signature)
{
    //If it's the first part use the seed signature
    if ( m_current_part <= 1 ) {
        return get_signature_v4();
    }

    //String to sign for Chunked uploads
    std::ostringstream oss;
    oss << "AWS4-HMAC-SHA256-PAYLOAD\n";
    oss << m_amzdate << "\n";
    oss << m_amzdate_short << "/" << m_storage_provider.region << "/s3/aws4_request\n";
    oss << prev_signature << "\n";
    oss << Hash::get_sha256_hash("") << "\n";
    oss << Hash::get_sha256_hash(&m_file.get_file_part(m_current_part)[0]);

    //Return the signature using the parted version of the string to sign
    return Hash::get_hmac_256(get_signing_key(), oss.str());

}

bool AwsS3Client::init_upload(const BackupFile& bf, AwsFlags flags )
{

    //Reset Part #
    m_current_part = 0;

    //Clear existing file content
    m_file_content.reset();

    //Refresh the date/time vars
    init_amz_date();

    m_file = bf;

    //Build the relative path on the cloud server
    build_file_uri_path();

    m_multipart = (flags & AwsFlags::Multipart);
    m_reduced_redundancy = (flags & AwsFlags::ReducedRedundancy);
    m_streaming = (flags & AwsFlags::Streaming);

    //For streaming uploads
    if ( m_streaming ) {
        m_content_sha256 = "STREAMING-AWS4-HMAC-SHA256-PAYLOAD";
    }

    if ( m_multipart )
    {
        //HTTP presets for headers
        m_http_verb = "POST";
        m_query_str = "uploads=";

        //When initializing a Multipart upload, it's a SHA256 hash of empty string
        m_content_sha256 = Hash::get_sha256_hash("");

        //Rebuild the request headers
        build_request_headers();

        //Sets the internal upload id
        if ( !(flags & AwsFlags::SkipMultiInit) ) {
            init_multipart_upload();
        }
    }
    else
    {
        //HTTP presets for headers
        m_http_verb = "PUT";
        m_query_str = "";

        //Set the file content
        m_file_content = std::make_shared<std::string>( m_file.get_file_contents() );

        //Get the SHA256 hash of the current payload, in this case - the entire file contents
        m_content_sha256 = m_file.get_hash_sha256();

        //Rebuild the request headers
        build_request_headers();
    }

    return true;

}

void AwsS3Client::build_request_headers()
{

    //Clear any existing headers
    m_headers.clear();

    //We always build these headers
    m_headers.insert ( std::pair<std::string,std::string>("Host", get_hostname() + ":" + std::to_string(get_port()) ) );
    m_headers.insert ( std::pair<std::string,std::string>("x-amz-date", m_amzdate) );
    m_headers.insert ( std::pair<std::string,std::string>("x-amz-content-sha256", m_content_sha256 ) );

    //If the current part == 0, init the upload
    if ( m_current_part == 0 )
    {

        //m_headers.insert ( std::pair<std::string,std::string>("Cache-Control", "no-cache") );

        if ( !m_file.get_mime_type().empty() ) {
            m_headers.insert ( std::pair<std::string,std::string>("Content-Encoding", m_file.get_mime_type()) );
            m_headers.insert ( std::pair<std::string,std::string>("Content-Type", m_file.get_mime_type()) );

        }

        if ( m_reduced_redundancy ) {
                m_headers.insert ( std::pair<std::string,std::string>("x-amz-storage-class", "REDUCED_REDUNDANCY" ) );
        }

        //m_headers.insert ( std::pair<std::string,std::string>("Expires", "") ); //TODO
        //Other considerations: x-amz-meta- , x-amz-tagging
    }
    else if ( m_current_part > 0 && !m_streaming ) //Multipart upload part
    {
        //m_headers.insert ( std::pair<std::string,std::string>("Cache-Control", "no-cache") );
        m_headers.insert ( std::pair<std::string,std::string>("Content-Length", std::to_string( m_file_content->size() ) ) );
        m_headers.insert ( std::pair<std::string,std::string>("Content-MD5", m_content_md5 ) );
        //m_headers.insert ( std::pair<std::string,std::string>("Expect", "100-continue") );
    }

    //Build these headers for streaming uploads
    if ( m_streaming )
    {
        m_headers.insert ( std::pair<std::string,std::string>("Content-Encoding", (m_file.get_mime_type().empty() ? "aws-chunked" : ("aws-chunked," + m_file.get_mime_type()) ) ) );
        m_headers.insert ( std::pair<std::string,std::string>("Expect", "100-continue") );
        m_headers.insert ( std::pair<std::string,std::string>("x-amz-decoded-content-length", std::to_string( m_file_content->size() ) ) );

        if ( m_reduced_redundancy ) {
                m_headers.insert ( std::pair<std::string,std::string>("x-amz-storage-class", "REDUCED_REDUNDANCY" ) );
        }

    }

}

std::string AwsS3Client::get_amz_headers()
{

    //Header stream
    std::ostringstream ss;

    for (std::map<std::string,std::string>::iterator it=m_headers.begin(); it!=m_headers.end(); ++it)
    {
        ss << boost::to_lower_copy(it->first) << ":" << it->second << "\n";
        /*
        if ( std::next(it) != m_headers.end() )
            ss << "\n";
        */
    }

    return ss.str();

}

std::string AwsS3Client::get_signed_headers()
{
    //Signed Header stream
    std::ostringstream ss;

    for (std::map<std::string,std::string>::iterator it=m_headers.begin(); it!=m_headers.end(); ++it)
    {
        ss << boost::to_lower_copy(it->first);
        if ( std::next(it) != m_headers.end() )
            ss << ";";
    }

    return ss.str();
}

void AwsS3Client::init_amz_date()
{
    //Generate the short version of the amz date
    std::stringstream ss;
    boost::posix_time::ptime pt( boost::posix_time::second_clock::universal_time() );
    boost::posix_time::time_facet* input_facet = new boost::posix_time::time_facet(); //No memory leak here
    input_facet->format("%Y%m%d");
    ss.imbue( std::locale(ss.getloc(), input_facet ) );
    ss << pt;

    //Set the ISO8601 date
    m_amzdate = (boost::posix_time::to_iso_string(pt) + "Z");

    //Set the short version of the date
    m_amzdate_short = ss.str();

    //Fri, 24 May 2013 00:00:00 GMT
    ss.str("");
    input_facet->format("%a, %d %b %Y %T GMT");
    ss.imbue( std::locale(ss.getloc(), input_facet ) );
    ss << pt;

    //Set the GMT Clean Date
    m_amzdate_clean = ss.str();

}

bool AwsS3Client::upload()
{

    //TODO: Migrate to using HttpRequest
    HttpRequest request;
    request.set_method("PUT");
    request.set_url("/" + encode_uri( get_file_uri_path() ));
    request.add_header("Date: " + m_amzdate_clean);
    request.set_auth_header("AWS4-HMAC-SHA256 Credential=" + m_storage_provider.access_id + "/" + m_amzdate_short + "/" + m_storage_provider.region + "/s3/aws4_request,SignedHeaders=" + get_signed_headers() + ",Signature=" + get_signature_v4() );

    if ( !m_file.get_mime_type().empty() )
    {
        request.add_header("Content-Encoding: " + m_file.get_mime_type());
        request.add_header("Content-Type: " + m_file.get_mime_type());
    }

    request.add_header("x-amz-content-sha256: " + m_content_sha256);

    if ( m_reduced_redundancy )
    {
        request.add_header("x-amz-storage-class: REDUCED_REDUNDANCY");
    }

    request.add_header("x-amz-date: " + m_amzdate);

    request.set_body(*m_file_content);

    //Upload the file
    int status = send_http_request(request);

    //Empty file content from memory
    m_file_content.reset();

    if ( status != 200 && status != 201 ) {
        return false;
    }

    return true;

}

void AwsS3Client::init_multipart_upload()
{

    std::string signature = get_signature_v4();

    //Send a request to AWS to Initialize the Multipart Upload

    HttpRequest request;
    request.set_method("POST");
    request.set_url("/" + encode_uri( get_file_uri_path() ) + "?uploads");
    request.add_header("Date: " + m_amzdate_clean);
    request.set_auth_header("AWS4-HMAC-SHA256 Credential=" + m_storage_provider.access_id + "/" + m_amzdate_short + "/" + m_storage_provider.region + "/s3/aws4_request,SignedHeaders=" + get_signed_headers() + ",Signature=" + signature);

    if ( !m_file.get_mime_type().empty() )
    {
        request.add_header("Content-Type: " + m_file.get_mime_type());
        request.add_header("Content-Encoding: " + m_file.get_mime_type());
    }

    request.add_header("x-amz-content-sha256: " + m_content_sha256);
    request.add_header("x-amz-date: " + m_amzdate);

    if ( m_reduced_redundancy )
    {
        request.add_header("x-amz-storage-class: REDUCED_REDUNDANCY");
    }

    //Send the request
    send_http_request(request);

    //Parse the response and get the upload ID here
    m_upload_id = parse_upload_id( get_response() );

}

std::string AwsS3Client::upload_part(int part, const std::string& upload_id )
{

    //Set part index
    m_current_part = part;

    //Set HTTP verb for part upload
    m_http_verb = "PUT";

    //Set the MD5 of the current part
    m_file_content = std::make_shared<std::string>( m_file.get_file_part(m_current_part) );

    m_content_sha256 =  Hash::get_sha256_hash(*m_file_content);
    m_content_md5 = Hash::get_md5_hash(*m_file_content, true);
    m_query_str = "partNumber=" + std::to_string(part) + "&uploadId=" + encode_uri(upload_id);

    //Refresh the date/time vars
    init_amz_date();

    //Rebuild the request headers
    build_request_headers();

    HttpRequest request;
    request.set_method("PUT");
    request.set_url("/" + encode_uri( get_file_uri_path() ) + "?partNumber=" + std::to_string(part) + "&uploadId=" + encode_uri(upload_id) );
    request.add_header("Date: " + m_amzdate_clean);
    request.add_header("Content-MD5: " + m_content_md5);
    request.add_header("x-amz-content-sha256: " + m_content_sha256);
    request.add_header("x-amz-date: " + m_amzdate);
    request.set_auth_header("AWS4-HMAC-SHA256 Credential=" + m_storage_provider.access_id + "/" + m_amzdate_short + "/" + m_storage_provider.region + "/s3/aws4_request,SignedHeaders=" + get_signed_headers() + ",Signature=" + get_signature_v4() );
    request.set_body(*m_file_content);

    //Clear file content - free some memory
    m_file_content.reset();

    //Send the request
    send_http_request(request);

    //Parse the ETag from the response
    return parse_etag();

}

void AwsS3Client::set_file(const BackupFile& bf)
{
    m_file = bf;
}

size_t AwsS3Client::get_current_part_size()
{

    if ( m_multipart )
    {
        size_t part_size = (m_part_size > 0) ? m_part_size : m_file.get_file_size();
        size_t byte_index = part_size * m_current_part;
        size_t overflow = ( byte_index > m_file.get_file_size() ) ? ( byte_index - m_file.get_file_size() ) : 0;
        return (part_size - overflow);
    }

    return m_file.get_file_size(); //Default

}

std::string AwsS3Client::get_last_signature()
{
    return m_previous_signature;
}

void AwsS3Client::set_part_size(size_t part_size)
{
    m_part_size = part_size;
}

std::string AwsS3Client::get_upload_id()
{
    return m_upload_id;
}

void AwsS3Client::set_upload_id(const std::string& upload_id)
{
    m_upload_id = upload_id;
}

std::string AwsS3Client::get_5mb_test_str()
{

    std::string test_string;
    std::string pattern = "Test";
    size_t total_bytes = 5 * 1024 * 1024;
    size_t total_copies = static_cast< uint32_t >( total_bytes / pattern.size() + 1 );

    for ( size_t i=0; i < total_copies; i++ )
    {
        test_string.append(pattern);
    }

    return test_string;

}

std::string AwsS3Client::parse_upload_id(const std::string& response)
{

    std::string upload_id;

    //Read response into a stream
    std::stringstream ss;
    ss << response;

    //Create a boost ptree
    boost::property_tree::ptree pt;

    //Read the XML
    try
    {
        boost::property_tree::read_xml( ss, pt );
        upload_id = pt.get<std::string>("InitiateMultipartUploadResult.UploadId");
    }
    catch ( const boost::property_tree::ptree_error& e )
    {
        throw AwsException(AwsException::XmlParseError, e.what());
    }

    //Example payload
    /*
    <?xml version="1.0" encoding="UTF-8"?>
    <InitiateMultipartUploadResult xmlns="http://s3.amazonaws.com/doc/2006-03-01/">
        <Bucket>example-bucket</Bucket>
        <Key>example-object</Key>
        <UploadId>VXBsb2FkIElEIGZvciA2aWWpbmcncyBteS1tb3ZpZS5tMnRzIHVwbG9hZA</UploadId>
    </InitiateMultipartUploadResult>
    */

    return upload_id;

}

std::string AwsS3Client::parse_etag()
{

    std::string etag;

    //Find the ETag in the response header
    std::istringstream iss(get_headers());
    std::string header;

    while ( getline(iss, header) && header != "\r" )
    {
        if ( header.find("ETag: ") != std::string::npos ) {
            etag = header.substr(6); //Everything after "ETag: "
            break;
        }
    }

    //Replace any remaining spaces or carriage returns
    boost::algorithm::replace_all(etag, " ", "");
    boost::algorithm::replace_all(etag, "\r", "");
    boost::algorithm::replace_all(etag, "\n", "");

    return etag;

}

std::string AwsS3Client::complete_multipart_upload(const std::vector<UploadTagSet>& etags, const std::string& upload_key)
{

    using namespace boost::property_tree;

    //ETag returned from the API for the final upload
    std::string etag;

    /*Example Payload

    <CompleteMultipartUpload>
      <Part>
        <PartNumber>1</PartNumber>
        <ETag>"6000ec57db7b1c7f039dd3a11b4117f1"</ETag>
      </Part>
      <Part>
        <PartNumber>2</PartNumber>
        <ETag>"06a9becc7448e147e64b6a043b582725"</ETag>
      </Part>
      <Part>
        <PartNumber>3</PartNumber>
        <ETag>"82ac8fcfddd6b29f7bbc284078dcdbbd"</ETag>
      </Part>
    </CompleteMultipartUpload>
    */

    //XML storage
    ptree pt;
    auto& root_node = pt.add("CompleteMultipartUpload", "");

    //XML Output
    std::ostringstream oss;

    //Iterate through the Etags and build the XML payload
    for ( unsigned i=0; i < etags.size(); i++ )
    {
        auto& node = root_node.add_child("Part", ptree{});
        node.put("PartNumber", etags[i].part_number );
        node.put("ETag", etags[i].tag );
    }

    write_xml(oss, pt, xml_parser::xml_writer_make_settings<std::string>(' ',4) );

    //XMl payload to send to the API
    std::string payload = oss.str();

    //std::cout << "CompleteMultipartUploadRequest:\n" << payload << '\n';

    //Send XML payload to the API and complete the upload

    m_current_part = -1; //Skip additional headers

    //Set HTTP verb for part upload
    m_http_verb = "POST";

    //Set the MD5 of the current part
    m_file_content = std::make_shared<std::string>( payload );
    m_content_sha256 =  Hash::get_sha256_hash(*m_file_content);
    m_query_str = "uploadId=" + encode_uri(upload_key);

    //Refresh the date/time vars
    init_amz_date();

    //Rebuild the request headers
    build_request_headers();

    HttpRequest request;
    request.set_method("POST");
    request.set_url("/" + encode_uri( get_file_uri_path() ) + "?uploadId=" + encode_uri(upload_key));
    request.add_header("Date: " + m_amzdate_clean);
    request.add_header("x-amz-content-sha256: " + m_content_sha256);
    request.add_header("x-amz-date: " + m_amzdate);
    request.set_auth_header("AWS4-HMAC-SHA256 Credential=" + m_storage_provider.access_id + "/" + m_amzdate_short + "/" + m_storage_provider.region + "/s3/aws4_request,SignedHeaders=" + get_signed_headers() + ",Signature=" + get_signature_v4() );
    request.set_body(*m_file_content);

    //Send the request
    send_http_request(request);

    //Clear content
    m_file_content.reset();

    //Parse the response and return the ETag
    std::string response = get_response();

    //Handle Empty Response
    if ( response.empty() ) {
        throw AwsException( AwsException::BadResponse, "Invalid Complete Multipart Upload Response");
    }

    std::stringstream iss; //Response stream
    iss << response;

    ptree rt;
    boost::property_tree::read_xml(iss, rt);

    etag = rt.get<std::string>("CompleteMultipartUploadResult.ETag");

    return etag;

}

void AwsS3Client::remote_signing(bool flag)
{
    if ( !flag ) {
        read_key_file();
        m_remote_signing=false;
    }
    else {
        m_remote_signing=true;
    }
}

void AwsS3Client::read_key_file()
{
    std::ifstream infile("keys/aws.key", std::ios::in );
    if ( !infile.is_open() ) {
        throw AwsException(AwsException::InvalidCredentials, "Unable to open aws.key file");
    }

    /**
     ** Line 1 = Server URL
     ** Line 2 = AWS Access ID
     ** Line 3 = AWS Secret Key
     ** Line 4 = AWS Region
     ** Line 5 = AWS Bucket Name
    */

    std::string tmp;

    if ( std::getline(infile, tmp) ) {
        m_storage_provider.server = tmp;
    }

    if ( std::getline(infile, tmp) ) {
        m_storage_provider.access_id = tmp;
    }

    if ( std::getline(infile, tmp) ) {
        m_storage_provider.access_key = tmp;
    }
    if ( std::getline(infile, tmp) ) {
        m_storage_provider.region = tmp;
    }

    if ( std::getline(infile, tmp) ) {
        m_storage_provider.bucket_name = tmp;
    }

    infile.close();

}

void AwsS3Client::set_storage_provider( const StorageProvider& provider )
{
    m_storage_provider = provider;
}

void AwsS3Client::build_file_uri_path()
{

    m_uri_file_path = m_user_id + m_file.get_parent_path() + "/" + m_file.get_file_name();

    #ifdef _WIN32
        boost::replace_all(m_uri_file_path, "\\", "/");
    #endif

}

std::string AwsS3Client::get_file_uri_path()
{
    return m_uri_file_path;
}
