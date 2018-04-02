#include "hash_util.hpp"

using namespace Backup::Utilities;

std::string Hash::get_sha1_hash(const std::string& data)
{
    using namespace CryptoPP;

    SHA1 hash;
    std::string digest;

    StringSource s(data, true, new HashFilter(hash, new HexEncoder( new StringSink(digest), false ) ) );

    return digest;
}

std::unique_ptr<unsigned char*> Hash::get_sha1_hash_raw(const std::string& data)
{
    using namespace CryptoPP;

    unsigned char* digest = new unsigned char[CryptoPP::SHA1::DIGESTSIZE];

    SHA1 hash;
    hash.CalculateDigest( digest, (unsigned char*)data.c_str(), data.size() );

    return std::make_unique<unsigned char*>(digest);
}

std::string Hash::get_sha256_hash(const std::string& data)
{
    using namespace CryptoPP;

    SHA256 hash;
    std::string digest;

    StringSource s(data, true, new HashFilter(hash, new HexEncoder( new StringSink(digest), false ) ) );

    return digest;
}

std::string Hash::get_hmac_256( const std::string& key, const std::string& data, bool hex )
{

    using namespace CryptoPP;

    std::string mac;
    std::string encoded;

    try
    {
        HMAC<SHA256> hmac( (unsigned char*)key.c_str(), key.size() );

        StringSource ss(
            data,
            true,
            new HashFilter( hmac, new StringSink(mac) )
        );
    }
    catch(const CryptoPP::Exception& e)
    {
        //Add log error here?
        return "";
    }

    if ( !hex ) //Do not hex encode
        return mac;

    // Pretty print
    StringSource ss3(mac, true,
        new HexEncoder(
            new StringSink(encoded), false
        )
    );

    return encoded;

}
