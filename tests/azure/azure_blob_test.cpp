#include <iostream>
#include <string>
#include <vessel/azure/azure_client.hpp>

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE AzureBlobTest

#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(AzureTestSuite)

BOOST_AUTO_TEST_CASE(AzureBlobTest)
{
    StorageProvider provider;
    std::string file_path;

    std::cout << "Enter file path to upload (file must be greater than 50MB): " << '\n';
    std::cin >> file_path;

    std::cout << '\n';

    BackupFile file(file_path);
    int total_parts = file.get_total_parts();

    std::cout << "Enter Azure Server URL (https://<your-account-name>.blob.core.windows.net): " << '\n';
    std::cin >> provider.server;

    std::cout << '\n';

    std::cout << "Enter Azure Account Name: " << '\n';
    std::cin >> provider.access_id;

    std::cout << '\n';

    std::cout << "Enter Azure Container Name: " << '\n';
    std::cin >> provider.bucket_name;

    std::cout << '\n';

    std::cout << "Enter Azure Shared Key: " << '\n';
    std::cin >> provider.access_key;

    std::cout << '\n';

    AzureClient client(provider);
    client.remote_signing(false);
    client.init_upload(file);
    client.init_block();

    for ( int i=1; i <= total_parts; i++ )
    {
        bool block_uploaded = client.upload_part(i);
        BOOST_TEST(block_uploaded, "Failed to upload blob block");
        std::cout << "Uploaded Part# " << i << '\n';
    }

    bool complete = client.complete_multipart_upload(total_parts);

    BOOST_TEST(complete, "Failed to complete the block upload");

    std::cout << "Successfully uploaded the following blocks:" << '\n' << client.get_block_list();

}

BOOST_AUTO_TEST_CASE(AzureBlockTest)
{

    StorageProvider provider;
    std::string file_path;

    std::cout << "Enter file path to upload (file must be greater than 50MB): " << '\n';
    std::cin >> file_path;

    std::cout << '\n';

    BackupFile file(file_path);
    int total_parts = file.get_total_parts();

    std::cout << "Enter Azure Server URL (https://<your-account-name>.blob.core.windows.net): " << '\n';
    std::cin >> provider.server;

    std::cout << '\n';

    std::cout << "Enter Azure Account Name: " << '\n';
    std::cin >> provider.access_id;

    std::cout << '\n';

    std::cout << "Enter Azure Container Name: " << '\n';
    std::cin >> provider.bucket_name;

    std::cout << '\n';

    std::cout << "Enter Azure Shared Key: " << '\n';
    std::cin >> provider.access_key;

    std::cout << '\n';

    AzureClient client(provider);
    client.remote_signing(false);
    client.init_upload(file);
    client.init_block();

    for ( int i=1; i <= total_parts; i++ )
    {
        bool block_uploaded = client.upload_part(i);
        BOOST_TEST(block_uploaded, "Failed to upload blob block");
        std::cout << "Uploaded Part# " << i << '\n';
    }

    bool complete = client.complete_multipart_upload(total_parts);

    BOOST_TEST(complete, "Failed to complete the block upload");

    std::cout << "Successfully uploaded the following blocks:" << '\n' << client.get_block_list();

}

BOOST_AUTO_TEST_SUITE_END()
