#ifndef LOCALDB_H
#define LOCALDB_H

#include <iostream>
#include <string>
#include <fstream>
#include <sqlite3.h>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#include "types.hpp"
#include "log.hpp"
#include "version.hpp"

#ifdef _WIN32
    #include <winsock2.h>
#elif __unix
    #include <unistd.h>
    #include <sys/types.h>
    #include <pwd.h>
    #include <sys/utsname.h>
#endif

#define DB_FILENAME "local.db"

namespace Backup{
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

                bool is_ignore_dir(const boost::filesystem::path& p, int level);
                bool is_ignore_ext(const std::string& ext);

                std::string get_setting_str(const std::string& s);
                int get_setting_int(const std::string&s );

                template <typename T>
                bool update_setting(const std::string& key, const T& val );

                //Update file extension count
                bool update_ext_count( const std::string& ext, int total );

                unsigned int add_file( Backup::Types::file_data* fd );
                unsigned int add_directory( Backup::Types::file_directory* fd );

                std::string get_last_err();

                void update_global_settings();

                void update_client_settings( const std::string& s );

                //Scans backup_file table and checks if files exist and mark for deletion if necessary
                void clean();

            private:

                LocalDatabase();

                int open_db(const std::string& filename);
                void close_db();
                void clean_dirs();
                void clean_files();

                sqlite3* m_db;
                int m_err_code;
                Backup::Logging::Log* m_log;

            protected:
                ~LocalDatabase();
        };

    }
}

#endif // LOCALDB_H
