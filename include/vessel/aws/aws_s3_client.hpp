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
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>

#include <cryptopp/cryptlib.h>
#include <cryptopp/hmac.h>
#include <cryptopp/sha.h>

#include <vessel/network/http_client.hpp>
#include <vessel/network/http_request.hpp>
#include <vessel/filesystem/file.hpp>
#include <vessel/crypto/hash_util.hpp>
#include <vessel/aws/aws_exception.hpp>

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

                enum AwsFlags
                {
                    NoFlags = 0,
                    ReducedRedundancy = 1,
                    Multipart = 2,
                    Streaming = 4,
                    Encrypted = 6,
                    SkipMultiInit = 8 //Skips the initialization of a multipart upload if the upload id already exists
                };

                struct etag_pair
                {
                    int part_number;
                    std::string etag;
                };
                typedef struct etag_pair etag_pair;

                friend AwsFlags operator|(AwsFlags a, AwsFlags b)
                {return static_cast<AwsFlags>(static_cast<int>(a) | static_cast<int>(b));}

                friend AwsFlags operator&(AwsFlags a, AwsFlags b)
                {return static_cast<AwsFlags>(static_cast<int>(a) & static_cast<int>(b));}

                void set_http_request( Backup::Networking::HttpRequest* r );

                /*! \fn void init_upload( Backup::File::BackupFile* bf, AwsFlags flags = AwsFlags::ReducedRedundancy );
                    \brief Initializes the AWS S3 upload
                */
                void init_upload( Backup::File::BackupFile* bf, AwsFlags flags = AwsFlags::ReducedRedundancy );

                /*! \fn void set_part_size(size_t part_size);
                    \brief Sets the part size in bytes for multipart uploads
                */
                void set_part_size(size_t part_size);

                /*! \fn bool upload();
                    \brief Uploads a single file to the AWS S3 REST API
                    \return Returns true if the upload was successful, or false if there was an error
                */
                bool upload();

                /*! \fn std::string upload_part(int part_number, const std::string& upload_id );
                    \brief Uploads a single file to the AWS S3 REST API
                    \return Returns the Etag for the upload part if successful
                */
                std::string upload_part(int part_number, const std::string& upload_id );

                /*! \fn std::string complete_multipart_upload(const std::vector<etag_pair>& etags, const std::string& upload_id)
                    \brief Completes a multipart upload and returns the ETag for the file upload
                    \return Completes a multipart upload and returns the ETag for the file upload
                */
                std::string complete_multipart_upload(const std::vector<etag_pair>& etags, const std::string& upload_id);

                /*! \fn bool upload_stream_chunk(int part_number, const std::string& prev_signature );
                    \brief Streaming file upload to the AWS S3 REST API
                    \return Returns true if the upload was successful, or false if there was an error
                */
                bool upload_stream_chunk(int part_number, const std::string& prev_signature );

                /*! \fn std::string get_last_signature();
                    \brief Returns the signature for the previous upload part
                    \return Returns the signature for the previous upload part
                */
                std::string get_last_signature();

                /*! \fn std::string get_upload_id();
                    \brief Returns the upload ID for Multipart uploads
                    \return Returns the upload ID for Multipart uploads
                */
                std::string get_upload_id();

                /*! \fn std::string set_upload_id(const std::string& upload_id);
                    \brief Sets the upload id of the AWS S3 upload. Can be used for resuming uploads or performing other operations
                */
                void set_upload_id(const std::string& upload_id);

                /*! \fn void set_file(Backup::File::BackupFile* bf);
                    \brief Sets a pointer to the BackupFile
                */
                void set_file(Backup::File::BackupFile* bf);

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
                std::string m_file_content; //Content of the current file or part
                std::string m_content_md5; //MD5 hash of the current file or part
                std::string m_previous_signature; //The previous signature used for streaming uploads
                std::string m_request_payload;
                std::string m_upload_id; //Upload ID returned for Multipart uploads
                int m_current_part; //For multipart uploads, the current part index
                bool m_multipart; //Indicates whether or not a multipart upload
                bool m_streaming; //Indicates whether or not a streaming upload (for unknown filesizes)
                bool m_reduced_redundancy; //Default = False
                size_t m_part_size; //part size bytes for multipart uploads
                Backup::Networking::HttpRequest* m_http_request;

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

                /*! \fn std::string get_stream_signature_v4(const std::string& prev_signature);
                    \param prev_signature The signature of the previous part or the seed signature if the first part
                    \brief Returns the AWS V4 signature for a parted upload
                    \return Returns the AWS V4 signature for a parted upload
                */
                std::string get_stream_signature_v4(const std::string& prev_signature);

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

                /*! \fn void init_multipart_upload();
                    \brief Initializes a multipart upload and sets the upload id. Called internally when the Multipart flag is set
                */
                void init_multipart_upload();

                /*! \fn std::string get_5mb_test_str();
                    \brief Returns a 5MB text string used for testing the Multipart upload API
                    \return Returns a 5MB text string used for testing the Multipart upload API
                */
                std::string get_5mb_test_str();

                /*! \fn std::string parse_upload_id( const std::string& response );
                    \brief Returns the upload id from the response after initiating a multipart upload
                    \return Returns the upload id from the response after initiating a multipart upload
                */
                std::string parse_upload_id( const std::string& response );

                /*! \fn std::string parse_etag( const std::string& response );
                    \brief Returns the etag for a file part upload returned from the S3 API
                    \return Returns the etag for a file part upload returned from the S3 API
                */
                std::string parse_etag( const std::string& response );


        };

    }
}

#endif
