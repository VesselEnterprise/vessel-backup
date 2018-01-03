#include "local_db.hpp"
#include "client.hpp"
#include "version.hpp"

#include <sstream>

using namespace Backup::Networking;
using namespace Backup::Database;

int main()
{

    #ifdef _WIN32
        WSADATA wsadata;
        WSAStartup( MAKEWORD(2, 2), &wsadata );
    #endif

    LocalDatabase *ldb = &LocalDatabase::getDatabase();
    ldb->update_global_settings();

    std::string host = "http://10.1.226.69";

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

    /**
    ** Test Heartbeat API
    **/
    c->heartbeat();

    //c->disconnect();

    /*

    //Sample file upload
    Backup::Types::file_data fd;
    fd.compressed_size = 100;
    fd.filename = "some_random_file.txt";
    fd.filesize = 100000;
    fd.file_ext = ".jpg";
    fd.last_modified = 1513045712;
    fd.parent_path = "/some/random/path";

    Backup::Types::http_upload_file f;
    f.compressed = false;
    f.fd = fd;
    f.multi_part = false;
    f.part = 0;
    f.upload_id = 1000;

    //c->upload_file_single(f);

    //c->disconnect();

    std::cout << "Disconnected!" << '\n';

    */

    delete c;

    #ifdef _WIN32
        WSACleanup();
    #endif

    return 0;

}
