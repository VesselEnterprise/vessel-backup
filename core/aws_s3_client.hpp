#ifndef AWSS3CLIENT_H
#define AWSS3CLIENT_H

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <iterator>
#include <map>

#include <boost/algorithm/string.hpp>
#include <boost/date_time/date_facet.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <cryptopp/cryptlib.h>
#include <cryptopp/hmac.h>
#include <cryptopp/sha.h>

#include "http_client.hpp"
#include "http_request.hpp"
#include "file.hpp"
#include "hash_util.hpp"

#define AWS_ACCESS_ID ""
#define AWS_SECRET_KEY ""
#define AWS_REGION "us-east-2"


namespace Backup {
    namespace Networking {

        class AwsS3Client : public HttpClient
        {

            public:

                AwsS3Client(const std::string & uri);
                ~AwsS3Client();

                void set_http_request( Backup::Networking::HttpRequest* r );

                /*! \fn void set_file( Backup::File::BackupFile* bf );
                    \brief Initializes the file internally to the object prior to send the file to S3
                */
                void set_file( Backup::File::BackupFile* bf );

                /*! \fn void set_chunked(bool flag);
                    \brief Sends the file to S3 as a multi part upload
                */
                void set_chunked(bool flag);

                /*! \fn void set_chunk_size(size_t chunk_size);
                    \brief Sets the chunk size in bytes for multipart uploads
                */
                void set_chunk_size(size_t chunk_size);

                /*! \fn std::string get_canonical_request();
                    \brief Returns the AWS S3 canonical request
                    \return Returns the AWS S3 canonical request
                */
                std::string get_canonical_request();

                /*! \fn std::string get_string_to_sign();
                    \brief Returns the string to sign (metadata) to be used when calculating the signature.
                    \return Returns the String to Sign for the AWS V4 signature
                */
                std::string get_string_to_sign();

                /*! \fn std::string get_signature_v4();
                    \brief Returns the AWS V4 signature derived from HMAC authentication codes
                    \return Returns the AWS V4 signature derived from HMAC authentication codes
                */
                std::string get_signature_v4();

                /*! \fn std::string get_chunk_signature_v4(const std::string& prev_signature);
                    \param prev_signature The signature of the previous chunk or the seed signature if the first chunk
                    \brief Returns the AWS V4 signature for a chunked upload
                    \return Returns the AWS V4 signature for a chunked upload
                */
                std::string get_chunk_signature_v4(const std::string& prev_signature);

                /*! \fn bool upload();
                    \brief Uploads a single file to the AWS S3 REST API
                    \return Returns true if the upload was successful, or false if there was an error
                */
                bool upload();

                /*! \fn bool upload_part();
                    \brief Uploads a single file to the AWS S3 REST API
                    \return Returns true if the upload was successful, or false if there was an error
                */
                bool upload_part(int part_number, const std::string& prev_signature);

                /*! \fn std::string get_last_signature();
                    \brief Returns the signature for the previous upload chunk
                    \return Returns the signature for the previous upload chunk
                */
                std::string get_last_signature();

            private:
                Backup::Database::LocalDatabase* m_ldb;
                Backup::File::BackupFile* m_file;
                std::map<std::string,std::string> m_headers;
                std::string m_http_verb;
                std::string m_query_str;
                std::string m_region; //AWS Region for API operations
                std::string m_amzdate; //ISO Date of the request
                std::string m_amzdate_short; //Shortened version of the AMZ date eg. 20130524
                std::string m_amzdate_clean; //Example: Fri, 24 May 2013 00:00:00 GMT
                std::string m_content_sha256; //SHA-256 hash of the file contents
                std::string m_previous_signature; //The previous signature used for multipart uploads
                std::string m_request_payload;
                int m_current_chunk; //For multipart uploads, the current chunk index
                bool m_chunked; //Indicates whether or not a multipart upload
                size_t m_chunk_size; //Chunk size bytes for multipart uploads
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

                /*! \fn std::string get_signing_key();
                    \brief Returns the key used to sign AWS S3 requests
                    \return Returns the key used to sign AWS S3 requests
                */
                std::string get_signing_key();

                /*! \fn void build_request_headers();
                    \brief Internal call to store request headers in a std::map
                */
                void build_request_headers();

                /*! \fn init_amz_date();
                    \brief Internal call to build the dates used by the AWS S3 API. Includes ISO8601 date and nice version
                */
                void init_amz_date();

                /*! \fn  size_t get_current_part_size();
                    \brief Returns the total size in bytes of the current part to be uploaded
                    \return Returns the total size in bytes of the current part to be uploaded
                */
                size_t get_current_part_size();

        };

    }
}

#endif
