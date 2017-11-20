#ifndef TYPES_H
#define TYPES_H

#include <string>
#include <vector>

namespace Backup {
    namespace Types {

        struct file_directory
        {
            std::string folder_name;
            std::string path;
            unsigned int directory_id;
            unsigned long last_modified;
            size_t filesize;
        };

        struct file_data
        {
            std::string filename;
            std::string file_ext;
            std::string parent_path;
            size_t filesize;
            size_t compressed_size;
            unsigned long last_modified;
            unsigned int directory_id;
            unsigned int file_id;
        };

    }
}

#endif // TYPES_H
