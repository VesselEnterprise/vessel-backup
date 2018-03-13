#include "s3client.hpp"

using namespace Backup::Networking;

S3Client::S3Client(const std::string& uri) : HttpClient(uri)
{

    m_http_verb = "PUT"; //Default for uploading new files

}

std::string S3Client::get_canonical_request()
{
    std::stringstream ss;

    ss << m_http_verb << "\n";
    ss << encode_uri(get_uri_path()) << "\n";
    ss << "\n";
    ss <<
}

void S3Client::make_headers()
{
    std::vector
}
