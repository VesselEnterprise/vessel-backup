#ifndef AWSS3CLIENT_H
#define AWSS3CLIENT_H

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>

#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <cryptopp/cryptlib.h>
#include <cryptopp/hmac.h>
#include <cryptopp/sha.h>

#include "http_client.hpp"
#include "http_request.hpp"
#include "file.hpp"


namespace Backup {
    namespace Networking {

        class AwsS3Client : public HttpClient
        {

            public:

                AwsS3Client(const std::string & uri);
                ~AwsS3Client();

                void set_http_request( Backup::Networking::HttpRequest* r );
                void set_file( Backup::File::BackupFile* bf );

                void set_chunked(bool flag);

            private:
                Backup::Database::LocalDatabase* m_ldb;
                Backup::File::BackupFile* m_file;
                std::map<std::string,std::string> m_headers;
                std::string m_http_verb;
                std::string m_query_str;
                std::string get_canonical_request();
                bool m_chunked;
                Backup::Networking::HttpRequest* m_http_request;

                /*! \fn std::string get_amz_headers();
                    \brief Returns the amz headers used for the canonical request
                    \return Returns the amz headers used for the canonical request
                */
                std::string get_amz_headers();

                /*! \fn std::string get_signed_headers();
                    \brief Returns the signed amz headers used for the canonical request
                    \return Returns the signed amz headers used for the canonical request
                */
                std::string get_signed_headers();


        };

    }
}

#endif
