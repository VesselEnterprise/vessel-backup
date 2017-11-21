#ifndef FILE_H
#define FILE_H

#include <iostream>
#include <string>
#include <boost/filesystem.hpp>
#include "local_db.hpp"
#include "types.hpp"
#include "log.hpp"

namespace fs = boost::filesystem;
using namespace Backup::Database;

namespace Backup {

    class FileIterator
    {

        public:
            FileIterator(const std::string& path);
            ~FileIterator();

            void scan();

            void set_local_db(LocalDatabase* db);

            std::string get_base_path();

        private:
            fs::path m_base_path;
            Backup::Types::file_directory m_current_dir;
            fs::recursive_directory_iterator m_itr;
            fs::recursive_directory_iterator m_itr_end; //Never changes
            LocalDatabase* m_ldb;
            bool m_skip_dir_periods; //Directories that start with a period
            Backup::Logging::Log* m_log;

            bool skip_dir(const fs::path& p, int level);
            unsigned long get_last_write_t( const fs::path& p );

    };

}

#endif // FILE_H
