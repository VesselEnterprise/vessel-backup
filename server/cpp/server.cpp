#include "server.hpp"

using boost::asio::ip::tcp;
using namespace Backup::Networking;

Server::Server( boost::asio::io_service& io_service ) : m_acceptor( io_service, tcp::endpoint(tcp::v4(), 8080) )
{
    this->start_accept();
}

Server::~Server()
{

}

void Server::start_accept()
{
    Connection::pointer new_connection = Connection::create( m_acceptor.get_io_service() );

    m_acceptor.async_accept(
        new_connection->socket(),
        boost::bind( &Server::handle_accept, this, new_connection, boost::asio::placeholders::error )
    );
}

void Server::handle_accept( Connection::pointer new_connection, const boost::system::error_code& error)
{
    if (!error)
    {
        new_connection->start();
    }

    this->start_accept();
}
