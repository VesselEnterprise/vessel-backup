#include "aws_s3_client.hpp"

using namespace Backup::Networking;

AwsS3Client::AwsS3Client(const std::string& uri) : HttpClient(uri)
{

    m_ldb = &Backup::Database::LocalDatabase::get_database();
    m_http_verb = "PUT"; //Default for uploading new files
    m_region = AWS_REGION;
    init_amz_date();

}

std::string AwsS3Client::get_canonical_request()
{

    //Build the request headers
    build_request_headers();

    std::stringstream ss;

    ss << m_http_verb << "\n";
    ss << encode_uri(get_uri_path() + m_file->get_file_name()) << "\n";
    ss << m_query_str << "\n";
    ss << get_amz_headers() << "\n";
    ss << get_signed_headers() << "\n";
    ss << m_content_sha256;

    return ss.str();
}

std::string AwsS3Client::get_string_to_sign()
{

    using namespace Backup::Utilities;

    //Get SHA256 hash of canonical request
    std::string cr_hash = Hash::get_sha256_hash(get_canonical_request());

    std::ostringstream oss;

    oss << "AWS4-HMAC-SHA256\n";
    oss << m_amzdate << "\n";
    oss << m_amzdate_short << "/" << m_region << "/s3/aws4_request\n";
    oss << cr_hash;

    return oss.str();

}

std::string AwsS3Client::get_signature_v4()
{
    using namespace Backup::Utilities;

    std::string secret = AWS_SECRET_KEY;
    std::string key_date = Hash::get_hmac_256("AWS4" + secret, m_amzdate_short, false);
    std::string key_region = Hash::get_hmac_256(key_date, m_region, false);
    std::string key_service = Hash::get_hmac_256(key_region, "s3", false);
    std::string key_signing = Hash::get_hmac_256(key_service, "aws4_request", false);

    //Signature calculation from signing key
    return Hash::get_hmac_256(key_signing, get_string_to_sign());

}

void AwsS3Client::set_file(Backup::File::BackupFile* bf )
{
    m_file = bf;

    //Get the SHA-256 hash of the file contents and save locally
    m_content_sha256 = m_file->get_hash_sha256();
}

void AwsS3Client::build_request_headers()
{

    //Clear any existing headers
    m_headers.clear();

    //Add amz headers to the map
    m_headers.insert ( std::pair<std::string,std::string>("Host", get_hostname()) );

    if ( m_chunked )
    {
        m_headers.insert ( std::pair<std::string,std::string>("Content-Encoding", "aws-chunked") );
        m_headers.insert ( std::pair<std::string,std::string>("Expect", "100-continue") );
        m_headers.insert ( std::pair<std::string,std::string>("x-amz-decoded-content-length", std::to_string(m_file->get_file_size()) ) );
    }


    if ( !m_file->get_mime_type().empty() )
        m_headers.insert ( std::pair<std::string,std::string>("Content-Type", m_file->get_mime_type()) );

    m_headers.insert ( std::pair<std::string,std::string>("Content-Length", std::to_string(m_file->get_file_size()) ) );
    m_headers.insert ( std::pair<std::string,std::string>("x-amz-content-sha256", m_content_sha256 ) );
    m_headers.insert ( std::pair<std::string,std::string>("x-amz-date", m_amzdate) );

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

void AwsS3Client::set_chunked(bool flag)
{
    m_chunked=flag;
}

bool AwsS3Client::upload()
{
    //Refresh the date/time vars
    init_amz_date();

    //Build the raw request
    std::stringstream ss;

    ss << m_http_verb << " /" << m_file->get_file_name() << " HTTP/1.1\r\n";
    ss << "Host: " << get_hostname() << "\r\n";
    ss << "Date: " << m_amzdate_clean << "\r\n";
    ss << "Authorization: AWS4-HMAC-SHA256 Credential=" << AWS_ACCESS_ID << "/" << m_amzdate_short << "/" << m_region << "/s3/aws4_request,SignedHeaders=" << get_signed_headers() << ",Signature=" << get_signature_v4() << "\r\n";
    ss << "Content-Length: " << m_file->get_file_size() << "\r\n";

    if ( !m_file->get_mime_type().empty() )
        ss << "Content-Type: " << m_file->get_mime_type() << "\r\n";

    ss << "x-amz-content-sha256: " << m_content_sha256 << "\r\n";
    ss << "x-amz-date: " << m_amzdate << "\r\n";
    //ss << "Accept: application/json\r\n";
    ss << "Connection: close\r\n\r\n";
    ss.write( &m_file->get_file_contents()[0], m_file->get_file_size() );

    std::cout << "Sending AWS Request:\n" << ss.str() << "\n";

    //Connect to socket
    connect();

    //Write to socket
    write_socket(&ss.str()[0]);

    do { run_io_service(); std::cout << "..." << '\n'; } while ( !get_error_code() );

    disconnect();
}

size_t AwsS3Client::get_current_part_size()
{

    if ( m_chunked )
    {
        size_t chunk_size = m_file->get_chunk_size();
        size_t remainder_bytes = m_file->get_file_size() - (chunk_size * m_file->get_total_parts());
        return (chunk_size-remainder_bytes);
    }

    return m_file->get_file_size(); //Default

}
