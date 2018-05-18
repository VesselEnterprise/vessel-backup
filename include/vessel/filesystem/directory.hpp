#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <iostream>
#include <string>
#include <memory>
#include <boost/filesystem.hpp>
#include <cryptopp/sha.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>

#include <vessel/crypto/hash_util.hpp>

namespace fs = boost::filesystem;

namespace Backup {
    namespace File {
        class BackupDirectory
        {
            /*! \struct dir_attrs
                \brief Struct containing common directory attributes
            */
            struct dir_attrs
            {
                std::string directory_name;
                std::string path;
                std::string parent_path;
                std::string relative_path;
                std::string canonical_path;
                size_t file_size;
                unsigned long last_write_time;
            };

            public:
                BackupDirectory(const boost::filesystem::path& fp );
                BackupDirectory(const BackupDirectory& bd );
                ~BackupDirectory();

                /*! \fn void set_path(const std::string& fp);
                    \brief Assigns a new directory path to the object. Setting the path triggers all of the associated member attributes and variables to be updated with private method update_attributes()
                */
                void set_path(const std::string& fp);

                /*! \fn void set_path(const fs::path& file_path);
                    \brief Assigns a new file path to the object. Setting the path triggers all of the associated member attributes and variables to be updated with private method update_attributes()
                */
                void set_path(const fs::path& file_path);

                /*! \fn std::string get_dir_name() const;
                    \brief
                    \return Returns the name of the directory
                */
                std::string get_dir_name() const;

                /*! \fn size_t get_file_size();
                    \brief
                    \return Returns the total filesize of the directory
                */
                size_t get_file_size() const;

                /*! \fn std::string get_unique_id();
                    \brief
                    \return Returns a SHA-1 hash of the directory path
                */
                std::string get_unique_id() const;

                /*! \fn std::unique_ptr<unsigned char*> get_unique_id_raw() const;
                    \brief
                    \return Returns a raw SHA-1 hash as a shared pointer.
                */
                std::unique_ptr<unsigned char*> get_unique_id_raw() const;

                /*! \fn unsigned int get_directory_id();
                    \brief
                    \return Returns the database ID of the directory
                */
                unsigned int get_directory_id() const;

                /*! \fn void set_directory_id(unsigned int id);
                    \brief Sets the associated database directory ID for the file (object only)
                */
                void set_directory_id(unsigned int id);

                /*! \fn std::string get_path();
                    \brief
                    \return Returns the path of the directory as a string
                */
                std::string get_path() const;

                /*! \fn std::string get_parent_path();
                    \brief
                    \return Returns the parent path of the directory as a string
                */
                std::string get_parent_path() const;

                 /*! \fn std::string get_relative_path();
                    \brief
                    \return Returns the relative path of the directory as a string
                */
                std::string get_relative_path() const;

                /*! \fn std::string get_canonical_path();
                    \brief
                    \return Returns the canonical (complete/full) path of the directory as a string
                */
                std::string get_canonical_path() const;

                /*! \fn unsigned long get_last_modified();
                    \brief
                    \return Returns the last write time of the directory as a Unix timestamp
                */
                unsigned long get_last_modified() const;

                /*! \fn bool exists();
                    \brief
                    \return Returns whether or not the directory exists
                */
                bool exists();

            private:

                boost::filesystem::path m_dir_path; //!< Boost::FileSystem path of the file
                std::string m_unique_id; //!< SHA-1 hash of the file path
                dir_attrs m_dir_attrs; //!< Struct containing common directory attributes
                unsigned int m_directory_id; //!< Database ID of the directory

                /*! \fn void update_attributes()
                    \brief Updates private member variables for directory properties. Called in constructor and when assigning an object a new file path
                */
                void update_attributes();

                /*! \fn std::string std::string calculate_unique_id();
                    \brief Calculates the SHA-1 hash of the directory path
                */
                std::string calculate_unique_id() const;


        };
    }
}

#endif
