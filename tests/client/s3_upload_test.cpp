#include <iostream>
#include <string>
#include <functional>

#include <vessel/aws/aws_s3_client.hpp>
#include <vessel/filesystem/file.hpp>

using namespace Vessel::Networking;
using namespace Vessel::File;
using namespace Vessel::Database;


int main( int argc, char** argv )
{

    using Vessel::Networking::AwsS3Client;

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
    aws->set_auth_profile(AwsS3Client::AuthProfile::Local);
    AwsS3Client::AwsFlags flags = AwsS3Client::AwsFlags::ReducedRedundancy;

    if ( upload_type == "multi" ) {
        flags = flags | AwsS3Client::AwsFlags::Multipart; // | AwsS3Client::AwsFlags::SkipMultiInit;
    }

    aws->init_upload(&bf, flags );

    std::ofstream outfile("output.txt", std::ofstream::out );
    if ( outfile.is_open() )
    {
        outfile << aws->get_response();
        outfile.close();
    }

    std::cin.get();

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

        //ETag storage
        std::vector<AwsS3Client::etag_pair> etags;

        for ( auto i=1; i <= total_parts; i++ )
        {
            std::cout << "Uploading file part " << i << " of " << total_parts << '\n';
            std::string etag = aws->upload_part(i, upload_id );
            std::cout << "HTTP Status: " << aws->get_http_status() << "\n";
            std::cout << "Response from server:\n" << aws->get_response() << "\n";
            std::cout << "ETag: " << etag << '\n';

            std::cin.get();

            //Store the etags
            AwsS3Client::etag_pair tag = {i, etag};
            etags.push_back( tag );
        }

        //Complete the Multipart upload
        std::string complete_etag = aws->complete_multipart_upload(etags, upload_id);

        std::cout << "Multipart upload was successful with ETag " << complete_etag << '\n';
    }

    return 0;

}
