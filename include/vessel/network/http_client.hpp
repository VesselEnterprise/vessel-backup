#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <iostream>
#include <cctype>
#include <iomanip>
#include <string>
#include <regex>
#include <memory>
#include <map>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/chrono.hpp>
#include <boost/bind.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/thread/thread.hpp>

#include <vessel/database/local_db.hpp>
#include <vessel/log/log.hpp>
#include <vessel/network/http_stream.hpp>
#include <vessel/network/http_exception.hpp>
#include <vessel/network/http_request.hpp>
#include <vessel/network/token_bucket.hpp>

#define MIN_TRANSFER_SPEED 500

using namespace Vessel;
using namespace Vessel::Database;
using namespace Vessel::Exception;
using namespace Vessel::Logging;

namespace Vessel {

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

                /*! \fn std::string get_header(const std::string& key);
                    \brief Returns the specified HTTP header value (if exists)
                    \return Returns the specified HTTP header value (if exists)
                */
                std::string get_header(const std::string& key);

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

                /*! \fn int send_http_request ( const HttpRequest& request );
                    \brief Sends a new HTTP request and writes to the socket
                    \return HTTP status code
                */
                int send_http_request ( const HttpRequest& request );

                /*! \fn bool http_logging();
                    \brief Returns true if http logging is enabled
                    \return True if http logging is enabled
                */
                bool http_logging();

                /*! \fn static void http_logging(bool flag);
                    \brief Enables or disables HTTP logging
                */
                static void http_logging(bool flag);

                /*! \fn size_t get_content_length();
                    \brief Returns the size of the content body
                    \return Returns the size of the content body
                */
                size_t get_content_length();

                /*! \fn int get_port();
                    \brief Returns the port used for HTTP connection
                    \return Returns the port used for HTTP connection
                */
                int get_port();

                /*! \fn std::string make_test_str(size_t length)
                    \brief Makes a test string of length bytes
                    \return Returns a test string of length bytes
                */
                std::string make_test_str(size_t length);

                /*! \fn void max_transfer_speed(size_t limit);
                    \brief Sets the max transfer speed for HTTP requests
                */
                void max_transfer_speed(size_t limit);

            private:

                LocalDatabase* m_ldb;
                Log* m_log;
                std::string m_hostname;
                std::string m_protocol;
                std::string m_header_data;
                std::string m_response_data;
                std::string m_error_message;
                unsigned int m_port; //or service name
                unsigned int m_http_status;
                size_t m_content_length;
                size_t m_max_transfer_speed;
                bool m_connected;
                bool m_use_ssl;
                bool m_verify_cert;
                bool m_ssl_good;
                bool m_chunked_encoding;
                bool m_stopped;
                static bool m_http_logging;
                long m_transfer_start_time;
                long m_last_write_time;
                size_t m_last_bytes_transferred;

                boost::asio::ssl::context m_ssl_ctx;
                boost::posix_time::time_duration m_timeout;

                boost::asio::io_service m_io_service;
                std::shared_ptr<boost::asio::io_service::work> m_work;
                std::shared_ptr<boost::asio::ip::tcp::socket> m_socket;
                std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> m_ssl_socket;
                std::shared_ptr<boost::asio::deadline_timer> m_deadline_timer;
                std::shared_ptr<boost::asio::streambuf> m_response_buffer;

                std::map<std::string,std::string> m_response_headers;

                boost::system::error_code m_conn_status;
                boost::system::error_code m_response_ec;

                std::shared_ptr<TokenBucket> m_token_bucket;
                std::shared_ptr<std::string> m_request_data;
                std::shared_ptr<boost::asio::const_buffer> m_request_buffer;


                void parse_url(const std::string& host );

                //Async function which persistently checks if the connection should timeout
                void check_deadline();

                void handle_connect( const boost::system::error_code& e );
                void handle_handshake(const boost::system::error_code& e );
                void handle_response( const boost::system::error_code& e );
                void handle_write( const boost::system::error_code& e, size_t bytes_transferred );
                void handle_write_throttled( const boost::system::error_code& e, size_t bytes_transferred );
                void handle_read_content( const boost::system::error_code& e, size_t bytes_transferred );
                void handle_read_headers( const boost::system::error_code& e );
                void read_chunked_content( const boost::system::error_code& e, size_t bytes_transferred );
                void read_buffer_data();
                void cancel_deadline();
                void init_deadline_timer();

                void cleanup();
                void set_defaults();


                void write_socket( const std::string& str );
                void run_io_service();

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

        };




    }

}

#endif
