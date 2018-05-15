#include <iostream>
#include <string>
#include <functional>

#include "aws_s3_client.hpp"
#include "file.hpp"

using namespace Backup::Networking;
using namespace Backup::File;
using namespace Backup::Database;


int main( int argc, char** argv )
{

    using Backup::Networking::AwsS3Client;

    std::string upload_type;
    std::string file_path;

    std::cout << "Upload a file in one part or multiparts (single/multi): " << '\n';
    std::cin >> upload_type;

    std::cin.ignore();

    std::cout << "Enter the full path to the file: " << '\n';
    std::getline( std::cin, file_path );

    BackupFile bf( boost::filesystem::path(file_path.c_str()) );

    std::cout << "Testing " << upload_type << " File Upload..." << '\n';

    AwsS3Client* aws = new AwsS3Client("https://vessel-backup.s3-us-east-2.amazonaws.com");
    AwsS3Client::AwsFlags flags = AwsS3Client::AwsFlags::ReducedRedundancy;

    if ( upload_type == "multi" ) {
        flags = flags | AwsS3Client::AwsFlags::Multipart | AwsS3Client::AwsFlags::SkipMultiInit;
    }

    aws->init_upload(&bf, flags );

    if ( upload_type != "multi" ) {
        aws->upload();
        std::cout << "HTTP Status: " << aws->get_http_status() << "\n";
        std::cout << "Response from server:\n" << aws->get_response() << "\n";
    }
    else {

        std::cout << "HTTP Status: " << aws->get_http_status() << "\n";
        std::cout << "Response from server:\n" << aws->get_response() << "\n";

        int total_parts = bf.get_total_parts();

        std::string upload_id = aws->get_upload_id();

        std::cout << "Uploading file part for upload id " << upload_id << '\n';

        for ( auto i=1; i <= total_parts; i++ )

        {
            std::cout << "Uploading file part " << i << " of " << total_parts << '\n';
            std::string etag = aws->upload_part(i, upload_id );
            std::cout << "HTTP Status: " << aws->get_http_status() << "\n";
            std::cout << "Response from server:\n" << aws->get_response() << "\n";
            std::cout << "ETag: " << etag << '\n';
            std::cin.get();
        }
    }

    return 0;

}
