#ifndef AZURE_HPP
#define AZURE_HPP

#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>

#include <boost/algorithm/string.hpp>
#include <boost/date_time/date_facet.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <cryptopp/cryptlib.h>
#include <cryptopp/hmac.h>
#include <cryptopp/sha.h>

#include <vessel/types.hpp>
#include <vessel/network/http_client.hpp>
#include <vessel/vessel/vessel_client.hpp>
#include <vessel/network/http_request.hpp>
#include <vessel/network/http_stream.hpp>
#include <vessel/filesystem/file.hpp>
#include <vessel/crypto/hash_util.hpp>
#include <vessel/azure/azure_exception.hpp>

using namespace Vessel::Types;
using namespace Vessel::Exception;
using namespace Vessel::Database;
using namespace Vessel::File;
using namespace Vessel::Networking;
using namespace Vessel::Utilities;

namespace Vessel {
    namespace Networking {

        class AzureClient : public HttpClient
        {

            public:

                AzureClient(const StorageProvider& provider);
                ~AzureClient();

                /*! \fn void init_upload(const BackupFile& file);
                    \brief Initializes the Azure upload
                */
                void init_upload(const BackupFile& file);

                /*! \fn bool init_block();
                    \brief Initializes a multi block upload
                */
                bool init_block();

                /*! \fn bool upload();
                    \brief Uploads a single file to the Azure Storage API
                    \return Returns true if the upload was successful, or false if there was an error
                */
                bool upload();

                /*! \fn bool upload_part(int part_number);
                    \brief Uploads a blob block
                    \return Returns true if the operation was success and false otherwise
                */
                bool upload_part(int part_number);

                /*! \fn bool complete_multipart_upload(int total_parts);
                    \brief Completes a multipart (block) upload
                    \return Returns true if the operation was successful
                */
                bool complete_multipart_upload(int total_parts);

                /*! \fn std::string get_block_list();
                    \brief Returns the block list for the current blob
                    \return Returns the block list for the current blob
                */
                std::string get_block_list();

                /*! \fn void remote_signing(bool flag);
                    \brief Enables or disables remote signing the request. Local key file is used for local
                */
                void remote_signing(bool flag);

                /*! \fn std::string last_request_id();
                    \brief Returns the last Azure request id
                    \return Returns the last Azure request id
                */
                std::string last_request_id();

                /*! \fn std::string get_upload_id();
                    \brief Returns the upload ID for Azure Blob Upload
                    \return Returns the upload ID for the Azure Blob Upload
                */
                std::string get_upload_id();

                /*! \fn std::string set_upload_id(const std::string& upload_id);
                    \brief Sets the upload/request id of the current Azure Blob upload
                */
                void set_upload_id(const std::string& upload_id);

                /*! \fn std::string get_padded_block_id(const std::string& id);
                    \brief Pads the block id (integer part number) by leading zeroes
                    \return RPads the block id (integer part number) by leading zeroes
                */
                std::string get_padded_block_id(const std::string& id);

            private:
                bool m_remote_signing;
                std::string m_xms_date;
                std::string m_xms_version;
                std::string m_xms_blob_type;
                std::string m_content_encoding;
                std::string m_http_verb;
                std::string m_content_language;
                std::string m_content_md5;
                std::string m_content_sha256;
                std::string m_content_type;
                std::string m_user_id;
                std::string m_file_uri_path;
                std::string m_block_id;
                std::string m_upload_id;
                std::map<std::string,std::string> m_headers;
                std::map<std::string,std::string> m_query_params;
                std::shared_ptr<std::string> m_content_body;
                int m_current_part;
                size_t m_content_length;
                size_t m_chunk_size;
                LocalDatabase* m_ldb;
                BackupFile m_file;
                StorageProvider m_storage_provider;

                /*! \fn std::string get_ms_date();
                    \brief Returns the current RFC 1123 formatted date/time
                    \return Returns the current RFC 1123 formatted date/time
                */
                std::string get_ms_date();

                /*! \fn std::string get_string_to_sign();
                    \brief Returns the string to sign used to sign the Azure API request
                    \return Returns the string to sign used to sign the Azure API request
                */
                std::string get_string_to_sign();

                /*! \fn std::string get_canonical_headers();
                    \brief Returns the headers used in the string to sign
                    \return Returns the headers used in the string to sign
                */
                std::string get_canonical_headers();

                /*! \fn std::string get_canonical_resources();
                    \brief Returns the canonical resources used in the string to sign
                    \return Returns the canonical resources used in the string to sign
                */
                std::string get_canonical_resources();

                /*! \fn void build_headers();
                    \brief Builds a map of request headers
                    \return Builds a map of request headers
                */
                void build_headers();

                /*! \fn std::string get_file_uri_path();
                    \brief Returns the relative URI path of the file
                    \return Returns the relative URI path of the file
                */
                std::string get_file_uri_path();

                /*! \fn std::string get_ms_signature();
                    \brief Returns the base 64 encoded signature for the request
                    \return Returns the base 64 encoded signature for the request
                */
                std::string get_ms_signature();

                /*! \fn void read_key_file()
                    \brief Reads the Azure key file and imports AccountName, ContainerName, and SharedKey
                */
                void read_key_file();

                /*! \fn std::string api_get_signature();
                    \brief Remotely signs the request using the Vessel API. Returns the base64 encoded signature
                    \return Remotely signs the request using the Vessel API. Returns the base64 encoded signature
                */
                std::string api_get_signature();

                /*! \fn void reset();
                    \brief Resets associated member vars for upload to defaults
                */
                void reset();

        };

    }
}

#endif

