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

    //Create new instance of local database
    LocalDatabase* db = &LocalDatabase::get_database();

    //Update global settings
    db->update_global_settings();

    std::string vessel_host = db->get_setting_str("master_server");

    //Create new instance of Vessel client
    boost::shared_ptr<VesselClient> vessel = boost::make_shared<VesselClient>(vessel_host);

    #ifdef _WIN32
        WSACleanup();
    #endif

    return 0;

}
