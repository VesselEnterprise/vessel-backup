#ifndef COMPRESS_H
#define COMPRESS_H

#include <iostream>
#include <assert.h>
#include <fstream>
#include <sstream>
#include <string.h>
#include <zlib.h>

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

#define Z_COMP_LEVEL 6
#define Z_CHUNK 16384

namespace Backup {
    namespace Compression {

        class Compressor {

            public:
                Compressor();
                ~Compressor();

                std::operator<<()

                std::string compress(const std::string& s, int z_level = Z_COMP_LEVEL ); //Z_DEFAULT_COMPRESSION
                std::string decompress(const std::string& s);

            private:
                z_stream m_strm;



        };

    }

}

#endif // COMPRESS_H
