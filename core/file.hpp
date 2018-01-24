#ifndef FILE_HPP
#define FILE_HPP

#include <iostream>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <boost/filesystem.hpp>
#include <cryptopp/sha.h>
#include <cryptopp/files.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>

#define BACKUP_LARGE_SZ 52428800 //Default size in bytes of what should be considered a larger file (50MB)
#define BACKUP_CHUNK_SZ 52428800 //Default chunk size if not defined in DB (10MB)

namespace fs = boost::filesystem;

namespace Backup {
    namespace File {

        class BackupFile
        {


            /*! \struct file_attrs
                \brief Struct containing common file attributes
            */
            struct file_attrs
            {
                std::string file_name;
                std::string file_type;
                std::string file_path;
                std::string parent_path;
                std::string relative_path;
                size_t file_size;
                unsigned long last_write_time;
            };

            public:
                BackupFile(const std::string& file_path);
                BackupFile(const fs::path& file_path);
                ~BackupFile();

                /*! \fn void set_path(const std::string& fp);
                    \brief Assigns a new file path to the object. Setting the path triggers all of the associated member attributes and variables to be updated with private method update_attributes()
                */
                void set_path(const std::string& fp);

                /*! \fn void set_path(const fs::path& file_path);
                    \brief Assigns a new file path to the object. Setting the path triggers all of the associated member attributes and variables to be updated with private method update_attributes()
                */
                void set_path(const fs::path& file_path);

                /*! \fn std::string get_file_name();
                    \brief
                    \return Returns the filename
                */
                std::string get_file_name();

                /*! \fn size_t get_file_size();
                    \brief
                    \return Returns the file size
                */
                size_t get_file_size();

                /*! \fn std::string get_file_type();
                    \return Returns the file extension as a string
                */
                std::string get_file_type();

                /*! \fn std::string get_file_contents();
                    \return Returns the file contents
                */
                std::string get_file_contents();

                /*! \fn std::string get_unique_id();
                    \brief
                    \return Returns a SHA-1 hash of the file path
                */
                std::string get_unique_id();

                /*! \fn std::string get_hash();
                    \brief
                    \return Returns a SHA-1 hash of the file contents
                */
                std::string get_hash();

                /*! \fn std::string get_hash(const std::string& content);
                    \brief
                    \return Returns a SHA-1 hash of the provided data
                */
                std::string get_hash(const std::string& data);

                /*! \fn unsigned int get_directory_id();
                    \brief
                    \return Returns the database ID of the directory
                */
                unsigned int get_directory_id();


                /*! \fn void set_directory_id(unsigned int id);
                    \brief Sets the associated database directory ID for the file (object only)
                */
                void set_directory_id(unsigned int id);

                /*! \fn unsigned int get_file_id();
                    \brief
                    \return Returns the database ID of the file
                */
                unsigned int get_file_id();

                /*! \fn void set_file_id(unsigned int id);
                    \brief Sets the associated database file ID for the file (object only)
                */
                void set_file_id(unsigned int id);

                /*! \fn std::string get_file_path();
                    \brief
                    \return Returns the complete file path as a string
                */
                std::string get_file_path();

                /*! \fn std::string get_parent_path();
                    \brief
                    \return Returns the parent path of the file as a string
                */
                std::string get_parent_path();

                 /*! \fn std::string get_relative_path();
                    \brief
                    \return Returns the relative path of the file as a string
                */
                std::string get_relative_path();

                /*! \fn unsigned long get_last_modified();
                    \brief
                    \return Returns the last write time of the file as a Unix timestamp
                */
                unsigned long get_last_modified();

                /*! \fn bool exists();
                    \brief
                    \return Returns whether or not the file exists
                */
                bool exists();

                /*! \fn bool is_compressed();
                    \brief
                    \return Returns whether or not the file is compressed
                */
                bool is_compressed();

                /*! \fn void set_chunk_size( size_t chunk_sz );
                    \brief Sets the size of chunks of bytes returned from a file part for multi part uploads
                */
                void set_chunk_size( size_t chunk_sz );

                /*! \fn std::string get_file_part(unsigned int num);
                    \brief
                    \return Returns the bytes of a file for the given part number
                */
                std::string get_file_part(unsigned int num);

                /*! \fn unsigned int get_total_parts();
                    \brief
                    \return Returns the total number of file parts based on the chunk size
                */
                unsigned int get_total_parts();

                /*! \fn void set_upload_id();
                    \brief Sets the server upload id for the file
                */
                void set_upload_id(unsigned int id);

                /*! \fn unsigned int get_upload_id();
                    \brief
                    \return Returns the server upload id from the initiated upload
                */
                unsigned int get_upload_id();



            private:
                boost::filesystem::path m_file_path; //!< Boost::FileSystem path of the file
                std::string m_hash; //!< SHA-1 hash of the file contents
                std::string m_unique_id; //!< SHA-1 hash of the file path
                std::string m_content; //!< Contents of the file (binary)
                file_attrs m_file_attrs; //!< Struct containing common file attributes
                unsigned int m_file_id; //!< Database ID of the file
                unsigned int m_directory_id; //!< Database ID of the parent directory
                unsigned int m_upload_id; //!< Server Upload ID of the file
                size_t m_chunk_size; //!< Size in bytes in a file part for multi part uploads

                /*! \fn void update_attributes()
                    \brief Updates private member variables for file properties. Called in constructor and when assigning an object a new file path
                */
                void update_attributes();

                /*! \fn std::string std::string calculate_unique_id();
                    \brief Calculates the SHA-1 hash of the file path
                */
                std::string calculate_unique_id();

        };

    }

}

#endif // FILE_HPP
