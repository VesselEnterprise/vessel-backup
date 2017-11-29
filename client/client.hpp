#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <string>
#include <vector>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/algorithm/string.hpp>

#include "types.hpp"

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

                bool send_file();

                 //Write data to socket
                void send_data( const std::string& data );

                void http_request( const Backup::Types::http_request& r );

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

                //Async function which persitently checks if the connection should timeout
                void check_deadline();

                void handle_connect( const boost::system::error_code& e );
                void handle_handshake(const boost::system::error_code& e );
                void handle_response( const boost::system::error_code& e );
                void handle_write( const boost::system::error_code& e );
                void handle_read_content( const boost::system::error_code& e );
                void handle_read_headers( const boost::system::error_code& e );

        };

    }

}



#endif // CLIENT_H
