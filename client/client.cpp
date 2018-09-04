/**
 ** Vessel Backup Client
 ** Copyright Kyle Ketterer 2017-2018
 **
**/

#include <iostream>
#include <client.hpp>

int main(int argc, char** argv)
{

    #ifdef _WIN32
        WSADATA wsadata;
        WSAStartup( MAKEWORD(2, 2), &wsadata );
    #endif

    //Declare supported program args
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        ("help", "Display supported options")
        ("auth-token", boost::program_options::value<std::string>(), "Sets the user auth token")
        ("with-client-token", boost::program_options::value<std::string>(), "Sets the client token")
        ("with-file-logging", boost::program_options::value<std::string>(), "Enables saving log output to flat files")
        ("with-sql-logging", boost::program_options::value<std::string>(), "Enables saving log output to SQLite database")
        ("with-http-logging", boost::program_options::value<std::string>(), "Enables the logging of HTTP requests and responses")
        ("with-deployment-key", boost::program_options::value<std::string>(), "Sets the deployment key")
        ("with-scan-dir", boost::program_options::value<std::string>(), "Sets the initial scanning directory")
    ;

    boost::program_options::variables_map vm;
    boost::program_options::store( boost::program_options::parse_command_line(argc, argv, desc), vm);
    boost::program_options::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    //Enable HTTP Logging
    if ( vm.count("with-http-logging") ) {
        HttpClient::http_logging(true);
    }

    //Override for development
    HttpClient::http_logging(true);

    //Create new instance of local database
    LocalDatabase* db = &LocalDatabase::get_database();

    //Get Log Instance
    Log* log = &Log::get_log();

    //Update global settings
    db->update_global_settings();

    std::string vessel_host = db->get_setting_str("master_server");

    //Create new instance of Vessel client
    std::shared_ptr<VesselClient> vessel = std::make_shared<VesselClient>(vessel_host);

    //Verify/install client
    if ( !vessel->has_client_token() )
    {
        vessel->install_client();
        log->add_message("Successfully installed client", "Installation");
    }

    //Create worker threads and pool them
    boost::asio::io_service io_service;
    boost::thread_group tpool;

    boost::asio::io_service::work work(io_service);

    //Determine number of threads we can run
    size_t thread_count = boost::thread::hardware_concurrency();

    std::cout << "Starting " << thread_count << " threads!" << '\n';

    //Start hardware_concurrency() threads for processing
    for ( unsigned int i=0; i < thread_count; i++ )
    {
        tpool.create_thread( [&]() { io_service.run(); });
    }

    //Create an empty instance of BackupDirectory for initial scan
    //TODO: This can be overridden by boost::program_options
    BackupDirectory scan_dir ("");

    //Add a new thread to the pool which scans the filesystem on intervals
    io_service.post([&](){
        for(;;)
        {
            //Create a file iterator object to scan the filesystem
            std::unique_ptr<FileIterator> file_iterator = std::make_unique<FileIterator>(scan_dir);

            try
            {
                file_iterator->scan();
            }
            catch( const std::exception& ex )
            {
                log->add_exception(ex);
            }

            boost::this_thread::sleep( boost::posix_time::seconds(900) );
            std::cout << "Restarting scan after 15 minutes..." << '\n';

            file_iterator.reset();
        }
    });

    //Manage the upload queue
    io_service.post([&](){
        for(;;)
        {

            //Create new upload QueueManager
            std::unique_ptr<QueueManager> queue_manager = std::make_unique<QueueManager>();

            try
            {
                //Rebuild the queue if no uploads are remaining
                if ( queue_manager->get_total_pending() == 0 ) {
                    queue_manager->rebuild_queue();
                }
            }
            catch( const std::exception& ex )
            {
                log->add_exception(ex);
            }

            boost::this_thread::sleep( boost::posix_time::seconds(10) );
            std::cout << "Restarting queue rebuild after 10 seconds..." << '\n';

            queue_manager.reset();
        }
    });

    //Manage uploads
    io_service.post([&](){

        //Get Storage Provider
        const StorageProvider& provider = vessel->get_storage_provider();

        //Start backing up files
        for ( ;; )
        {

            //Create a new UploadManager
            std::unique_ptr<UploadManager> upload_manager = std::make_unique<UploadManager>(provider);

            try
            {
                upload_manager->run_uploader();
            }
            catch( const std::exception& ex )
            {
                log->add_exception(ex);
                continue;

            }

            upload_manager.reset();

            boost::this_thread::sleep( boost::posix_time::seconds(5) );
            std::cout << "Restarting file uploader after 5 seconds..." << '\n';


        }

    });

    //Allow threads to do their work
    tpool.join_all();

    #ifdef _WIN32
        WSACleanup();
    #endif

    return 0;

}
