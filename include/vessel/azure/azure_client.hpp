#ifndef AZURE_HPP
#define AZURE_HPP

#include <iostream>
#include <string>

#include <boost/date_time/date_facet.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <cryptopp/cryptlib.h>
#include <cryptopp/hmac.h>
#include <cryptopp/sha.h>

#include <vessel/types.hpp>
#include <vessel/network/http_client.hpp>
#include <vessel/vessel/vessel_client.hpp>
#include <vessel/network/http_request.hpp>
#include <vessel/network/http_stream.hpp>
#include <vessel/filesystem/file.hpp>
#include <vessel/crypto/hash_util.hpp>
#include <vessel/azure/azure_exception.hpp>

using namespace Vessel::Types;
using namespace Vessel::Exception;
using namespace Vessel::Database;
using namespace Vessel::File;
using namespace Vessel::Networking;
using namespace Vessel::Utilities;

namespace Vessel {
    namespace Networking {

        class AzureClient : public HttpClient
        {

            public:

                AzureClient(const StorageProvider& provider);
                ~AzureClient();

            private:
                bool m_remote_signing;
                std::string m_xms_date;
                std::string m_xms_version;
                StorageProvider m_storage_provider;

                /*! \fn std::string get_ms_date();
                    \brief Returns the current RFC 1123 formatted date/time
                    \return Returns the current RFC 1123 formatted date/time
                */
                std::string get_ms_date();

        };

    }
}

#endif

