#include <vessel/vessel/upload_manager.hpp>

UploadManager::UploadManager(const StorageProvider& provider)
{

    m_service = get_upload_service(provider.provider_type);

}

UploadManager::~UploadManager()
{

}

void UploadManager::run_uploader()
{

}

std::shared_ptr<UploadInterface> UploadManager::get_upload_service(const std::string& type)
{

    std::shared_ptr<UploadInterface> service;

    if ( type == "aws_s3" )
    {
        service = std::make_shared<AwsUpload>();
    }
    else if ( type == "azure" )
    {
        //Add support for Azure Here
    }
    else if ( type == "google" )
    {
        //Add support for Google Here
    }
    else if ( type == "vessel" )
    {
        service = std::make_shared<VesselUpload>();
    }
    else if ( type == "local" )
    {
        //Add support for Local Uploads here
    }
    else
    {
        throw VesselException(VesselException::ProviderError,"Bad storage provider type");
    }

    return service;

}
