#ifndef S3CLIENT_H
#define S3CLIENT_H

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>

#include <cryptopp/cryptlib.h>
#include <cryptopp/hmac.h>
#include <cryptopp/sha.h>

#include "http_client.hpp"
#include "http_request.hpp"


namespace Backup {
    namespace Networking {

        class S3Client : public HttpClient
        {

            public:

                S3Client(const std::string & uri);
                ~S3Client();

            private:
                std::string m_http_verb;
                std::string get_canonical_request();
                void make_headers();


        };

    }
}

#endif
