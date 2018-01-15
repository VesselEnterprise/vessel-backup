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

    //std::string test_file = "C:\\Users\\kett.ky\\Downloads\\FileZilla_3.30.0_win64-setup.exe"; //Windows
    std::string test_file = "/home/kyle/Downloads/ServiceNowCAD.pdf";

    BackupFile* bf = new BackupFile(test_file);

    //Test file upload
    cli->upload_file_single(bf);

    delete bf;

    //Print response
    std::cout << cli->get_response() << std::endl;

    //Test Multi Part Uploads
    std::string large_file = "/home/kyle/Downloads/artful-desktop-amd64.iso";

    bf = new BackupFile( large_file );

    //Get Hash of a Very Large File
    //std::cout << "SHA-1 Hash: " << bf->get_hash() << std::endl;

    std::cout << "Total file size: " << bf->get_file_size() << std::endl;
    std::cout << "Total file parts: " << bf->get_total_parts() << std::endl;

    delete bf;

    delete cli;

}
