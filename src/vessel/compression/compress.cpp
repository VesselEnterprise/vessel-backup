#include <vessel/compression/compress.hpp>

using namespace Vessel::Compression;

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

std::string Compressor::compress_str(const std::string& str)
{
    return (*this << str);
}

std::string Compressor::decompress_str(const std::string& str)
{
    return (*this >> str);
}

//Compress data
std::string Compressor::operator<<(const std::string& str)
{

    //Buffer
    std::string out_s;

    //Return code
    int ret;

    //Output Buffer
    unsigned char out[Z_CHUNK];

    /* allocate deflate state */
    z_stream zs = {0};
    /*
    zs.zalloc = Z_NULL;
    zs.zfree = Z_NULL;
    zs.opaque = Z_NULL;
    */

    ret = deflateInit2(&zs, m_z_level, Z_DEFLATED, MAX_WBITS | GZIP_ENCODING, 8, Z_DEFAULT_STRATEGY);

    if (ret != Z_OK)
        return out_s;

    zs.next_in = (unsigned char*)str.c_str();
    zs.avail_in = str.size();

    //Compress contents
    do
    {

        zs.next_out = out;
        zs.avail_out = sizeof(out);

        ret = deflate(&zs, Z_FINISH );

        assert(ret != Z_STREAM_ERROR);

        //Write to output buffer
        out_s.append((char*)out, (Z_CHUNK - zs.avail_out) );

    }
    while (zs.avail_out == 0);

    assert(zs.avail_in == 0); //All input should be used

    deflateEnd(&zs);

    return out_s;

}

int Compressor::start_decompression(size_t len)
{
    /* allocate inflate state */
    m_zs_decomp = {0};

    /*
    zs.zalloc = Z_NULL;
    zs.zfree = Z_NULL;
    zs.opaque = Z_NULL;
    */

    m_zs_decomp.avail_in = len;

    return inflateInit2(&m_zs_decomp, MAX_WBITS | GZIP_ENCODING );

}

void Compressor::end_decompression()
{
    inflateEnd(&m_zs_decomp); //Cleanup
}

//Decompress data
std::string Compressor::operator>>(const std::string & str)
{

    //Output string
    std::string out_s;

    //Return code
    int ret;

    //Output Buffer
    unsigned char out[Z_CHUNK];

    m_zs_decomp.next_in = (unsigned char*)str.c_str();

    //Decompress contents
    do
    {

        m_zs_decomp.next_out = out;
        m_zs_decomp.avail_out = sizeof(out);

        ret = inflate(&m_zs_decomp, Z_NO_FLUSH );

        assert(ret != Z_STREAM_ERROR);

        //std::cout << "AVAILOUT: " << m_zs_decomp.avail_out << std::endl;

        out_s.append( (char*)out, (Z_CHUNK - m_zs_decomp.avail_out) );

    }
    while (m_zs_decomp.avail_out == 0);

    return out_s;

}

void Compressor::compress_file( const std::string & in, const std::string & out )
{

    std::ifstream infile( in, std::ifstream::in | std::ifstream::binary );
    if ( !infile.is_open() )
        return;

    std::ofstream outfile( out, std::ofstream::out | std::ofstream::binary );
    if ( !outfile.is_open() )
        return;

    size_t fs = boost::filesystem::file_size(in);
    size_t bytes_read=0;

    while ( !infile.eof() && infile.good() )
    {

        char* buffer = new char[Z_CHUNK];

        infile.read(buffer, Z_CHUNK );

        //Save buffer to string
        std::string ts(buffer, infile.gcount() );

        //Compress the block of data and save it to out file
        std::string ds = *this << ts;

        //Write to outfile
        outfile << ds;

        bytes_read += infile.gcount();

        std::cout << bytes_read << " / " << fs << " bytes compressed" << " (" << (((double)bytes_read / fs)*100) << "%)" << std::endl;

        delete[] buffer;

    }

    infile.close();
    outfile.close();

}

void Compressor::decompress_file( const std::string & in, const std::string & out )
{

    std::ifstream infile( in, std::ifstream::in | std::ifstream::binary );
    if ( !infile.is_open() )
        return;

    std::ofstream outfile( out, std::ofstream::out | std::ofstream::binary );
    if ( !outfile.is_open() )
        return;

    size_t fs = boost::filesystem::file_size(in);
    size_t bytes_read=0;

    this->start_decompression(fs);

    while ( !infile.eof() && infile.good() )
    {

        char* buffer = new char[Z_CHUNK];

        infile.read(buffer, Z_CHUNK );

        //Save buffer to string
        std::string ts(buffer, infile.gcount() );

        //Decompress the block of data and save it to out file
        std::string ds = *this >> ts;

        //Write to outfile
        outfile << ds;

        bytes_read += infile.gcount();

        std::cout << bytes_read << " / " << fs << " bytes decompressed" << " (" << (((double)bytes_read / fs)*100) << "%)" << std::endl;

        delete[] buffer;

    }

    this->end_decompression();

    infile.close();
    outfile.close();

}
