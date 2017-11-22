#include "client.hpp"

using namespace Backup::Networking;

int main()
{

    Client* c = new Client("google.com", "443");
    c->set_timeout( boost::posix_time::seconds(3) );
    c->connect();

}
