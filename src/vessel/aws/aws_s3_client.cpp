#include <vessel/aws/aws_s3_client.hpp>

using namespace Vessel::Networking;
using namespace Vessel::Utilities;

AwsS3Client::AwsS3Client(const std::string& uri) : HttpClient(uri), m_reduced_redundancy(true)
{

    m_ldb = &Vessel::Database::LocalDatabase::get_database();
    m_http_verb = "PUT"; //Default for uploading new files
    m_region = "us-east-2"; //Default
    m_part_size = Vessel::File::BackupFile::get_chunk_size();
    init_amz_date();

}

std::string AwsS3Client::get_canonical_request()
{

    std::stringstream ss;

    ss << m_http_verb << "\n";
    ss << "/" << encode_uri(get_uri_path() + m_file->get_file_name()) << "\n";
    ss << m_query_str << "\n";
    ss << get_amz_headers() << "\n";
    ss << get_signed_headers() << "\n";
    ss << m_content_sha256;

    //std::cout << "Canonical request: " << ss.str() << "\n";

    return ss.str();
}

std::string AwsS3Client::get_string_to_sign()
{

    using namespace Vessel::Utilities;

    //Get SHA256 hash of canonical request
    std::string cr_hash = Hash::get_sha256_hash(get_canonical_request());

    std::ostringstream oss;
    oss << "AWS4-HMAC-SHA256\n";
    oss << m_amzdate << "\n";
    oss << m_amzdate_short << "/" << m_region << "/s3/aws4_request\n";
    oss << cr_hash;

    return oss.str();

}

std::string AwsS3Client::get_signing_key()
{
    using namespace Vessel::Utilities;

    std::string secret = m_secret_key;
    std::string key_date = Hash::get_hmac_256("AWS4" + secret, m_amzdate_short, false);
    std::string key_region = Hash::get_hmac_256(key_date, m_region, false);
    std::string key_service = Hash::get_hmac_256(key_region, "s3", false);
    std::string key_signing = Hash::get_hmac_256(key_service, "aws4_request", false);

    return key_signing;
}

std::string AwsS3Client::get_signature_v4()
{
    using namespace Vessel::Utilities;

    //Signature calculation from signing key
    return Hash::get_hmac_256(get_signing_key(), get_string_to_sign());

}

std::string AwsS3Client::get_stream_signature_v4(const std::string& prev_signature)
{
    using namespace Vessel::Utilities;

    //If it's the first part use the seed signature
    if ( m_current_part <= 1 ) {
        return get_signature_v4();
    }

    //String to sign for Chunked uploads
    std::ostringstream oss;
    oss << "AWS4-HMAC-SHA256-PAYLOAD\n";
    oss << m_amzdate << "\n";
    oss << m_amzdate_short << "/" << m_region << "/s3/aws4_request\n";
    oss << prev_signature << "\n";
    oss << Hash::get_sha256_hash("") << "\n";
    oss << Hash::get_sha256_hash(&m_file->get_file_part(m_current_part)[0]);

    //Return the signature using the parted version of the string to sign
    return Hash::get_hmac_256(get_signing_key(), oss.str());

}

void AwsS3Client::init_upload(Vessel::File::BackupFile* bf, AwsFlags flags )
{

    //Reset Part #
    m_current_part = 0;

    //Clear existing file content
    m_file_content.reset();

    //Refresh the date/time vars
    init_amz_date();

    m_file = bf;

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
        m_file_content = std::make_shared<std::string>( m_file->get_file_contents() );

        //Get the SHA256 hash of the current payload, in this case - the entire file contents
        m_content_sha256 = m_file->get_hash_sha256();

        //Rebuild the request headers
        build_request_headers();
    }

}

void AwsS3Client::build_request_headers()
{

    //Clear any existing headers
    m_headers.clear();

    //We always build these headers
    m_headers.insert ( std::pair<std::string,std::string>("Host", get_hostname()) );
    m_headers.insert ( std::pair<std::string,std::string>("x-amz-date", m_amzdate) );
    m_headers.insert ( std::pair<std::string,std::string>("x-amz-content-sha256", m_content_sha256 ) );

    //If the current part == 0, init the upload
    if ( m_current_part == 0 )
    {

        //m_headers.insert ( std::pair<std::string,std::string>("Cache-Control", "no-cache") );

        if ( !m_file->get_mime_type().empty() ) {
            m_headers.insert ( std::pair<std::string,std::string>("Content-Encoding", m_file->get_mime_type()) );
            m_headers.insert ( std::pair<std::string,std::string>("Content-Type", m_file->get_mime_type()) );

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
        m_headers.insert ( std::pair<std::string,std::string>("Content-Encoding", (m_file->get_mime_type().empty() ? "aws-chunked" : ("aws-chunked," + m_file->get_mime_type()) ) ) );
        m_headers.insert ( std::pair<std::string,std::string>("Expect", "100-continue") );
        m_headers.insert ( std::pair<std::string,std::string>("x-amz-decoded-content-length", std::to_string( m_file->get_file_size() ) ) );

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
    boost::posix_time::time_facet* input_facet = new boost::posix_time::time_facet();
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

    //Build the raw request
    HttpRequestStream http_request;

    http_request << m_http_verb << " /" << encode_uri(m_file->get_file_name()) << " HTTP/1.1\r\n";
    http_request << "Host: " << get_hostname() << "\r\n";
    http_request << "Date: " << m_amzdate_clean << "\r\n";
    http_request << "Authorization: AWS4-HMAC-SHA256 Credential=" << m_access_id << "/" << m_amzdate_short << "/" << m_region << "/s3/aws4_request,SignedHeaders=" << get_signed_headers() << ",Signature=" << get_signature_v4() << "\r\n";
    http_request << "Content-Length: " << m_file->get_file_size() << "\r\n";

    if ( !m_file->get_mime_type().empty() ) {
        http_request << "Content-Encoding: " << m_file->get_mime_type() << "\r\n";
        http_request << "Content-Type: " << m_file->get_mime_type() << "\r\n";
    }

    http_request << "x-amz-content-sha256: " << m_content_sha256 << "\r\n";

    if ( m_reduced_redundancy ) {
        http_request << "x-amz-storage-class: REDUCED_REDUNDANCY" << "\r\n";
    }

    http_request << "x-amz-date: " << m_amzdate << "\r\n";
    http_request << "Cache-Control: " << "no-cache" << "\r\n";
    http_request << "Connection: close\r\n\r\n";
    http_request << *m_file_content;

    //Connect to socket
    connect();

    //Write to socket
    write_socket(http_request.str());

    do { run_io_service(); std::cout << "..." << '\n'; } while ( !get_error_code() );

    disconnect();

    //Empty file content from memory
    m_file_content.reset();

    return true;

}

void AwsS3Client::init_multipart_upload()
{

    std::string signature = get_signature_v4();

    //Send a request to AWS to Initialize the Multipart Upload

    //Build the raw request
    HttpRequestStream http_request;

    http_request << "POST /" << encode_uri(m_file->get_file_name()) << "?uploads" << " HTTP/1.1\r\n";
    http_request << "Host: " << get_hostname() << "\r\n";
    http_request << "Date: " << m_amzdate_clean << "\r\n";
    http_request << "Authorization: AWS4-HMAC-SHA256 Credential=" << m_access_id << "/" << m_amzdate_short << "/" << m_region << "/s3/aws4_request,SignedHeaders=" << get_signed_headers() << ",Signature=" << signature << "\r\n";

    if ( !m_file->get_mime_type().empty() ) {
        http_request << "Content-Type: " << m_file->get_mime_type() << "\r\n";
        http_request << "Content-Encoding: " << m_file->get_mime_type() << "\r\n";
    }

    http_request << "x-amz-content-sha256: " << m_content_sha256 << "\r\n";
    http_request << "x-amz-date: " << m_amzdate << "\r\n";

    if ( m_reduced_redundancy ) {
        http_request << "x-amz-storage-class: REDUCED_REDUNDANCY" << "\r\n";
    }

    //http_request << "Cache-Control: " << "no-cache" << "\r\n";
    http_request << "Connection: close\r\n\r\n";

    //Connect to socket
    connect();

    //Write to socket
    write_socket(http_request.str());

    do { run_io_service(); std::cout << "..." << '\n'; } while ( !get_error_code() );

    disconnect();

    std::cout << "Init Multipart Upload Response:\n" << get_response() << '\n';
    std::cin.get();

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
    m_file_content = std::make_shared<std::string>( m_file->get_file_part(m_current_part) );

    m_content_sha256 =  Hash::get_sha256_hash(*m_file_content);
    m_content_md5 = Hash::get_md5_hash(*m_file_content, true);
    m_query_str = "partNumber=" + std::to_string(part) + "&uploadId=" + encode_uri(upload_id);

    //Refresh the date/time vars
    init_amz_date();

    //Rebuild the request headers
    build_request_headers();

    //Build the HTTP request
    HttpRequestStream http_request;

    http_request.append(m_http_verb + " /" + encode_uri(m_file->get_file_name()) + "?partNumber=" + std::to_string(part) + "&uploadId=" + encode_uri(upload_id) + " HTTP/1.1\r\n");
    http_request.append("Host: " + get_hostname() + "\r\n");
    http_request.append("Date: " + m_amzdate_clean + "\r\n");
    http_request.append("Authorization: AWS4-HMAC-SHA256 Credential=" + m_access_id + "/" + m_amzdate_short + "/" + m_region + "/s3/aws4_request,SignedHeaders=" + get_signed_headers() + ",Signature=" + get_signature_v4() + "\r\n");
    http_request.append("Content-Length: " + std::to_string(m_file_content->size()) + "\r\n");
    http_request.append("Content-MD5: " + m_content_md5 + "\r\n");
    http_request.append("x-amz-content-sha256: " + m_content_sha256 + "\r\n");
    http_request.append("x-amz-date: " + m_amzdate + "\r\n");
    //http_request.append("Cache-Control: no-cache" + "\r\n");
    //http_request.append("Expect: 100-continue\r\n");
    http_request.append("Connection: close\r\n");

    //TODO: Optional Encryption

    http_request.append("\r\n");
    http_request.append(*m_file_content);

    //Clear file content - free some memory
    m_file_content.reset();

    //Connect to socket
    connect();

    //Write to socket
    write_socket( http_request.str() );

    do { run_io_service(); std::cout << "..." << '\n'; } while ( !get_error_code() );

    disconnect();

    //Parse the ETag from the response
    return parse_etag();

}

void AwsS3Client::set_file(Vessel::File::BackupFile* bf)
{
    m_file = bf;
}

size_t AwsS3Client::get_current_part_size()
{

    if ( m_multipart )
    {
        size_t part_size = (m_part_size > 0) ? m_part_size : m_file->get_chunk_size();
        size_t byte_index = part_size * m_current_part;
        size_t overflow = (byte_index > m_file->get_file_size()) ? byte_index-m_file->get_file_size() : 0;
        return (part_size - overflow);
    }

    return m_file->get_file_size(); //Default

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
        //Log error here
        throw AwsException(AwsException::XmlParseError, e.what());
        //std::cout << "XML Parser error: " << e.what() << '\n';
        return "";
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

std::string AwsS3Client::complete_multipart_upload(const std::vector<etag_pair>& etags, const std::string& upload_id)
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
        node.put("ETag", etags[i].etag );
    }

    write_xml(oss, pt, xml_parser::xml_writer_make_settings<std::string>(' ',4) );

    //XMl payload to send to the API
    std::string payload = oss.str();

    std::cout << "CompleteMultipartUploadRequest:\n" << payload << '\n';

    //Send XML payload to the API and complete the upload

    m_current_part = -1; //Skip additional headers

    //Set HTTP verb for part upload
    m_http_verb = "POST";

    //Set the MD5 of the current part
    m_file_content = std::make_shared<std::string>( payload );
    m_content_sha256 =  Hash::get_sha256_hash(*m_file_content);
    m_query_str = "uploadId=" + encode_uri(upload_id);

    //Refresh the date/time vars
    init_amz_date();

    //Rebuild the request headers
    build_request_headers();

    //Build the raw request
    HttpRequestStream http_request;

    http_request << m_http_verb << " /" << encode_uri(m_file->get_file_name()) << "?uploadId=" << encode_uri(upload_id) << " HTTP/1.1\r\n";
    http_request << "Host: " << get_hostname() << "\r\n";
    http_request << "Date: " << m_amzdate_clean << "\r\n";
    http_request << "Authorization: AWS4-HMAC-SHA256 Credential=" << m_access_id << "/" << m_amzdate_short << "/" << m_region << "/s3/aws4_request,SignedHeaders=" << get_signed_headers() << ",Signature=" << get_signature_v4() << "\r\n";
    http_request << "Content-Length: " << m_file_content->size() << "\r\n";
    http_request << "x-amz-content-sha256: " << m_content_sha256 << "\r\n";
    http_request << "x-amz-date: " << m_amzdate << "\r\n";
    http_request << "Connection: close\r\n\r\n";
    http_request << *m_file_content; //XML payload

    //Connect to socket
    connect();

    //Write to socket
    write_socket( http_request.str() );

    do { run_io_service(); std::cout << "..." << '\n'; } while ( !get_error_code() );

    disconnect();

    //Clear content
    m_file_content.reset();

    //Parse the response and return the ETag
    std::string response = get_response();

    std::cout << "Complete Multipart Upload Response:\n" << response << '\n';

    //Handle Empty Response
    if ( response.empty() ) {
        throw AwsException( AwsException::BadResponse, "Invalid Complete Multipart Upload Response");
        return "";
    }

    std::stringstream iss; //Response stream
    iss << response;

    ptree rt;
    boost::property_tree::read_xml(iss, rt);

    etag = rt.get<std::string>("CompleteMultipartUploadResult.ETag");

    return etag;

}

void AwsS3Client::set_auth_profile(AuthProfile profile)
{
    if ( profile == AuthProfile::Local ) {
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
        return;
    }

    /**
     ** Line 1 = AWS Access ID
     ** Line 2 = AWS Secret Key
     ** Line 3 = AWS Region
    */

    std::string tmp;

    if ( std::getline(infile, tmp) ) {
        m_access_id = tmp;
    }

    if ( std::getline(infile, tmp) ) {
        m_secret_key = tmp;
    }

    if ( std::getline(infile, tmp) ) {
        m_region = tmp;
    }

    infile.close();

}
