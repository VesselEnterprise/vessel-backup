#include "local_db.hpp"
#include "file.hpp"
#include "client.hpp"

#include <sstream>
#include <boost/timer/timer.hpp>

using namespace Backup::Networking;
using namespace Backup::Database;
using namespace Backup::File;

int main(int argc, char** argv)
{

    LocalDatabase* ldb = &LocalDatabase::get_database();
    ldb->update_global_settings();

    std::string host = ldb->get_setting_str("master_server");

    BackupClient* cli = new BackupClient(host);

    std::string test_file = "C:\\Users\\kett.ky\\Downloads\\FileZilla_3.30.0_win64-setup.exe";

    BackupFile bf(test_file);

    //Test file upload
    cli->upload_file_single(bf);

    //Print response
    std::cout << cli->get_response() << std::endl;

    delete cli;

}
