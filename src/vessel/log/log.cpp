#include <vessel/log/log.hpp>

using namespace Vessel::Logging;

Log::Log(const std::string& filename) : m_filename(filename), m_logger(keywords::channel = filename), m_level(error)
{

    logging::register_simple_formatter_factory< Vessel::Logging::severity_level, char >("Severity");

    logging::add_file_log
    (
        keywords::file_name = m_filename + "_%Y%m%d.log",
        //keywords::open_mode = std::ios::out | std::ios::app,
        keywords::rotation_size = 10 * 1024 * 1024, //Rotate files every 10MB
        keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0), //Rotate at Midnight
        keywords::format = "[%TimeStamp%][%Category%][%Severity%]: %Message%",
        keywords::auto_flush = true,
        keywords::filter = channel == m_filename
    );

    logging::add_console_log
    (
        std::cout,
        keywords::format = "[%TimeStamp%][%Category%][%Severity%]: %Message%",
        keywords::auto_flush = true
    );

    logging::core::get()->set_filter
    (
        Vessel::Logging::severity >= Vessel::Logging::info //Use error by default
    );

    logging::add_common_attributes();

}

Log::~Log()
{

}

void Log::add_message(const std::string& msg, const std::string& cat )
{
    BOOST_LOG_SEV( m_logger, static_cast<Vessel::Logging::severity_level>(m_level) ) << logging::add_value(category, cat) << msg;
}

void Log::set_level ( unsigned int level )
{
    m_level = level;
}
