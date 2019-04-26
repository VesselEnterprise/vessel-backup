#ifndef GOOGLE_H
#define GOOGLE_H

#include <iostream>
#include <string>

#include <vessel/types.hpp>
#include <vessel/network/http_client.hpp>
#include <vessel/vessel/vessel_client.hpp>
#include <vessel/network/http_request.hpp>
#include <vessel/network/http_stream.hpp>
#include <vessel/filesystem/file.hpp>

using namespace Vessel::Types;
using namespace Vessel::Exception;
using namespace Vessel::Database;
using namespace Vessel::File;
using namespace Vessel::Networking;
using namespace Vessel::Utilities;

namespace Vessel {
    namespace Networking {

        class GcsClient : public HttpClient
        {

            public:
                GcsClient(const StorageProvider & provider);

                /*! \fn void init_upload(const BackupFile& file);
                    \brief Initializes the Gcs upload
                */
                void init_upload(const BackupFile& file);

        };

    }
}

#endif
