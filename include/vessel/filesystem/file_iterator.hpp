#ifndef FILE_H
#define FILE_H

#include <iostream>
#include <string>
#include <memory>
#include <boost/filesystem.hpp>

#include <vessel/database/local_db.hpp>
#include <vessel/types.hpp>
#include <vessel/log/log.hpp>
#include <vessel/filesystem/file.hpp>
#include <vessel/filesystem/directory.hpp>

namespace fs = boost::filesystem;
using namespace Vessel::Logging;
using namespace Vessel::Database;

namespace Vessel {
    namespace File {
        class FileIterator
        {
            public:
                FileIterator(const BackupDirectory& bd);
                ~FileIterator();

                void scan();

                std::string get_base_path();

                bool is_ignore_dir(const boost::filesystem::path& p, int level);
                bool is_ignore_ext(const std::string& ext);

                int add_file( const BackupFile& bf );
                int add_directory( const BackupDirectory& bd );

            private:
                BackupDirectory m_base_dir;
                BackupDirectory m_current_dir;
                fs::recursive_directory_iterator m_itr;
                fs::recursive_directory_iterator m_itr_end; //Never changes
                LocalDatabase* m_ldb;
                bool m_skip_dir_periods; //Directories that start with a period
                Vessel::Logging::Log* m_log;

                bool skip_dir(const fs::path& p, int level);
                unsigned long get_last_write_t( const fs::path& p );

        };

    }

}

#endif // FILE_H
