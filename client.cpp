#include "client.hpp"

using boost::asio::ip::tcp;
using boost::asio::deadline_timer;
using namespace Backup::Networking;

Client::Client( const std::string& hostname, const std::string& port ) : m_socket(m_io_service), m_deadline_timer(m_io_service), m_hostname(hostname), m_port(port)
{

    //Timer should not expire until a connection attempt is made
    m_deadline_timer.expires_at(boost::posix_time::pos_infin);
    this->check_deadline();

}

Client::~Client()
{

}

void Client::set_timeout( boost::posix_time::time_duration t )
{
    m_timeout = t;
}

bool Client::connect()
{

    //Set timeout
    m_deadline_timer.expires_from_now(m_timeout);

    try
    {

        tcp::resolver resolver(m_io_service);
        tcp::resolver::query query( m_hostname, m_port );
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

        boost::system::error_code ec = boost::asio::error::would_block;
        boost::asio::async_connect(m_socket, endpoint_iterator, boost::lambda::var(ec) = boost::lambda::_1 );

        //Run async connect until operation completed
        do m_io_service.run_one(); while (ec == boost::asio::error::would_block);

        if ( !ec && m_socket.is_open() )
            std::cout << "Connected!" << std::endl;
        else
            throw boost::system::system_error( ec ? ec : boost::asio::error::operation_aborted );

        for (;;)
        {
            boost::array<char, 128> buf;
            boost::system::error_code error;

            size_t len = m_socket.read_some(boost::asio::buffer(buf), error);

            if (error == boost::asio::error::eof)
                break; // Connection closed cleanly by peer.
            else if (error)
                throw boost::system::system_error(error); // Some other error.

            std::cout.write(buf.data(), len);

            std::cout << "Reading data..." << std::endl;
        }

    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return true;
}

void Client::check_deadline()
{
    // Check whether the deadline has passed. We compare the deadline against
    // the current time since a new asynchronous operation may have moved the
    // deadline before this actor had a chance to run.
    if (m_deadline_timer.expires_at() <= deadline_timer::traits_type::now())
    {
        // The deadline has passed. The socket is closed so that any outstanding
        // asynchronous operations are cancelled. This allows the blocked
        // connect(), read_line() or write_line() functions to return.
        boost::system::error_code ignored_ec;
        m_socket.close(ignored_ec);

        // There is no longer an active deadline. The expiry is set to positive
        // infinity so that the actor takes no action until a new deadline is set.
        m_deadline_timer.expires_at(boost::posix_time::pos_infin);
    }

    // Put the actor back to sleep.
    m_deadline_timer.async_wait(boost::bind(&Client::check_deadline, this));

}

void Client::disconnect()
{
    m_socket.close();
}

