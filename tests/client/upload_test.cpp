#include "local_db.hpp"
#include "file.hpp"
#include "vessel_client.hpp"

#include <sstream>
#include <boost/timer/timer.hpp>

using namespace Vessel::Networking;
using namespace Vessel::Database;
using namespace Vessel::File;
using Vessel::Compression::Compressor;

int main(int argc, char** argv)
{

    LocalDatabase* ldb = &LocalDatabase::get_database();
    ldb->update_global_settings();

    std::string host = ldb->get_setting_str("master_server");

    VesselClient* cli = new VesselClient(host);
    cli->use_compression(false);

    std::cout << "URI path: " << cli->get_uri_path() << "\n";
    std::cout << "Encoded URL: " << cli->encode_uri(cli->get_uri_path()) << "\n";

    //return 0;

    /**
    ** Test User Activation
    **/
    cli->activate();

    if ( !cli->is_activated() ) {
        std::cout << "Error: BackupClient is not activated" << std::endl;
        return 1;
    }

    //std::string test_file = "C:\\Users\\kett.ky\\Downloads\\FileZilla_3.30.0_win64-setup.exe"; //Windows
    //std::string test_file = "/home/kyle/Downloads/ServiceNowCAD.pdf";

    std::string test_file = "";

    BackupFile* bf = new BackupFile(test_file);

    std::cout << "File Unique ID: " << bf->get_unique_id() << "\n";

    //Test Multi Part Uploads
    //std::string large_file = "/home/kyle/Downloads/artful-desktop-amd64.iso"; //Linux
    //std::string large_file = "D:/iso/ubuntu-17.10-server-amd64.iso";

    std::cout << "Multipart size is: " << ldb->get_setting_int("multipart_filesize") << std::endl;

    size_t chunk_size = ldb->get_setting_int("multipart_filesize");

    bf->set_chunk_size( chunk_size );

    //Get Hash of a Very Large File
    //std::cout << "SHA-1 Hash: " << bf->get_hash() << std::endl;

    std::cout << "Total file size: " << bf->get_file_size() << std::endl;
    std::cout << "Total file parts: " << bf->get_total_parts() << std::endl;

    //Initialize upload
    int upload_id = 0; //TODO

    if ( upload_id < 0 )
    {
        std::cout << "There was an error intializing the file upload" << std::endl;
        return 1;
    }

    std::cout << "Initialized upload id: " << upload_id << std::endl;

    std::cin.get();

    bf->set_upload_id(""); //TODO

    //Create a compressed tmp file
    std::string tmp_file = "tmp/" + bf->get_file_name() + ".tmp";
    Compressor* c = new Compressor();
    c->compress_file( bf->get_file_path(), tmp_file );

    //Create a new backup file for the compressed tmp file
    BackupFile* bfc = new BackupFile(tmp_file);
    bfc->set_chunk_size( chunk_size );

    int total_parts = bfc->get_total_parts();

    //Upload parts of the upload
    for ( int i=1; i <= total_parts; i++ )
    {
        std::cout << "Uploading part " << i << " of " << total_parts << std::endl;
        cli->upload_file_part(bf, i);
    }

    delete bfc;
    delete bf;
    delete cli;

}
