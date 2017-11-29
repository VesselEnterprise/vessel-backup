#ifndef COMPRESS_H
#define COMPRESS_H

#include <iostream>
#include <assert.h>
#include <fstream>
#include <sstream>
#include <string.h>
#include <math.h>
#include <iomanip>
#include <zlib.h>
#include <boost/filesystem.hpp>

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

#define Z_COMP_LEVEL 6
#define Z_CHUNK 32768
#define GZIP_ENCODING 16

namespace Backup {
    namespace Compression {

        class Compressor {

            public:

                Compressor();
                ~Compressor();

                //Compression operator
                std::string operator << ( const std::string& s );

                //Decompression operator
                std::string operator >> ( const std::string& s );

                //Compress file and save to temp file
                void compress_file( const std::string & in, const std::string & out );

                //Decompress file and save to temp file
                void decompress_file( const std::string & in, const std::string & out );

                int start_decompression(size_t len);
                void end_decompression();

                //Set compression level
                void set_z_level ( int level );

            private:

                z_stream m_zs_decomp;
                int m_z_level; //Compression level

        };

    }

}

#endif // COMPRESS_H
