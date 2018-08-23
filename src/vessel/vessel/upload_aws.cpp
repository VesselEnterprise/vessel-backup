#include <vessel/vessel/upload_manager.hpp>

AwsUpload::AwsUpload()
{

}

void AwsUpload::resume_uploads()
{

}

void AwsUpload::upload_file(const BackupFile& file)
{

    //Initialize the upload with the Vessel API
    init_upload(file);

}

void AwsUpload::complete_upload()
{

}

void AwsUpload::init_upload(const BackupFile& file)
{

    std::cout << "Uploading " << file.get_file_name() << "..." << '\n';
    get_vessel_client()->init_upload(file);

}
