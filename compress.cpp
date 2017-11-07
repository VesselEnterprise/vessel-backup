#include "compress.hpp"

using namespace Backup::Compression;

Compressor::Compressor() : m_z_level(Z_COMP_LEVEL)
{

}

Compressor::~Compressor()
{

}

void Compressor::set_z_level(int level)
{
    m_z_level = level;
}

//Compress data
std::string Compressor::operator<<(const std::string & s)
{

    //Buffer
    std::string out_s;

    //Return code
    int ret;

    //Output Buffer
    char out[Z_CHUNK];

    /* allocate deflate state */
    z_stream zs = {0};

    /*
    zs.zalloc = Z_NULL;
    zs.zfree = Z_NULL;
    zs.opaque = Z_NULL;
    */

    ret = deflateInit2(&zs, m_z_level, Z_DEFLATED, 16 + MAX_WBITS, 8, Z_DEFAULT_STRATEGY);

    if (ret != Z_OK)
        return out_s;

    zs.next_in = (unsigned char*)s.data();
    zs.avail_in = s.size();

    //Compress contents
    do
    {

        zs.next_out = reinterpret_cast<unsigned char*>(out);
        zs.avail_out = sizeof(out);

        ret = deflate(&zs, Z_NO_FLUSH );

        //Write to output buffer
        if ( out_s.size() < zs.total_out )
        {
            out_s.append(out, zs.total_out - out_s.size() );
        }

    }
    while (ret == Z_OK);

    deflateEnd(&zs);

    return out_s;

}

//Decompress data
std::string Compressor::operator>>(const std::string & s)
{

    //Output string
    std::string out_s;

    //Return code
    int ret;

    //Output Buffer
    char out[Z_CHUNK];

    /* allocate inflate state */
    z_stream zs = {0};

    /*
    zs.zalloc = Z_NULL;
    zs.zfree = Z_NULL;
    zs.opaque = Z_NULL;
    zs.avail_in = 0;
    zs.next_in = Z_NULL;
    */

    ret = inflateInit2(&zs, 16 + MAX_WBITS);

    if (ret != Z_OK)
        return out_s;

    zs.next_in = (unsigned char*)s.data();
    zs.avail_in = s.size();

    //Decompress contents
    do
    {

        zs.next_out = reinterpret_cast<unsigned char*>(out);
        zs.avail_out = sizeof(out);

        ret = inflate(&zs, Z_NO_FLUSH );

        if ( out_s.size() < zs.total_out )
        {
            out_s.append( out, zs.total_out - out_s.size() );
        }

    }
    while (ret == Z_OK);

    inflateEnd(&zs);

    return out_s;

}

void Compressor::compress_file( const std::string & in, const std::string & out )
{

    std::ifstream infile( in, std::ifstream::in | std::ifstream::binary );
    if ( !infile.is_open() )
        return;

    std::ofstream outfile( out, std::ofstream::out );
    if ( !outfile.is_open() )
        return;

    unsigned int fs = boost::filesystem::file_size(in);

    while ( !infile.eof() && infile.good() )
    {

        char* buffer = new char[Z_CHUNK];

        infile.read(buffer, Z_CHUNK );

        //Compress the block of data and save it to out file
        std::string ds = *this << buffer;

        outfile << ds;

        unsigned int bytes_read = infile.tellg();

        std::cout << bytes_read << " / " << fs << " bytes compressed" << " (" << (((double)bytes_read / fs)*100) << "%)" << std::endl;

        delete[] buffer;

    }

    infile.close();
    outfile.close();

}
