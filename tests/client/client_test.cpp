#include "client.hpp"

#include <sstream>

using namespace Backup::Networking;

int main()
{

    std::string host = "http://10.1.10.208/phpmyadmin/index.php";

    Client* c = new Client(host);
    c->set_timeout( boost::posix_time::seconds(5) );
    if ( c->connect() )
        std::cout << "Connection was successful!" << '\n';
    else
        std::cout << "Error: Connection failed" << '\n';

//    std::ostringstream request_stream;
//    request_stream << "GET " << "/phpmyadmin" << " HTTP/1.1\r\n";
//    request_stream << "Host: " << "10.1.10.208" << "\r\n";
//    request_stream << "Accept: */*\r\n";
//    request_stream << "Connection: close\r\n\r\n";

    Backup::Types::http_request r;
    r.content_type = "text/plain";
    r.data = "Hello World!";
    r.request_type = Backup::Types::http::POST;
    r.uri = "/backup/test.php";

    c->http_request(r);

    c->disconnect();

    std::cout << "Disconnected!" << '\n';

    delete c;

    return 0;

}
