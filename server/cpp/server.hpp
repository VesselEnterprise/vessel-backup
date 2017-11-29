#ifndef SERVER_H
#define SERVER_H

#include <ctime>
#include <iostream>
#include <string>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

#include "connection.hpp"

using boost::asio::ip::tcp;

namespace Backup {
    namespace Networking {

        class Server
        {

            enum { buffer_size = 1024 };

            public:
                Server( boost::asio::io_service& io_service );
                ~Server();

            private:
                tcp::acceptor m_acceptor;

                void start_accept();
                void handle_accept( Connection::pointer new_connection, const boost::system::error_code& e );

        };

    }
}

#endif // SERVER_H
