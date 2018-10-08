#include <iostream>
#include <string>

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE AwsClientTest

#include <boost/test/included/unit_test.hpp>

#include <vessel/aws/aws_s3_client.hpp>

BOOST_AUTO_TEST_SUITE(AwsTestSuite)

BOOST_AUTO_TEST_CASE(AwsObjectTest)
{

    StorageProvider provider;

    std::string file_path;

    std::cout << "Enter file path to upload: " << '\n';
    std::cin >> file_path;

    std::cout << '\n';

    BackupFile file(file_path);

    std::cout << "Enter AWS Server URL (https://<your-bucket>.s3-<your-region>.amazonaws.com): " << '\n';
    std::cin >> provider.server;

    std::cout << '\n';

    std::cout << "Enter AWS Region (eg. us-east-2): " << '\n';
    std::cin >> provider.region;

    std::cout << '\n';

    std::cout << "Enter AWS Access ID: " << '\n';
    std::cin >> provider.access_id;

    std::cout << '\n';

    std::cout << "Enter Bucket Name: " << '\n';
    std::cin >> provider.bucket_name;

    std::cout << '\n';

    std::cout << "Enter AWS Access Key: " << '\n';
    std::cin >> provider.access_key;

    std::cout << '\n';

    AwsS3Client client(provider);
    client.remote_signing(false);
    client.init_upload(file);

    bool success = client.upload();

    BOOST_TEST(success, "Failed to complete the upload");

    if ( success )
    {
        std::cout << "Successfully uploaded " << file.get_file_name() << '\n';
    }
    else
    {
        std::cout << "Upload failed with response: " << client.get_response() << '\n';
    }

}

BOOST_AUTO_TEST_CASE(AwsMultiPartTest)
{

    StorageProvider provider;

    std::string file_path;

    std::cout << "Enter file path to upload: " << '\n';
    std::cin >> file_path;

    std::cout << '\n';

    BackupFile file(file_path);

    std::cout << "Enter AWS Server URL (https://<your-bucket>.s3-<your-region>.amazonaws.com): " << '\n';
    std::cin >> provider.server;

    std::cout << '\n';

    std::cout << "Enter AWS Region (eg. us-east-2): " << '\n';
    std::cin >> provider.region;

    std::cout << '\n';

    std::cout << "Enter AWS Access ID: " << '\n';
    std::cin >> provider.access_id;

    std::cout << '\n';

    std::cout << "Enter Bucket Name: " << '\n';
    std::cin >> provider.bucket_name;

    std::cout << '\n';

    std::cout << "Enter AWS Access Key: " << '\n';
    std::cin >> provider.access_key;

    std::cout << '\n';

    AwsS3Client client(provider);
    client.remote_signing(false);
    client.init_upload(file, AwsS3Client::AwsFlags::Multipart | AwsS3Client::AwsFlags::ReducedRedundancy);

    int total_parts = file.get_total_parts();

    std::string upload_id = client.get_upload_id();

    BOOST_TEST(!upload_id.empty(), "Failed to initialize multipart upload: Invalid upload ID");

    //ETag storage
    std::vector<UploadTagSet> etags;

    //Upload Parts
    for ( int i=1; i <= total_parts; i++ )
    {
        std::string etag = client.upload_part(i, upload_id);
        BOOST_TEST(!etag.empty(), "Failed to upload file part #" + std::to_string(i) + ". HTTP Status: " + std::to_string(client.get_http_status()) );
        BOOST_TEST(client.get_http_status()==200, "Failed to upload file part with HTTP Status " + std::to_string(client.get_http_status()) );

        //Store the ETag
        UploadTagSet tag = {i, etag};
        etags.push_back( tag );
    }

    //Complete MultiPart Upload
    std::string complete_etag = client.complete_multipart_upload(etags, upload_id);

    BOOST_TEST(!complete_etag.empty(), "Failed to complete multipart upload. HTTP Status: " + std::to_string(client.get_http_status()) );
    BOOST_TEST(client.get_http_status()==200, "Failed to complete multipart upload. HTTP Status: " + std::to_string(client.get_http_status()) );

}

BOOST_AUTO_TEST_SUITE_END()
