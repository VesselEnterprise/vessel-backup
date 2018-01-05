#ifndef FILE_HPP
#define FILE_HPP

#include <iostream>
#include <string>
#include <boost/filesystem.hpp>

namespace Backup {
    namespace File {

        class BackupFile
        {

            public:
                BackupFile(const std::string& file_path);
                ~BackupFile();

                void set_path(const std::string& fp);

                std::string get_file_name();
                size_t get_file_size();
                std::string get_file_type();
                std::string get_hash();
                std::string get_file_path();
                unsigned long get_last_modified();
                bool is_compressed();
                bool exists();

            private:
                boost::filesystem::path m_file_path;
                std::string m_file_name;
                std::string m_file_type;
                std::string m_hash;
                size_t m_file_size;
                unsigned long m_last_modified;

                void update_attributes();

        };

    }

}

#endif // FILE_HPP
