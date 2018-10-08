#ifndef UPLOADMANAGER_H
#define UPLOADMANAGER_H

#include <iostream>
#include <string>
#include <memory>

#include <vessel/vessel/vessel_exception.hpp>
#include <vessel/filesystem/file.hpp>
#include <vessel/filesystem/file_upload.hpp>
#include <vessel/vessel/queue_manager.hpp>
#include <vessel/aws/aws_s3_client.hpp>
#include <vessel/azure/azure_client.hpp>
#include <vessel/vessel/vessel_client.hpp>

using namespace Vessel::Types;
using namespace Vessel::Database;
using namespace Vessel::File;
using namespace Vessel::Networking;

namespace Vessel
{
    class UploadInterface
    {
        public:
            UploadInterface();
            virtual void upload_file(FileUpload& upload) {}
            virtual void resume_uploads() {}
            virtual void complete_upload() {}

        protected:
            std::shared_ptr<VesselClient> get_vessel_client();

        private:
            std::shared_ptr<VesselClient> m_vessel;

    };

    class VesselUpload : public UploadInterface
    {

        public:

            VesselUpload();

            virtual void upload_file();
            virtual void resume_uploads();
            virtual void complete_upload();

        private:
            std::shared_ptr<LocalDatabase> m_database;

    };

    class AwsUpload : public UploadInterface
    {

        public:

            AwsUpload();

            void upload_file(FileUpload& upload);
            void resume_uploads();
            void complete_upload();

        private:
            std::shared_ptr<LocalDatabase> m_database;
            std::shared_ptr<AwsS3Client> m_client;

            void init_upload(const BackupFile& file);

    };

    class AzureUpload : public UploadInterface
    {

        public:

            AzureUpload();

            void upload_file(FileUpload& upload);
            void resume_uploads();
            void complete_upload();

        private:
            std::shared_ptr<LocalDatabase> m_database;
            std::shared_ptr<AzureClient> m_client;

            void init_upload(const BackupFile& file);

    };

    class UploadManager
    {

        public:
            UploadManager(const StorageProvider& provider);
            ~UploadManager();

            void run_uploader();

        protected:

            /*! \fn std::shared_ptr<UploadInterface> get_upload_service(const std::string& provider_type);
                \brief Returns upload service interface for a given provider type
                \return Returns upload service interface for a given provider type
            */
            std::shared_ptr<UploadInterface> get_upload_service(const std::string& provider_type);

        private:
            StorageProvider m_provider;
            std::shared_ptr<UploadInterface> m_service;

            void cleanup_service();

    };
}

#endif
