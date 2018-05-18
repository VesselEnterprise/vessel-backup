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

        enum http { HTTP_GET, HTTP_POST, HTTP_PUT, HTTP_DELETE };

        struct http_request
        {
            unsigned int request_type;
            std::string uri;
            std::string content_type;
            std::string data;
            std::string auth_token;
        };

    }
}

#endif // TYPES_H