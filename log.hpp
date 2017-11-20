#ifndef LOG_H
#define LOG_H

#define BOOST_LOG_DYN_LINK 1

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;

namespace Backup {
    namespace Logging {

        enum severity_level
        {
            info,
            notification,
            warning,
            error,
            critical
        };

        BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", Backup::Logging::severity_level )
        BOOST_LOG_ATTRIBUTE_KEYWORD(category, "Category", std::string)

        class Log
        {

            public:
                Log(const std::string& filename);
                ~Log();

                void add_message( const std::string& msg, const std::string& category );

                void set_level ( unsigned int level );

            private:
                src::severity_logger< Backup::Logging::severity_level > m_logger;
                std::string m_filename;
                unsigned int m_level;
        };

    }

}

#endif // LOG_H
