#ifndef VESSELLOG_H
#define VESSELLOG_H

#define BOOST_LOG_DYN_LINK 1
#define LOG_FILENAME "vessel"

#include <typeinfo>

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

namespace Vessel {
    namespace Logging {

        enum severity_level
        {
            info,
            notification,
            warning,
            error,
            critical
        };

        BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", Vessel::Logging::severity_level )
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
                static Log& get_log()
                {
                    static Log instance;
                    return instance;
                }

                Log(Log const&) = delete;
                void operator=(Log const&) = delete;

                void add_message( const std::string& msg, const std::string& type );
                void add_error( const std::string& msg, const std::string& type );
                void add_http_message( const std::string& request, const std::string& response, int status );
                void add_sql_message(const std::string& msg, const std::string& type, bool is_error=false);
                void set_level ( unsigned int level );
                void add_exception(const std::exception& ex);
                void set_file_logging(bool flag);
                void set_sql_logging(bool flag);
                void set_filename(const std::string& filename);

            protected:
                ~Log();

            private:
                Log();
                std::string m_filename;
                src::severity_channel_logger< Vessel::Logging::severity_level, std::string > m_logger;
                unsigned int m_level;
                bool m_file_logging;
                bool m_sql_logging;
                void add_file_exception(const std::exception& ex);
                void add_sql_exception(const std::exception& ex);
        };

    }

}

#include <vessel/database/local_db.hpp>

#endif // LOG_H
