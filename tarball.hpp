#ifndef TARBALL_H
#define TARBALL_H

#include <iostream>
#include <fstream>
#include <string>
#include <archive.h>
#include <archive_entry.h>
#include <boost/filesystem.hpp>
#include "compress.hpp"

namespace Backup {
    namespace Compression {

        class Tarball
        {
            public:

                Tarball(const std::string& filename);
                ~Tarball();

                void add_file(const std::string& filename);
                void save_tar();

            private:

                int m_total_files;
                struct archive * m_archive;
                struct stat m_stat;

                void create_tar(const std::string& filename);



        };



    }

}

#endif // TARBALL_H
