#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include "compress.hpp"
#include "tarball.hpp"
#include "local_db.hpp"
#include "file_iterator.hpp"

using namespace std;
using namespace Backup::Compression;
using namespace Backup::Database;

int main(int argc, char** argv)
{

    #ifdef _WIN32
        WSADATA wsadata;
        WSAStartup( MAKEWORD(2, 2), &wsadata );
    #endif

    LocalDatabase* ldb = new LocalDatabase("local.db");
    ldb->update_global_settings();

    delete ldb;

    #ifdef _WIN32
        WSACleanup();
    #endif

    return 0;

}
