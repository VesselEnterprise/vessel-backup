#ifndef CONNECTION_H
#define CONNECTION_H

#include <ctime>
#include <iostream>
#include <string>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

namespace Backup {
    namespace Networking {

        class Connection : public boost::enable_shared_from_this<Connection>
        {
            public:

                typedef boost::shared_ptr<Connection> pointer;

                static pointer create(boost::asio::io_service& io_service);

                tcp::socket& socket();

                void start();

            private:
                tcp::socket m_socket;
                std::string m_message;

                Connection(boost::asio::io_service& io_service) : m_socket(io_service){}

                void handle_write( const boost::system::error_code& e, size_t bytes_t );

                std::string make_daytime_string();
        };

    }

}

#endif // CONNECTION_H
