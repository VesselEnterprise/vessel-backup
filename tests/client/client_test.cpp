#include "local_db.hpp"
#include "client.hpp"
#include "version.hpp"
#include "file.hpp"
#include "file_iterator.hpp"

#include <sstream>
#include <boost/timer/timer.hpp>

using namespace Vessel::Networking;
using namespace Vessel::Database;
using namespace Vessel::File;

int main()
{

    #ifdef _WIN32
        WSADATA wsadata;
        WSAStartup( MAKEWORD(2, 2), &wsadata );
    #endif

    LocalDatabase *ldb = &LocalDatabase::get_database();
    ldb->update_global_settings();

    std::string host = ldb->get_setting_str("master_server");

    BackupClient* c = new BackupClient(host);
    c->set_timeout( boost::posix_time::seconds(5) );

    /**
    ** Test User Activation
    **/
    c->activate();

    if ( !c->is_activated() ) {
        std::cout << "Error: BackupClient is not activated" << std::endl;
        return 1;
    }

    /**
    ** Test Heartbeat API
    **/
    c->heartbeat();

    std::cout << c->get_response();

    /**
    ** Test Get BackupClient Settings
    **/
    std::string client_settings = c->get_client_settings();

    //Update BackupClient Settings
    ldb->update_client_settings( client_settings );

    /**
    ** Test File Techniques
    **/

    std::string test_file = "";

    BackupFile* bf = new BackupFile(test_file);

    std::cout << "File Name: " << bf->get_file_name() << std::endl;
    std::cout << "File Size: " << bf->get_file_size() << std::endl;
    std::cout << "File Type: " << bf->get_file_type() << std::endl;
    std::cout << "File Path: " << bf->get_file_path() << std::endl;
    std::cout << "File Parent Path: " << bf->get_parent_path() << std::endl;
    std::cout << "File Relative Path: " << bf->get_parent_path() << std::endl;
    std::cout << "File Exists?: " << bf->exists() << std::endl;

    //Measure SHA1 Hash total time
    boost::timer::cpu_timer t;

    std::cout << "File SHA1 Hash: " << bf->get_hash(false) << std::endl;

    t.stop();

    const boost::timer::cpu_times hash_elapsed ( t.elapsed() );
    const boost::timer::nanosecond_type elapsed_ns (hash_elapsed.system + hash_elapsed.user);

    double total = (elapsed_ns / 1000000000.00);

    std::cout << "Total Hashing Time Elapsed: " << total << "s" << std::endl;

    //Test File Scanner
    Vessel::File::FileIterator* fi = new Vessel::File::FileIterator( ldb->get_setting_str("home_folder") );

    //Clean files
    std::cout << "Cleaning database..." << std::endl;
    ldb->clean();

    std::cin.get();

    t.start();
    fi->scan();
    t.stop();

    const boost::timer::cpu_times scan_elapsed ( t.elapsed() );
    const boost::timer::nanosecond_type scan_elapsed_ns (scan_elapsed.system + scan_elapsed.user);

    double total_scan_time = (scan_elapsed_ns / 1000000000.00);

    std::cout << "Total Scan Time Elapsed: " << total_scan_time  << "s" << std::endl;

    delete fi;
    delete bf;
    delete c;

    #ifdef _WIN32
        WSACleanup();
    #endif

    return 0;

}
