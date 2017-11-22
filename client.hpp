#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <string>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lambda/lambda.hpp>

using boost::asio::ip::tcp;
using boost::asio::deadline_timer;

namespace Backup {
    namespace Networking {

        class Client
        {

            enum { buffer_size = 1024 };

            public:
                Client( const std::string& hostname, const std::string& port );
                ~Client();

                bool connect();
                void disconnect();
                bool is_connected();
                void set_timeout( boost::posix_time::time_duration t );

                bool send_file();
                bool resume_transfer();

            private:
                std::string m_hostname;
                std::string m_port; //or service name
                bool m_connected;
                boost::posix_time::time_duration m_timeout;
                boost::asio::io_service m_io_service;
                tcp::socket m_socket;
                deadline_timer m_deadline_timer;

                //Async function which persitently checks if the connection should timeout
                void check_deadline();


        };

    }

}



#endif // CLIENT_H
