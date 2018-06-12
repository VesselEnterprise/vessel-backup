#include <iostream>
#include "local_db.hpp"

using namespace Vessel::Database;

int main(int c, char** argv)
{

    LocalDatabase *db = new LocalDatabase("local.db");

    std::cout << "Testing database..." << std::endl;
    std::cout << db->get_last_err() << std::endl;

    std::cout << "Get local setting" << std::endl;
    std::cout << db->get_setting_int("compression_level") << std::endl;

    std::cout << "Update extension count" << std::endl;
    std::cout << db->update_ext_count(".jpg", 100 ) << std::endl;

    std::cout << "Update setting" << std::endl;
    std::cout << db->update_setting<std::string>("master_server", "some.server.com") << std::endl;

    std::cout << "Test Ignore Ext" << std::endl;
    std::cout << db->is_ignore_ext(".mp3") << std::endl;

    delete db;

    return 0;
}
