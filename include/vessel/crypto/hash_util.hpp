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
        class Hash
        {
            public:
                Hash();
                ~Hash();

                static std::shared_ptr<unsigned char> get_sha1_hash_ptr( const std::string& data );
                static std::string get_sha1_hash(const std::string& data);
                static std::string get_sha256_hash(const std::string& data);
                static std::string get_md5_hash(const std::string& data, bool base64=false);
                static std::string get_base64(const std::string& data);
                static std::string get_hmac_256(const std::string& key, const std::string& data, bool hex=true);
                static std::string base64_decode( const std::string& str );

            private:
                std::string m_hash;

        };
    }
}

#endif
