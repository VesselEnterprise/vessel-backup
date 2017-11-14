#include <iostream>
#include "local_db.hpp"

using namespace Backup::Database;

int main(int c, char** argv)
{

    LocalDatabase *db = new LocalDatabase("local.db");

    std::cout << "Testing database..." << std::endl;
    std::cout << db->get_last_err() << std::endl;

    std::cout << "Get local setting" << std::endl;
    std::cout << db->get_setting_int("compression_level") << std::endl;

    delete db;

    return 0;
}
