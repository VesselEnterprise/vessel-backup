#ifndef CRYPTO_H
#define CRYPTO_H

#include <iostream>
#include <string>
#include <memory>

#include <boost/algorithm/string.hpp>

#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1

#include <cryptopp/hmac.h>
#include <cryptopp/base64.h>
#include <cryptopp/md5.h>
#include <cryptopp/sha.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>

using namespace CryptoPP;

namespace Vessel {
    namespace Utilities {

        /*! \class Hash Static Helper class for common crypto style functions */
        class Hash
        {
            public:
                Hash();

                /*! \fn static std::shared_ptr<unsigned char> get_sha1_hash_ptr( const std::string& data );
                    \brief Returns a shared_ptr to a binary id (unsigned char*)
                    \return Returns a shared_ptr to a binary id (unsigned char*)
                */
                static std::shared_ptr<unsigned char> get_sha1_hash_ptr( const std::string& data );

                /*! \fn static std::string get_sha1_hash(const std::string& data);
                    \brief Returns a sha-1 hash string
                    \return Returns a sha-1 hash string
                */
                static std::string get_sha1_hash(const std::string& data);

                /*! \fn static std::string get_sha256_hash(const std::string& data);
                    \brief Returns a sha-256 hash string
                    \return Returns a sha-256 hash string
                */
                static std::string get_sha256_hash(const std::string& data);

                /*! \fn static std::string get_md5_hash(const std::string& data, bool base64=false);
                    \brief Returns a md5 hash string
                    \param base64 Base64 encodes the string if set to true
                    \return Returns a md5 hash string
                */
                static std::string get_md5_hash(const std::string& data, bool base64=false);

                /*! \fn static std::string get_base64(const std::string& data);
                    \brief Returns a base64 encoded string
                    \return Returns a base64 encoded string
                */
                static std::string get_base64(const std::string& data);

                /*! \fn static std::string get_hmac_256(const std::string& key, const std::string& data, bool hex=true);
                    \brief Returns a HMAC SHA-256 string
                    \param key Key string used to generate the HMAC
                    \param data Data to be used when calculating the HMAC
                    \param hex Hex encodes the string if set to true
                    \return Returns a HMAC SHA-256 string
                */
                static std::string get_hmac_256(const std::string& key, const std::string& data, bool hex=true);

                /*! \fn static std::string base64_decode( const std::string& str );
                    \brief Returns a base64 decoded string
                    \param str Base64 encoded string to decode
                    \return Returns a base64 decoded string
                */
                static std::string base64_decode( const std::string& str );

            private:
                std::string m_hash;

        };
    }
}

#endif
