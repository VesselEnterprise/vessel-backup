#ifndef LOCALDB_H
#define LOCALDB_H

#include <iostream>
#include <string>
#include <fstream>
#include <sqlite3.h>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#include <vessel/database/db_exception.hpp>
#include <vessel/types.hpp>
#include <vessel/log/log.hpp>
#include <vessel/version.hpp>

#ifdef _WIN32
    #include <winsock2.h>
#elif __unix
    #include <unistd.h>
    #include <sys/types.h>
    #include <pwd.h>
    #include <sys/utsname.h>
#endif

#define VACUUM_ON_LOAD 1
#define DB_FILENAME "local.db"

using namespace Vessel::Exception;

namespace Vessel{
    namespace Database {

        class LocalDatabase
        {

            public:

                static LocalDatabase& get_database()
                {
                    static LocalDatabase instance;
                    return instance;
                }

                LocalDatabase(LocalDatabase const&) = delete;
                void operator=(LocalDatabase const&) = delete;

                std::string get_setting_str(const std::string& s);
                int get_setting_int(const std::string&s );

                template <typename T>
                bool update_setting(const std::string& key, const T& val );

                static std::string get_sqlite_str(const void* data);

                //Update file extension count
                bool update_ext_count( const std::string& ext, int total );

                std::string get_last_err();

                void update_global_settings();

                void update_client_settings( const std::string& s );

                //Scans backup_file table and checks if files exist and mark for deletion if necessary
                void clean();

                void build_queue();

                void start_transaction();
                void end_transaction();

                void purge_file(const unsigned char* file_id);

            //protected:
                sqlite3* get_handle();

            private:

                LocalDatabase();

                int vacuum_db();

                int open_db(const std::string& filename);
                void close_db();
                void clean_dirs();
                void clean_files();

                sqlite3* m_db;
                int m_err_code;
                Vessel::Logging::Log* m_log;

            protected:
                ~LocalDatabase();
        };

    }
}

#endif // LOCALDB_H
