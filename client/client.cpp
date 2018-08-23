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
        ("with-auth-token", boost::program_options::value<std::string>(), "Sets the user auth token")
        ("with-client-token", boost::program_options::value<std::string>(), "Sets the client token")
        ("with-deployment-key", boost::program_options::value<std::string>(), "Sets the deployment key")
        ("scan-directory", boost::program_options::value<std::string>(), "Sets the initial scanning directory")
    ;

    boost::program_options::variables_map vm;
    boost::program_options::store( boost::program_options::parse_command_line(argc, argv, desc), vm);
    boost::program_options::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    //Create new instance of local database
    LocalDatabase* db = &LocalDatabase::get_database();

    //Update global settings
    db->update_global_settings();

    std::string vessel_host = db->get_setting_str("master_server");

    //Create new instance of Vessel client
    std::shared_ptr<VesselClient> vessel = std::make_shared<VesselClient>(vessel_host);

    //Verify/install client
    if ( !vessel->has_client_token() )
    {
        vessel->install_client();
    }

    //Create new upload QueueManager
    std::shared_ptr<QueueManager> queue_manager = std::make_shared<QueueManager>();

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

    //Create a file iterator object to scan the filesystem
    std::shared_ptr<FileIterator> file_iterator = std::make_shared<FileIterator>(scan_dir);

    //Add a new thread to the pool which scans the filesystem on intervals
    io_service.post([&](){
        for(;;)
        {
            //file_iterator->scan();
            boost::this_thread::sleep( boost::posix_time::seconds(10) );
            std::cout << "Restarting scan after 10 seconds..." << '\n';
        }
    });

    //Manage the upload queue
    io_service.post([&](){
        for(;;)
        {
            //queue_manager->rebuild_queue();
            boost::this_thread::sleep( boost::posix_time::seconds(10) );
            std::cout << "Restarting queue rebuild after 10 seconds..." << '\n';
        }
    });

    //Manage uploads
    io_service.post([&](){

        //Get Storage Provider
        const StorageProvider& provider = vessel->get_storage_provider();

        //Create a new UploadManager
        std::shared_ptr<UploadManager> upload_manager = std::make_shared<UploadManager>(provider);

        //Start backing up files
        for ( ;; )
        {
            try
            {
                upload_manager->run_uploader();
            }
            catch( const std::exception& ex )
            {
                std::cout << "Caught an exception: " << ex.what() << '\n';
                continue;

            }
            boost::this_thread::sleep( boost::posix_time::seconds(10) );
            std::cout << "Restarting file uploader after 10 seconds..." << '\n';
        }

    });

    //Allow threads to do their work
    tpool.join_all();

    #ifdef _WIN32
        WSACleanup();
    #endif

    return 0;

}
