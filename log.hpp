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
#include <boost/log/sources/severity_channel_logger.hpp>

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
        BOOST_LOG_ATTRIBUTE_KEYWORD(channel, "Channel", std::string)

        // The formatting logic for the severity level
        template< typename CharT, typename TraitsT >
        inline std::basic_ostream< CharT, TraitsT >& operator<< ( std::basic_ostream< CharT, TraitsT >& strm, severity_level lvl )
        {
            static const char* const str[] =
            {
                "INFO",
                "NOTIFICATION",
                "WARNING",
                "ERROR",
                "CRITICAL"
            };

            if (static_cast< std::size_t >(lvl) < (sizeof(str) / sizeof(*str)))
                strm << str[lvl];
            else
                strm << static_cast< int >(lvl);

            return strm;
        }

        class Log
        {

            public:
                Log(const std::string& filename);
                ~Log();

                void add_message( const std::string& msg, const std::string& category );

                void set_level ( unsigned int level );

            private:
                src::severity_channel_logger< Backup::Logging::severity_level, std::string > m_logger;
                std::string m_filename;
                unsigned int m_level;
        };

    }

}

#endif // LOG_H
