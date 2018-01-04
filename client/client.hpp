#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <string>
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

//#define BOOST_NETWORK_ENABLE_HTTPS 1

using boost::asio::ip::tcp;
using boost::asio::deadline_timer;
namespace ssl = boost::asio::ssl;

namespace Backup {
    namespace Networking {

        class Client
        {

            enum { buffer_size = 1024 };

            public:
                Client( const std::string& hostname );
                ~Client();

                bool connect();
                void disconnect();
                bool is_connected();
                void set_timeout( boost::posix_time::time_duration t );
                void set_ssl(bool f);

                void send_request( const Backup::Networking::HttpRequest& r );

                bool upload_file_single( const Backup::Types::http_upload_file& f);
                bool upload_file_part ( const Backup::Types::http_upload_file& f );
                bool heartbeat();

                bool resume_transfer();

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
                boost::asio::streambuf m_response;
                boost::asio::streambuf m_request;
                boost::system::error_code m_conn_status;
                bool m_ssl_good;
                boost::system::error_code m_response_ec;

                void parse_url(const std::string& host );

                //Async function which persistently checks if the connection should timeout
                void check_deadline();

                void handle_connect( const boost::system::error_code& e );
                void handle_handshake(const boost::system::error_code& e );
                void handle_response( const boost::system::error_code& e );
                void handle_write( const boost::system::error_code& e );
                void handle_read_content( const boost::system::error_code& e );
                void handle_read_headers( const boost::system::error_code& e );

                //POST a new file upload
                std::string make_upload_json( const Backup::Types::http_upload_file& f );

                std::string m_auth_token;

        };

    }

}



#endif // CLIENT_H
