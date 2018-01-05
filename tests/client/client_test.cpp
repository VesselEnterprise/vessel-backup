#include "local_db.hpp"
#include "client.hpp"
#include "version.hpp"
#include "file.hpp"

#include <sstream>

using namespace Backup::Networking;
using namespace Backup::Database;

int main()
{

    #ifdef _WIN32
        WSADATA wsadata;
        WSAStartup( MAKEWORD(2, 2), &wsadata );
    #endif

    LocalDatabase *ldb = &LocalDatabase::get_database();
    ldb->update_global_settings();

    std::string host = ldb->get_setting_str("master_server");

    Client* c = new Client(host);
    c->set_timeout( boost::posix_time::seconds(5) );

    /**
    ** Test User Activation
    **/
    c->activate();

    if ( !c->is_activated() ) {
        std::cout << "Error: Client is not activated" << std::endl;
        return 1;
    }

    /**
    ** Test Heartbeat API
    **/
    c->heartbeat();

    std::cout << c->get_response();

    /**
    ** Test Get Client Settings
    **/
    std::string client_settings = c->get_client_settings();

    //Update Client Settings
    ldb->update_client_settings( client_settings );

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
