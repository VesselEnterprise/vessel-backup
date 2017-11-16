#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include "compress.hpp"
#include "tarball.hpp"
#include "local_db.hpp"
#include "file_iterator.hpp"

using namespace std;
using namespace Backup::Compression;
using namespace Backup::Database;

int main()
{

    WSADATA wsadata;
    WSAStartup( MAKEWORD(2, 2), &wsadata );

    LocalDatabase* ldb = new LocalDatabase("local.db");

    //Update user home folder
    std::string home_drive = std::getenv("HOMEDRIVE");
    std::string home_path = std::getenv("HOMEPATH");

    ldb->update_setting("home_folder", home_drive.append(home_path) );

    //Update host name
    char hbuf[128];
    gethostname( hbuf,sizeof(hbuf) );

    std::string hostname(hbuf);

    std::cout << "Hostname: " << hostname << std::endl;

    ldb->update_setting("hostname", hostname );

    delete ldb;

    WSACleanup();

    return 0;

}
