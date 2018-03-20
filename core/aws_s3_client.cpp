#include "aws_s3_client.hpp"

using namespace Backup::Networking;

AwsS3Client::AwsS3Client(const std::string& uri) : HttpClient(uri)
{

    m_ldb = &Backup::Database::LocalDatabase::get_database();
    m_http_verb = "PUT"; //Default for uploading new files

}

std::string AwsS3Client::get_canonical_request()
{

    //Build the request headers
    build_request_headers();

    std::stringstream ss;

    ss << m_http_verb << "\n";
    ss << encode_uri(get_uri_path()) << "\n";
    ss << m_query_str << "\n";
    ss << get_amz_headers() << "\n";
    ss <<
}

void AwsS3Client::set_file(Backup::File::BackupFile* bf )
{
    m_file = bf;
}

void AwsS3Client::build_request_headers()
{

    //Get ISO 8601 Timestamp
    boost::posix_time::ptime t = boost::posix_time::microsec_clock::universal_time();
    std::string amzdate = (boost::posix_time::to_iso_extended_string(t) + "Z");

    //Clear any existing headers
    m_headers.clear();

    //Add amz headers to the map
    m_headers.insert ( std::pair<std::string,std::string>("Host", get_hostname()) );

    if ( m_chunked )
        m_headers.insert ( std::pair<std::string,std::string>("Expect", "100-continue") );

    if ( !m_file->get_mime_type().empty() )
        m_headers.insert ( std::pair<std::string,std::string>("Content-Type", m_file->get_mime_type()) );

    m_headers.insert ( std::pair<std::string,std::string>("Content-Length", std::to_string(m_file->get_file_size()) ) );
    m_headers.insert ( std::pair<std::string,std::string>("x-amz-content-sha256", m_file->get_hash_sha256() ) );
    m_headers.insert ( std::pair<std::string,std::string>("x-amz-date", amzdate) );

}

std::string AwsS3Client::get_amz_headers()
{

    //Header stream
    std::ostringstream ss;

    for (std::map<std::string,std::string>::iterator it=m_headers.begin(); it!=m_headers.end(); ++it)
    {
        ss << boost::to_lower_copy(it->first) << ":" << boost::to_lower_copy(it->second);
        if ( it+1 != m_headers.end() )
            ss << "\n";
    }

}

std::string AwsS3Client::get_signed_headers()
{
    //Signed Header stream
    std::ostringstream ss;

    for (std::map<std::string,std::string>::iterator it=m_headers.begin(); it!=m_headers.end(); ++it)
    {
        ss << boost::to_lower_copy(it->first);
        if ( it+1 != m_headers.end() )
            ss << ";";
    }
}

void AwsS3Client::set_chunked(bool flag)
{
    m_chunked=flag;
}
