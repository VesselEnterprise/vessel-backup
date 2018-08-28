#include <vessel/log/log.hpp>

using namespace Vessel::Logging;
using namespace Vessel::Database;

Log::Log() : m_logger(keywords::channel = LOG_FILENAME), m_level(error), m_sql_logging(true)
{

    m_filename = LOG_FILENAME;

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

void Log::set_file_logging(bool flag)
{
    m_file_logging = flag;
}

void Log::set_sql_logging(bool flag)
{
    m_sql_logging = flag;
}

void Log::add_message(const std::string& msg, const std::string& cat )
{
    BOOST_LOG_SEV( m_logger, static_cast<Vessel::Logging::severity_level>(m_level) ) << logging::add_value(category, cat) << msg;
    if ( m_sql_logging ) add_sql_message(msg);
}

void Log::add_sql_message(const std::string& msg, bool is_error)
{

    sqlite3_stmt* stmt;

    std::string query = "INSERT INTO backup_log (message,error) VALUES(?1,?2)";

    if ( sqlite3_prepare(LocalDatabase::get_database().get_handle(), query.c_str(), query.size(), &stmt, NULL) != SQLITE_OK ) {
        return;
    }

    sqlite3_bind_text(stmt, 1, msg.c_str(), msg.size(), 0);
    sqlite3_bind_int(stmt, 2, (int)is_error );

    sqlite3_step(stmt);

    sqlite3_finalize(stmt);
}

void Log::set_level ( unsigned int level )
{
    m_level = level;
}

void Log::add_exception(const std::exception& ex)
{
    if ( m_file_logging ) add_file_exception(ex);
    if ( m_sql_logging ) add_sql_exception(ex);
}

void Log::add_file_exception(const std::exception& ex)
{
    BOOST_LOG_SEV( m_logger, static_cast<Vessel::Logging::severity_level>(m_level) ) << logging::add_value(category, "") << ex.what();
}

void Log::add_sql_exception(const std::exception& ex)
{

    std::string message = ex.what();

    sqlite3_stmt* stmt;

    std::string query = "INSERT INTO backup_log (message,exception,error) VALUES(?1, '', 1)";

    if ( sqlite3_prepare(LocalDatabase::get_database().get_handle(), query.c_str(), query.size(), &stmt, NULL) != SQLITE_OK ) {
        return;
    }

    sqlite3_bind_text(stmt, 1, message.c_str(), message.size(), 0);

    sqlite3_step(stmt);

    sqlite3_finalize(stmt);

}

void Log::set_filename(const std::string& filename)
{
    m_filename = filename;
    /*
    m_logger
    {
        keywords::channel = filename
    }
    */
}
