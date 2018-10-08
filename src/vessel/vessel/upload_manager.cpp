#include <vessel/vessel/upload_manager.hpp>

UploadManager::UploadManager(const StorageProvider& provider) : m_provider(provider)
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
        Log::get_log().add_message("No pending uploads were found in queue", "File Upload" );
        return; //No work to do
    }

    for ( auto i=0; i < total_pending; i++ )
    {

        //Free memory
        cleanup_service();

        //Get the next upload from the queue
        FileUpload upload = manager->get_next_upload();
        BackupFile file = upload.get_file();

        //Validate error count
        //If more than 5 errors, purge the file and move on
        if ( upload.get_error_count() >= 5 )
        {
            Log::get_log().add_error("More than 5 errors detected for file upload - skipping: " + file.get_file_name(), "File Upload" );
            LocalDatabase::get_database().purge_file( file.get_file_id().get() );
            continue;
        }

        //If the file no longer exists, purge it from the database
        if ( !file.exists() ) {
            LocalDatabase::get_database().purge_file( file.get_file_id().get() );
            continue;
        }

        //If the file is not readable, remove from the upload queue and move on
        if ( !file.is_readable() )
        {
            Log::get_log().add_error("Unable to read file: " + file.get_file_name(), "Filesystem" );
            manager->pop_file( file.get_file_id() );
            continue;
        }

        std::cout << "Uploading file " << file.get_file_name() << '\n';

        bool upload_success=true;

        try
        {
            m_service->upload_file(upload);
        }
        catch( const std::exception& ex )
        {
            Log::get_log().add_error("Failed to upload file: " + file.get_file_name() + " (" + ex.what() + ")", "File upload");
            upload.increment_error(); //Increase upload error count in DB
            upload_success=false;
        }

        if ( upload_success ) {

            std::cout << "File Upload was successful: " << file.get_file_name() << '\n';

            //Update the last backup time for the file
            file.update_last_backup();

            //Remove the file from the queue, regardless of success or failure
            manager->pop_file( file.get_file_id() );

        }

    }

}

void UploadManager::cleanup_service()
{
    m_service.reset();
    m_service = get_upload_service(m_provider.provider_type);
}

std::shared_ptr<UploadInterface> UploadManager::get_upload_service(const std::string& type)
{

    if ( type == "aws_s3" ) return std::make_shared<AwsUpload>();
    if ( type == "vessel" ) return std::make_shared<VesselUpload>();
    if ( type == "azure_blob" ) return std::make_shared<AzureUpload>();
    //if ( type == "google" )
    //if ( type == "local" )

    throw VesselException(VesselException::ProviderError,"Bad storage provider type");

}
