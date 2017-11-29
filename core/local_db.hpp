#ifndef LOCALDB_H
#define LOCALDB_H

#include <iostream>
#include <string>
#include <fstream>
#include <sqlite3.h>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include "types.hpp"
#include "log.hpp"

#ifdef __unix
    #include <unistd.h>
    #include <sys/types.h>
    #include <pwd.h>
#endif

namespace Backup{
    namespace Database {

        class LocalDatabase
        {
            public:
                LocalDatabase(const std::string& filename);
                ~LocalDatabase();

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

                //Scans backup_file table and checks if files exist and mark for deletion if necessary
                void clean();

            private:

                int open_db(const std::string& filename);
                void close_db();
                void clean_dirs();
                void clean_files();

                sqlite3* m_db;
                int m_err_code;
                Backup::Logging::Log* m_log;
        };

    }
}

#endif // LOCALDB_H
