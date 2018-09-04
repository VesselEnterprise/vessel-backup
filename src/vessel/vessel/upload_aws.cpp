#include <vessel/vessel/upload_manager.hpp>

AwsUpload::AwsUpload()
{
    m_client = std::make_shared<AwsS3Client>( get_vessel_client()->get_storage_provider() );
    m_client->remote_signing(true);
}

void AwsUpload::resume_uploads()
{

}

void AwsUpload::upload_file(FileUpload& upload)
{

    BackupFile file = upload.get_file();
    bool should_init=true;
    int total_parts = file.get_total_parts(); //Default
    AwsS3Client::AwsFlags aws_flags = AwsS3Client::AwsFlags::ReducedRedundancy; //Default

    //Determine if MultiPart
    if ( total_parts > 1 )
    {
        aws_flags = aws_flags | AwsS3Client::AwsFlags::Multipart;

        //Determine if there is an existing multipart upload (db upload id will be set)
        if ( !upload.get_upload_key().empty() ) {
            //If the file hash has not changed, we do not need to re-init the upload
            if ( file.get_hash_sha1() == upload.get_hash() )
            {
                should_init=false; //Do not re-init upload if it already exists
                aws_flags = aws_flags | AwsS3Client::AwsFlags::SkipMultiInit; //Do not generate a new AWS Upload ID
                m_client->set_upload_id( upload.get_upload_key() );
                std::cout << "Upload will NOT be initialized" << '\n';
            }
        }
    }

    if ( should_init )
    {

        std::cout << "Upload is being initialized..." << '\n';

        //Initialize the upload with the Vessel API
        init_upload(file);

    }

    //Initialize the AWS upload
    if ( m_client->init_upload(file, aws_flags ) )
    {
        //Update the upload key for the FileUpload if does not already exist
        if ( upload.get_upload_key().empty() ) {
            upload.update_key( m_client->get_upload_id() ); //AWS UploadId
        }
    }
    else
    {
        throw AwsException( AwsException::InitFailed, "Failed to initialize AWS Upload");
    }

    //Upload Single File
    if ( total_parts == 1 ) {
        m_client->upload();
    }
    //Upload Multipart
    else {

        //Confirm upload id
        if ( upload.get_upload_key().empty() ) {
            throw AwsException( AwsException::BadUploadId, "Invalid upload id was provided: " + upload.get_upload_key() );
        }

        std::cout << "Uploading file part for upload id " << upload.get_upload_key() << '\n';

        //Flag indicating whether or not the multipart upload should be completed
        bool should_complete=true;

        //ETag storage
        std::vector<UploadTagSet> etags;

        //The current part number we are uploading
        int part_number = upload.get_current_part();

        //Determine if we are resuming an existing upload
        //If the part number is > 1,  preload the existing etags
        if ( part_number > 1 ) {
            etags = upload.get_part_tags();
            std::cout << "Returned existing etags!" << '\n';
        }

        for ( int part_number = upload.get_current_part(); part_number <= total_parts; part_number++ )
        {

            std::cout << "Uploading file part " << part_number << " of " << total_parts << '\n';
            std::string etag = m_client->upload_part(part_number, upload.get_upload_key() );

            if ( etag.empty() ) {
                should_complete=false;
                Log::get_log().add_error("Failed to upload file part: " + std::to_string(part_number), "AWS");
                break;
            }

            //Store the etags
            UploadTagSet tag = {part_number, etag};
            etags.push_back( tag );

            //Add part to database
            FilePart part;
            part.upload_id = upload.get_upload_id();
            part.upload_key = upload.get_upload_key();
            part.part_number = part_number;
            part.total_bytes = m_client->get_current_part_size();
            part.tag = etag;

            //Save to database
            upload.add_part( part );

        }

        //Complete the Multipart upload
        if ( should_complete )
        {
            std::string complete_etag = m_client->complete_multipart_upload(etags, upload.get_upload_key() );
            std::cout << "Multipart upload was successful with ETag " << complete_etag << '\n';
        }

    }

}

void AwsUpload::complete_upload()
{

}

void AwsUpload::init_upload(const BackupFile& file)
{

    std::cout << "Uploading " << file.get_file_name() << "..." << '\n';
    get_vessel_client()->init_upload(file);

}
