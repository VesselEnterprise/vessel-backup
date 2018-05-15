#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <iostream>
#include <cctype>
#include <iomanip>
#include <string>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

#include "local_db.hpp"
#include "log.hpp"

namespace Backup {

    namespace Networking {

        class HttpClient
        {

            public:

                HttpClient( const std::string& uri );
                ~HttpClient();

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

                /*! \fn std::string get_error();
                    \brief Returns the most recent error message recorded from the last operation
                    \return Returns the most recent error message recorded from the last operation
                */
                std::string get_error();

                /*! \fn std::string get_hostname();
                    \brief Returns the hostname of the URI
                    \return Returns the hostname of the URI
                */
                std::string get_hostname();

                /*! \fn std::string get_uri_path();
                    \brief Returns the URI path of the URL
                    \return Returns the URI path of the URL
                */
                std::string get_uri_path();

                /*! \fn bool is_https();
                    \brief Returns whether or not the connection uses HTTPS (secure)
                    \return Returns whether or not the connection uses HTTP (secure)
                */
                bool is_https();

                boost::system::error_code get_error_code();

                /*! \fn std::string encode_url(const std::string& url);
                    \brief Encodes URL according to RFC 3986. Spaces are encoded to "%20"
                    \return Returns the URL encoded string
                */
                std::string encode_uri(const std::string& uri);

                //TBD
                std::string decode_uri(const std::string& uri);

            private:

                Backup::Database::LocalDatabase* m_ldb;
                std::string m_hostname;
                std::string m_uri;
                std::string m_protocol;
                unsigned int m_port; //or service name
                bool m_connected;
                bool m_use_ssl;
                bool m_verify_cert;
                boost::posix_time::time_duration m_timeout;
                boost::asio::ssl::context m_ssl_ctx;
                boost::asio::io_service m_io_service;
                boost::shared_ptr<boost::asio::ip::tcp::socket> m_socket;
                boost::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> m_ssl_socket;
                boost::shared_ptr<boost::asio::deadline_timer> m_deadline_timer;

                std::ostream m_transfer_stream; //Data to send
                boost::asio::streambuf m_response_buffer;
                boost::asio::streambuf m_request_buffer;
                boost::asio::streambuf m_transfer_buffer; //Buffer containing data to send
                std::string m_request_data;

                unsigned int m_http_status;
                std::string m_header_data;
                std::string m_response_data;
                boost::system::error_code m_conn_status;
                bool m_ssl_good;
                boost::system::error_code m_response_ec;


                void parse_url(const std::string& host );

                //Async function which persistently checks if the connection should timeout
                void check_deadline();

                void handle_connect( const boost::system::error_code& e );
                void handle_handshake(const boost::system::error_code& e );
                void handle_response( const boost::system::error_code& e );
                void handle_write( const boost::system::error_code& e, size_t bytes_transferred );
                void handle_read_content( const boost::system::error_code& e );
                void handle_read_headers( const boost::system::error_code& e );

                /** Logging and errors **/
                Backup::Logging::Log* m_log;
                std::string m_error_message;

            protected:

                /*! \fn bool connect();
                    \brief Connect to the master server
                */
                bool connect();

                /*! \fn bool disconnect();
                    \brief Disconnect from the master server
                */
                void disconnect();

                void set_error(const std::string& msg);

                void clear_response();
                void clear_headers();

                void set_deadline(long seconds);

                void clear_error_code();

                void write_socket( const std::string& data );

                void run_io_service();

        };




    }

}

#endif
