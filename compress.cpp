#include "compress.hpp"

using namespace Backup::Compression;

Compressor::Compressor()
{

}

Compressor::~Compressor()
{

}

std::string Compressor::compress(const std::string& s, int level)
{

    //Stream
    std::istringstream iss(s);
    std::ostringstream ss;

    int ret, flush;
    unsigned have;

    //IO Buffers
    unsigned char in[Z_CHUNK];
    unsigned char out[Z_CHUNK];

    /* allocate deflate state */

    m_strm.zalloc = Z_NULL;
    m_strm.zfree = Z_NULL;
    m_strm.opaque = Z_NULL;
    m_strm.avail_in = s.size();
    ret = deflateInit(&m_strm, level);
    if (ret != Z_OK)
        return ss.str();

    //Compress contents
    while ( !iss.eof() ) {

        iss.read((char*)in, Z_CHUNK);

        do {

            flush = iss.eof() ? Z_FINISH : Z_NO_FLUSH;
            m_strm.next_in = in;

            /* run deflate() on input until output buffer not full, finish compression if all of source has been read in */
            do {
                m_strm.avail_out = Z_CHUNK;
                m_strm.next_out = out;
                ret = deflate(&m_strm, flush);    /* no bad return value */
                assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
                have = Z_CHUNK - m_strm.avail_out;

                //Write to output buffer
                ss << (char*)out;

                if ( ss.tellp() != have || ss.bad() ) {
                    (void)deflateEnd(&m_strm);
                    return ss.str();
                }
            } while (m_strm.avail_out == 0);
            assert(m_strm.avail_in == 0);     /* all input will be used */

            /* done when last data in file processed */
        }
        while (flush != Z_FINISH);

        assert(ret == Z_STREAM_END);        /* stream will be complete */

    }

    /* clean up and return */
    (void)deflateEnd(&m_strm);
    return ss.str();

}

std::string Compressor::decompress( const std::string& s )
{

    //Stream
    std::istringstream iss(s);
    std::ostringstream ss;

    int ret;
    unsigned have;
    unsigned char in[Z_CHUNK];
    unsigned char out[Z_CHUNK];

    /* allocate inflate state */
    m_strm.zalloc = Z_NULL;
    m_strm.zfree = Z_NULL;
    m_strm.opaque = Z_NULL;
    m_strm.avail_in = s.size();
    m_strm.next_in = Z_NULL;
    ret = inflateInit(&m_strm);
    if (ret != Z_OK)
        return ss.str();

    //Decompress contents
    while ( !iss.eof() ) {

        iss.read( (char*)in, Z_CHUNK );

        do {

            if (iss.bad()) {
                (void)inflateEnd(&m_strm);
                return ss.str();
            }

            if (m_strm.avail_in == 0)
                break;

            m_strm.next_in = in;

            /* run inflate() on input until output buffer not full */
            do {
                m_strm.avail_out = Z_CHUNK;
                m_strm.next_out = out;
                ret = inflate(&m_strm, Z_NO_FLUSH);
                assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
                switch (ret) {
                    case Z_NEED_DICT:
                        ret = Z_DATA_ERROR;     /* and fall through */
                    case Z_DATA_ERROR:
                    case Z_MEM_ERROR:
                        (void)inflateEnd(&m_strm);
                        return ss.str();
                }
                have = Z_CHUNK - m_strm.avail_out;

                ss << (char*)out;

                if ( ss.tellp() != have || ss.bad() ) {
                    (void)inflateEnd(&m_strm);
                    return ss.str();
                }
            } while (m_strm.avail_out == 0);

            /* done when inflate() says it's done */
        }
        while (ret != Z_STREAM_END);

    }

    /* clean up and return */
    (void)inflateEnd(&m_strm);

    return ss.str();

    //return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;

}
