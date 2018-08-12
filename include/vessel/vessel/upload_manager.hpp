#ifndef UPLOADMANAGER_H
#define UPLOADMANAGER_H

#include <iostream>
#include <string>
#include <memory>

#include <vessel/vessel/vessel_exception.hpp>
#include <vessel/aws/aws_s3_client.hpp>
#include <vessel/vessel/vessel_client.hpp>

using namespace Vessel::Types;
using namespace Vessel::Database;
using namespace Vessel::Networking;

namespace Vessel
{
    class UploadInterface
    {
        public:
            virtual void upload_file() = 0;
            virtual void resume_uploads() = 0;
            virtual void complete_upload() = 0;

        private:

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
            std::shared_ptr<VesselClient> m_client;

    };

    class AwsUpload : public UploadInterface
    {

        public:

            AwsUpload();

            virtual void upload_file();
            virtual void resume_uploads();
            virtual void complete_upload();

        private:
            std::shared_ptr<LocalDatabase> m_database;
            std::shared_ptr<AwsS3Client> m_client;

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
            std::shared_ptr<UploadInterface> m_service;

    };
}

#endif
