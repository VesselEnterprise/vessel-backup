#ifndef LOCALDB_H
#define LOCALDB_H

#include <iostream>
#include <string>
#include <fstream>
#include <sqlite3.h>
#include <boost/lexical_cast.hpp>

namespace Backup{
    namespace Database {

        class LocalDatabase
        {
            public:
                LocalDatabase(const std::string& filename);
                ~LocalDatabase();

                bool is_ignore_dir(const std::string& dir_name, int level);
                bool is_ignore_ext(const std::string& ext);

                std::string get_setting_str(const std::string& s);
                int get_setting_int(const std::string&s );

                template <typename T>
                bool update_setting(const std::string& key, const T& val );

                //Update file extension count
                bool update_ext_count( const std::string& ext, int total );

                std::string get_last_err();

            private:

                int open_db(const std::string& filename);
                void close_db();

                sqlite3* m_db;
                int m_err_code;
        };

    }
}

#endif // LOCALDB_H
