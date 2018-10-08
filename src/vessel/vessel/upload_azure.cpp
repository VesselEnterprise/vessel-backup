#include <vessel/vessel/upload_manager.hpp>

AzureUpload::AzureUpload()
{
    m_client = std::make_shared<AzureClient>( get_vessel_client()->get_storage_provider() );
    m_client->remote_signing(true);
}

void AzureUpload::resume_uploads()
{

}

void AzureUpload::upload_file(FileUpload& upload)
{

    BackupFile file = upload.get_file();
    bool should_init=true;
    int total_parts = file.get_total_parts(); //Default

    //Determine if MultiPart
    if ( total_parts > 1 )
    {

        //Determine if there is an existing multipart upload (db upload id will be set)
        if ( !upload.get_upload_key().empty() ) {
            //If the file hash has not changed, we do not need to re-init the upload
            if ( file.get_hash_sha1() == upload.get_hash() )
            {
                should_init=false; //Do not re-init upload if it already exists
                m_client->set_upload_id( upload.get_upload_key() );
            }
        }
    }

    if ( should_init )
    {

        std::cout << "Upload is being initialized..." << '\n';

        //Initialize the upload with the Vessel API
        init_upload(file);

    }

    //Initialize the Azure upload
    m_client->init_upload(file);

    //Upload Single File
    if ( total_parts == 1 ) {
        if ( !m_client->upload() )
        {
            throw AzureException( AzureException::UploadFailed, "Azure single blob upload failed: " + m_client->last_request_id() );
        }
    }
    //Upload Multiple Blocks
    else {

        std::cout << "Uploading file part for upload id " << upload.get_upload_key() << '\n';

        //Flag indicating whether or not the multi block upload should be completed
        bool should_complete=true;

        //ETag storage
        std::vector<UploadTagSet> etags;

        //The current part number we are uploading
        int part_number = upload.get_current_part();

        //Determine if we are resuming an existing upload
        //If the part number is > 1,  preload the existing etags
        if ( part_number > 1 ) {
            etags = upload.get_part_tags();
        }
        else
        {
            //Multi block upload must be initialized
            if ( !m_client->init_block() )
            {
                throw AzureException( AzureException::UploadFailed, "Failed to initialize multiple block upload: " + m_client->last_request_id() );
            }

        }

        //std::cout << "GET Block List: " << '\n' << m_client->get_block_list();

        for ( int part_number = upload.get_current_part(); part_number <= total_parts; part_number++ )
        {

            std::cout << "Uploading file part " << part_number << " of " << total_parts << '\n';

            if ( !m_client->upload_part(part_number) ) {
                should_complete=false;
                Log::get_log().add_error("Failed to upload file block #: " + std::to_string(part_number), "Azure");
                break;
            }

            //Store the etags
            std::string etag = Hash::get_base64( m_client->get_padded_block_id(std::to_string(part_number)) ); //Base64 encoded part number
            UploadTagSet tag = {part_number, etag};
            etags.push_back( tag );

            //Add part to database
            FilePart part;
            part.upload_id = upload.get_upload_id();
            part.upload_key = upload.get_upload_key();
            part.part_number = part_number;
            part.total_bytes = m_client->get_content_length();
            part.tag = etag;

            //Save to database
            upload.add_part( part );

        }

        //Complete the Multipart upload
        if ( should_complete )
        {
            if ( !m_client->complete_multipart_upload(total_parts) )
            {
                throw AzureException( AzureException::UploadFailed, "Failed to complete the multi block upload (PUT block list): " + m_client->last_request_id() );
            }
            std::cout << "Multi block upload was successful with Request Id: " << m_client->last_request_id() << '\n';
        }

    }

}

void AzureUpload::complete_upload()
{

}

void AzureUpload::init_upload(const BackupFile& file)
{

    std::cout << "Uploading " << file.get_file_name() << "..." << '\n';
    get_vessel_client()->init_upload(file);

}

