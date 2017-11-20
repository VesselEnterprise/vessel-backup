#include "log.hpp"

using namespace Backup::Logging;

Log::Log(const std::string& filename) : m_filename(filename)
{

    logging::add_file_log
    (
        keywords::file_name = m_filename,
        keywords::rotation_size = 10 * 1024 * 1024, //Rotate files every 10MB
        keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0), //Rotate at Midnight
        keywords::format = "[%TimeStamp%][%Category%]: %Message%",
        keywords::auto_flush = true
    );

    logging::add_console_log
    (
        std::cout,
        keywords::format = "[%TimeStamp%][%Category%]: %Message%",
        keywords::auto_flush = true
    );

    /*
    logging::core::get()->set_filter
    (
        logging::trivial::severity >= logging::trivial::info
    );
    */

    logging::add_common_attributes();

}

Log::~Log()
{

}

void Log::add_message(const std::string& msg, const std::string& cat )
{
    BOOST_LOG_SEV( m_logger, static_cast<Backup::Logging::severity_level>(m_level) ) << logging::add_value(category, cat) << msg;
}

void Log::set_level ( unsigned int level )
{
    m_level = level;
}
