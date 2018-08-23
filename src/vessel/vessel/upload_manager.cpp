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

    //Scan the queue and start uploading files using the provider service
    std::unique_ptr<QueueManager> manager = std::make_unique<QueueManager>();

    int total_pending = manager->get_total_pending();

    if ( total_pending <= 0 )
    {
        return; //No work to do
    }

    for ( auto i=0; i < total_pending; i++ )
    {

        BackupFile file = manager->get_next_file();

        //If the file is not readable, remove from the upload queue and move on
        if ( !file.is_readable() )
        {
            manager->pop_file(*file.get_file_id());
            continue;
        }

        m_service->upload_file(file);
    }

}

std::shared_ptr<UploadInterface> UploadManager::get_upload_service(const std::string& type)
{

    if ( type == "aws_s3" ) return std::make_shared<AwsUpload>();
    if ( type == "vessel" ) return std::make_shared<VesselUpload>();
    //if ( type == "azure" )
    //if ( type == "google" )
    //if ( type == "local" )

    throw VesselException(VesselException::ProviderError,"Bad storage provider type");

}
