#include <iostream>
#include <string>
#include <functional>

#include "aws_s3_client.hpp"
#include "file.hpp"

using namespace Backup::Networking;
using namespace Backup::File;

int main( int argc, char** argv )
{

    BackupFile bf( boost::filesystem::path("test_files/test.txt") );

    AwsS3Client* aws = new AwsS3Client("https://vessel-backup.s3-us-east-2.amazonaws.com");
    aws->set_file(&bf);

    std::cout << "Canonical Request:\n" << aws->get_canonical_request() << "\n\n";
    std::cout << "String to sign:\n" << aws->get_string_to_sign() << "\n";
    std::cout << "AWS V4 Signature:\n" << aws->get_signature_v4() << "\n";

    //aws->upload();

    std::cout << "HTTP Status: " << aws->get_http_status() << "\n";
    std::cout << "Response from server:\n" << aws->get_response() << "\n";

    std::cout << "File path: " << bf.get_canonical_path() << "\n";
    std::cout << "File Sha-1 hash: " << Backup::Utilities::Hash::get_sha1_hash( bf.get_canonical_path() ) << "\n";

    unsigned char* ptrhash = *bf.get_unique_id_raw();
    std::cout << "File Sha-1 hash raw:\r\n" << ptrhash << "\n";

    delete ptrhash;

    return 0;

}
