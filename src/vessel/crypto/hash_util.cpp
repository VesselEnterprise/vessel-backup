#include <vessel/crypto/hash_util.hpp>

using namespace Vessel::Utilities;

std::string Hash::get_sha1_hash(const std::string& data)
{

    SHA1 hash;
    std::string digest;

    StringSource s(data, true, new HashFilter(hash, new HexEncoder( new StringSink(digest), false ) ) );

    return digest;
}

std::shared_ptr<unsigned char> Hash::get_sha1_hash_ptr(const std::string& data)
{

    std::shared_ptr<unsigned char> digest( new unsigned char[CryptoPP::SHA1::DIGESTSIZE] );

    SHA1 hash;
    hash.CalculateDigest( digest.get(), (unsigned char*)data.c_str(), data.size() );

    return digest;

}

std::string Hash::get_sha256_hash(const std::string& data)
{

    SHA256 hash;
    std::string digest;

    StringSource s(data, true, new HashFilter(hash, new HexEncoder( new StringSink(digest), false ) ) );

    return digest;
}

std::string Hash::get_md5_hash(const std::string& data, bool base64)
{

    /*

    unsigned char digest[ Weak::MD5::DIGESTSIZE ];

    Weak::MD5 hash;
    hash.CalculateDigest( digest, (const unsigned char*)data.c_str(), data.length() );

    HexEncoder encoder;
    std::string output;

    encoder.Attach( new StringSink( output ) );
    encoder.Put( digest, sizeof(digest) );
    encoder.MessageEnd();

    boost::algorithm::to_lower(output);

    */

    Weak::MD5 hash;
    std::string digest;

    if ( !base64 ) {
        StringSource s(data, true, new HashFilter(hash, new HexEncoder( new StringSink(digest), false ) ) );
    }
    else {
        StringSource s(data, true, new HashFilter(hash, new Base64Encoder( new StringSink(digest), false ) ) );
    }

    return digest;
}

std::string Hash::get_base64( const std::string& data )
{

    //unsigned char* decoded = (unsigned char*)data.c_str();
    std::string encoded;

    StringSource ss(data, true, new Base64Encoder( new StringSink(encoded), false ) );

    return encoded;
}

std::string Hash::get_hmac_256( const std::string& key, const std::string& data, bool hex )
{

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

std::string Hash::base64_decode(const std::string& str)
{

    std::string decoded;

    StringSource ss(str, true,
        new Base64Decoder(
            new StringSink(decoded)
        ) // Base64Decoder
    ); // StringSource

    return decoded;

}
