#ifndef COMPRESS_H
#define COMPRESS_H

#include <iostream>
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

#define COMP_LEVEL 6
#define CHUNK 16384

namespace Backup {
    namespace Compression {

        class Compressor {

            public:
                Compressor();
                ~Compressor();

                std::string deflate_s(const std::string& s, int c = Z_DEFAULT_COMPRESSION );
                std::string inflate_s(const std::string& s);

            private:



        };

    }

}

#endif // COMPRESS_H
