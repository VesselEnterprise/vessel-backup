#include "connection.hpp"

using namespace Backup::Networking;

Connection::pointer Connection::create( boost::asio::io_service& io_service )
{
    return pointer(new Connection(io_service));
}

tcp::socket& Connection::socket()
{
    return m_socket;
}

void Connection::start()
{

    m_message = this->make_daytime_string();

    boost::asio::async_write(
        m_socket,
        boost::asio::buffer(m_message),
        boost::bind(&Connection::handle_write, shared_from_this(),boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)
    );
}

void Connection::handle_write( const boost::system::error_code& e, size_t bytes_t )
{

    std::cout << "Wrote some data" << std::endl;

}

std::string Connection::make_daytime_string()
{
    using namespace std;
    time_t now = time(0);
    return ctime(&now);
}
