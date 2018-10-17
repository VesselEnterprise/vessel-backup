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

        /*!
          \class FileIterator
          \brief Helper class for scanning files and adding them to the database
        */
        class FileIterator
        {
            public:
                FileIterator(const BackupDirectory& bd);

                /*!
                  \fn void scan();
                  \brief Scans the filesystem for files and directories
                */
                void scan();

                /*!
                  \fn std::string get_base_path();
                  \brief Returns the default scan path for the current user
                  \return Returns the default scan path for the current user
                */
                std::string get_base_path();

                /*!
                  \fn bool is_ignore_dir(const boost::filesystem::path& p, int level);
                  \brief Checks if the directory is being ignored
                  \param level Enforces the nesting level of the directory
                  \return Returns true if the directory is being ignored
                */
                bool is_ignore_dir(const boost::filesystem::path& p, int level);

                /*!
                  \fn bool is_ignore_ext(const std::string& ext);
                  \brief Checks if the file extension is ignored
                  \param ext The file extension (eg. .jpg)
                  \return Returns true if the file extension is ignored
                */
                bool is_ignore_ext(const std::string& ext);

                /*!
                  \fn int add_file( const BackupFile& bf );
                  \brief Adds a file to the database
                  \return Returns true if the file extension is ignored
                */
                bool add_file( const BackupFile& bf );

                /*!
                  \fn int add_directory( const BackupDirectory& bd );
                  \brief Adds a directory to the database
                  \return Returns the database directory id (integer) or -1 if the operation fails
                */
                int add_directory( const BackupDirectory& bd );

            private:
                BackupDirectory m_base_dir;
                BackupDirectory m_current_dir;
                fs::recursive_directory_iterator m_itr;
                fs::recursive_directory_iterator m_itr_end; //Never changes
                LocalDatabase* m_ldb;
                bool m_skip_dir_periods; //Directories that start with a period
                Log* m_log;

                /*!
                  \fn bool skip_dir(const fs::path& p, int level);
                  \brief Adds a directory to skip to the database
                  \return Returns true if the directory was skipped
                */
                bool skip_dir(const fs::path& p, int level);

                /*!
                  \fn unsigned long get_last_write_t( const fs::path& p );
                  \brief Gets a UNIX timestamp of the last write time of the file path
                  \return Gets a UNIX timestamp of the last write time of the file path
                */
                unsigned long get_last_write_t( const fs::path& p );

        };

    }

}

#endif // FILE_H
