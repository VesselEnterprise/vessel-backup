#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/algorithm/string.hpp>

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#ifdef _WIN32
    #include <winsock2.h>
#endif // _WIN32

#include "types.hpp"
#include "local_db.hpp"
#include "compress.hpp"
#include "http_request.hpp"
#include "file.hpp"
#include "log.hpp"

//#define BOOST_NETWORK_ENABLE_HTTPS 1

using boost::asio::ip::tcp;
using boost::asio::deadline_timer;
using Backup::File::BackupFile;
using Backup::Logging::Log;
using namespace rapidjson;
namespace ssl = boost::asio::ssl;

namespace Backup {
    namespace Networking {

        class BackupClient
        {

            enum { buffer_size = 1024 };

            public:

                BackupClient( const std::string& hostname );
                ~BackupClient();


                /*! \fn bool is_connected();
                    \brief Return whether or not the client is currently connected
                    \return Return whether or not the client is currently connected
                */
                bool is_connected();

                /*! \fn void set_timeout( boost::posix_time::time_duration t );
                    \brief Sets the connection timeout
                */
                void set_timeout( boost::posix_time::time_duration t );

                /*! \fn void set_ssl(bool f);
                    \brief Set whether or not to use SSL when connecting. Automatically determined from URL typically
                */
                void set_ssl(bool f);

                /*! \fn int init_upload( Backup::File::BackupFile * bf );
                    \brief Initialize a new file upload with the server REST API
                    \return Returns 0 if successful, 1 if there were errors
                */
                int init_upload( Backup::File::BackupFile * bf );

                /*! \fn upload_file_part( Backup::File::BackupFile * bf, int part_number );
                    \brief Sends part of a file (or the entire file) to the server with metadata
                    \param bf BackupFile object
                    \param part_number The part or chunk number of the file content being uploaded
                    \return Returns true if successful, false if there were errors
                */
                bool upload_file_part( Backup::File::BackupFile * bf, int part_number );

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

                /*! \fn std::string get_response();
                    \brief Returns the HTTP response payload
                    \return Returns the HTTP response payload
                */
                std::string get_response();

                /*! \fn std::string get_headers();
                    \brief Returns the HTTP headers returned in the response
                    \return Returns the HTTP headers returned in the response
                */
                std::string get_headers();

                /*! \fn unsigned int get_http_status();
                    \brief Returns the HTTP status code (eg. 200) of the response
                    \return Returns the HTTP status code (eg. 200) of the response
                */
                unsigned int get_http_status();

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

                /*! \fn std::string get_error();
                    \brief Returns the most recent error message recorded from the last operation
                    \return Returns the most recent error message recorded from the last operation
                */
                std::string get_error();

                /*! \fn void use_compression(bool flag);
                    \brief Set to true to use Zlib compression for the file content before sending. Reduces transfer sizes but increases CPU processing
                */
                void use_compression(bool flag);

            private:
                std::string m_hostname;
                std::string m_uri;
                std::string m_protocol;
                unsigned int m_port; //or service name
                bool m_connected;
                bool m_use_ssl;
                bool m_verify_cert;
                boost::posix_time::time_duration m_timeout;
                boost::asio::io_service m_io_service;
                ssl::context m_ssl_ctx;
                tcp::socket m_socket;
                boost::asio::ssl::stream<tcp::socket> m_ssl_socket;
                deadline_timer m_deadline_timer;

                boost::asio::streambuf m_response_buffer;
                boost::asio::streambuf m_request_buffer;
                std::string m_request_data;

                unsigned int m_http_status;
                std::string m_header_data;
                std::string m_response_data;
                boost::system::error_code m_conn_status;
                bool m_ssl_good;
                boost::system::error_code m_response_ec;

                /*! \fn bool connect();
                    \brief Connect to the master server
                */
                bool connect();

                /*! \fn bool disconnect();
                    \brief Disconnect from the master server
                */
                void disconnect();

                /*! \fn void send_request( Backup::Networking::HttpRequest* r );
                    \brief Send the HTTP request to the server
                */
                void send_request( Backup::Networking::HttpRequest* r );


                void parse_url(const std::string& host );

                //Async function which persistently checks if the connection should timeout
                void check_deadline();

                void handle_connect( const boost::system::error_code& e );
                void handle_handshake(const boost::system::error_code& e );
                void handle_response( const boost::system::error_code& e );
                void handle_write( const boost::system::error_code& e );
                void handle_read_content( const boost::system::error_code& e );
                void handle_read_headers( const boost::system::error_code& e );

                /** Authorization **/
                void handle_auth_error();
                bool refresh_token();

                std::string m_auth_token;
                bool m_activated;
                bool m_use_compression; //Send file content compressed

                /** Logging and errors **/
                Log* m_log;
                std::string m_error_message;
                void set_error(const std::string& msg);

        };

    }

}



#endif // CLIENT_H
