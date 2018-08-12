#ifndef VESSELCLIENT_H
#define VESSELCLIENT_H

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <boost/array.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/algorithm/string.hpp>

//CryptoPP
#include <cryptopp/base64.h>

//RapidJSON
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/error/en.h>

#ifdef _WIN32
    #include <winsock2.h>
#endif // _WIN32

//Custom includes
#include <vessel/types.hpp>
#include <vessel/database/local_db.hpp>
#include <vessel/compression/compress.hpp>
#include <vessel/network/http_request.hpp>
#include <vessel/filesystem/file.hpp>
#include <vessel/filesystem/directory.hpp>
#include <vessel/log/log.hpp>
#include <vessel/network/http_client.hpp>
#include <vessel/vessel/vessel_exception.hpp>

//#define BOOST_NETWORK_ENABLE_HTTPS 1

using boost::asio::ip::tcp;
using boost::asio::deadline_timer;

using namespace Vessel;
using namespace Vessel::Types;
using namespace Vessel::File;
using namespace Vessel::Database;
using namespace Vessel::Logging;
using namespace rapidjson;

namespace ssl = boost::asio::ssl;

namespace Vessel {
    namespace Networking {

        class VesselClient : public HttpClient
        {

            enum { buffer_size = 1024 };

            public:

                VesselClient( const std::string& hostname );
                ~VesselClient();

                /*! \fn int init_upload( Vessel::File::BackupFile * bf );
                    \brief Initialize a new file upload with the server REST API
                    \return Returns 0 if successful, 1 if there were errors
                */
                int init_upload( Vessel::File::BackupFile * bf );

                /*! \fn upload_file_part( Vessel::File::BackupFile * bf, int part_number );
                    \brief Sends part of a file (or the entire file) to the server with metadata
                    \param bf BackupFile object
                    \param part_number The part or chunk number of the file content being uploaded
                    \return Returns true if successful, false if there were errors
                */
                bool upload_file_part( Vessel::File::BackupFile * bf, int part_number );

                /*! \fn bool heartbeat();
                    \brief Sends a check-in payload to the server. Server will return with a JSON payload to the client
                    \return Returns true if successful, false if errors
                */
                bool heartbeat();

                /*! \fn std::string get_client_settings();
                    \brief Returns client settings and overrides from the master server
                    \return Returns client settings and overrides from the master server
                */
                std::string get_client_settings();

                /*! \fn bool activate();
                    \brief Attempts to activate the user with the master server
                    \return Returns true if successful, false if not
                */
                bool activate();

                /*! \fn bool is_activated();
                    \brief Returns whether or not the user has been activated
                    \return Returns true if the user is activated, false if not
                */
                bool is_activated();

                /*! \fn void use_compression(bool flag);
                    \brief Set to true to use Zlib compression for the file content before sending. Reduces transfer sizes but increases CPU processing
                */
                void use_compression(bool flag);

                /*! \fn bool has_deployment_key();
                    \brief Returns true if the deployment_key setting has been set
                    \return Returns true if the deployment_key setting has been set
                */
                bool has_deployment_key();

                /*! \fn bool has_client_token();
                    \brief Returns true if the client_token setting has been set
                    \return Returns true if the client_token setting has been set
                */
                bool has_client_token();

                /*! \fn void install_client();
                    \brief Installs and initializes the client application with the Vessel API using the deployment key
                */
                void install_client();

                /*! \fn StorageProvider get_storage_provider();
                    \brief Returns the highest priority storage provider
                    \return Returns the highest priority storage provider
                */
                StorageProvider get_storage_provider();

            private:

                LocalDatabase* m_ldb;
                Vessel::Logging::Log* m_log;

                /*! \fn void send_request( Vessel::Networking::HttpRequest* r );
                    \brief Send the HTTP request to the server
                */
                void send_request( Vessel::Networking::HttpRequest* r );

                /** Authorization **/
                void handle_auth_error();

                /*! \fn bool refresh_token();
                    \brief Sends a token refresh request to the API
                    \return Returns true if the token was refreshed successfully, false if otherwise
                */
                bool refresh_token();

                std::string get_auth_header(const std::string& token, const std::string& user_id);

                std::string m_auth_header;
                std::string m_auth_token;
                std::string m_api_path;
                std::string m_user_id;
                bool m_activated;
                bool m_use_compression; //Send file content compressed

                /*! \fn void handle_api_error();
                    \brief Parses a JSON error response from the server and saves to log
                */
                void handle_api_error();

                /*! \fn void handle_json_error(const ParseResult& res);
                    \brief Handles a JSON parsing error and saves it to the log
                */
                void handle_json_error(const ParseResult& res);

                /*! \fn bool sync_storage_provider(const Value& obj);
                    \brief Syncs a single storage provider to the local database
                */
                bool sync_storage_provider(const Value& obj);

                /*! \fn bool sync_storage_provider_all(const std::vector<std::string>& provider_ids);
                    \brief Compares a vector of provider_ids to the local database and removes obsolete providers
                */
                void sync_storage_provider_all(const std::vector<std::string>& provider_ids);

                /*! \fn void delete_storage_provider(const std::string& id );
                    \brief Removes a storage provider from the local database
                */
                bool delete_storage_provider(const std::string& id );

        };

    }

}



#endif // VESSELCLIENT_H
