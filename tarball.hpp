#ifndef TARBALL_H
#define TARBALL_H

#include <iostream>
#include <string>
#include <archive.h>
#include <archive_entry.h>

namespace Backup {
    namespace Compression {

        class Tarball
        {
            public:

                Tarball(const std::string& filename);
                ~Tarball();

            private:

                int m_total_files;



        };



    }

}

#endif // TARBALL_H
