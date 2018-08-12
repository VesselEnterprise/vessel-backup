#ifndef TYPES_H
#define TYPES_H

#include <string>
#include <vector>

namespace Vessel {
    namespace Types {

        struct file_directory
        {
            std::string folder_name;
            std::string path;
            unsigned int directory_id;
            unsigned long last_modified;
            size_t filesize;
        };
        typedef struct file_directory file_directory;

        enum http { HTTP_GET, HTTP_POST, HTTP_PUT, HTTP_DELETE };

        struct http_request
        {
            unsigned int request_type;
            std::string uri;
            std::string content_type;
            std::string data;
            std::string auth_token;
        };
        typedef struct request_type request_type;

        struct StorageProvider
        {
            std::string provider_id;
            std::string provider_name;
            std::string description;
            std::string server;
            std::string region;
            std::string bucket_name;
            std::string storage_path;
            std::string provider_type;
            int priority;
        };
        typedef struct StorageProvider StorageProvider;

    }
}

#endif // TYPES_H
