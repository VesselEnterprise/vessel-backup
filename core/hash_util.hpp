#ifndef CRYPTO_H
#define CRYPTO_H

#include <iostream>
#include <string>
#include <memory>

#include <cryptopp/hmac.h>
#include <cryptopp/sha.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>

namespace Backup {
    namespace Utilities {
        class Hash
        {
            public:
                Hash();
                ~Hash();

                static std::unique_ptr<unsigned char*> get_sha1_hash_raw( const std::string& data );
                static std::string get_sha1_hash(const std::string& data);
                static std::string get_sha256_hash(const std::string& data);
                static std::string get_hmac_256(const std::string& key, const std::string& data, bool hex=true);

            private:
                std::string m_hash;

        };
    }
}

#endif
