#include "compress.hpp"

using namespace Backup::Compression;

Compressor::Compressor()
{
    std::cout << "Test123" << std::endl;
}

Compressor::~Compressor()
{
    std::cout << "Test123" << std::endl;
}

std::string Compressor::deflate_s(const std::string& s, int level)
{
    //Stream
    std::stringstream ss(s);

    //Compressed output
    std::istringstream zos;

    int ret, flush;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, level);
    if (ret != Z_OK)
        return zos.str();

    /* compress until end of file */
    do {

        strm.avail_in = ss.read(in, CHUNK);
        if (ss.fail()) {
            (void)deflateEnd(&strm);
            return zos.str();
        }

        flush = ss.eof() ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in;

        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, flush);    /* no bad return value */
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            have = CHUNK - strm.avail_out;

            //Write to stream
            zos >> out;

            if (ss.eof() || ss.fail() ) {
                (void)deflateEnd(&strm);
                return zos.str();
            }

        } while (strm.avail_out == 0);
        assert(strm.avail_in == 0);     /* all input will be used */

        /* done when last data in file processed */
    } while (flush != Z_FINISH);
    assert(ret == Z_STREAM_END);        /* stream will be complete */

    /* clean up and return */
    (void)deflateEnd(&strm);

    return zos.str();

}
