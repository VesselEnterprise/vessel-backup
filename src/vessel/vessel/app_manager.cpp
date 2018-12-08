#include <vessel/vessel/app_manager.hpp>

using namespace Vessel;

AppManager::AppManager() : m_runtime_errors(0)
{

}

void AppManager::prepare()
{
    //Determine user folder and appdata dirs
    #ifdef _WIN32
        m_user_dir = std::getenv("USERPROFILE");
        if ( m_user_dir.empty() || !boost::filesystem::exists(m_user_dir) )
        {
            long unsigned int buflen = 512;
            char buffer[buflen];

            HANDLE hToken;

            if( !OpenProcessToken( GetCurrentProcess(), TOKEN_READ, &hToken ) )
            {
                throw std::runtime_error("Unable to determine user home directory");
            }

            if ( !GetUserProfileDirectory(hToken, buffer, &buflen) )
            {
                throw std::runtime_error("Unable to determine user home directory");
            }

            m_user_dir = buffer;
            CloseHandle(hToken);
        }
    #else
        m_user_dir = std::getenv("HOME");
        if ( m_user_dir.empty() || !boost::filesystem::exists(m_user_dir) )
        {
            struct passwd *pw = getpwuid(getuid());
            m_user_dir = pw->pw_dir;
        }
    #endif

    m_data_dir = m_user_dir + PATH_SEPARATOR() + ".vessel";

    //Create data directory if it does not already exist
    if ( !boost::filesystem::exists(m_data_dir) )
    {
        if ( boost::filesystem::create_directory(m_data_dir) )
        {
            Log::get_log().add_message("Created .vessel directory in: " + m_data_dir, "AppManager");
        }
        else
        {
            Log::get_log().add_error("Unable to create .vessel directory in: " + m_data_dir, "AppManager");
            throw std::runtime_error("Unable to create .vessel directory in: " + m_data_dir);
        }
    }

    //Create log directory if it does not exist
    m_log_dir = m_data_dir + PATH_SEPARATOR() + "logs";

    if ( !boost::filesystem::is_directory(m_log_dir) )
    {
        if ( boost::filesystem::create_directory(m_log_dir) )
        {
            Log::get_log().add_message("Created /logs directory in: " + m_log_dir, "AppManager");
        }
    }

    m_db_path = m_data_dir + PATH_SEPARATOR() + DB_FILENAME;

    //Verify DB exists in user data directory
    if ( !boost::filesystem::exists(m_db_path) )
    {
        //Copy database template from working dir to data dir
        try
        {
            boost::filesystem::copy(DB_FILENAME, m_db_path);
        }
        catch(const boost::filesystem::filesystem_error& ex)
        {
            Log::file_logging(true); //Force file logging here
            std::string errmsg = "Fatal error: Failed to copy database file to user data dir (" + std::string(ex.what()) + ")";
            Log::get_log().add_error(errmsg, "AppManager");
            throw std::runtime_error(errmsg);
        }

    }

}

void AppManager::increment_error()
{
    m_runtime_errors.fetch_add(1, std::memory_order_relaxed);
}

std::string AppManager::get_working_dir()
{
    return boost::filesystem::current_path().string();
}

std::string AppManager::get_user_dir()
{
    return m_user_dir;
}

std::string AppManager::get_data_dir()
{
    return m_data_dir;
}

std::string AppManager::get_db_path()
{
    return m_db_path;
}

int AppManager::get_total_errors()
{
    return m_runtime_errors.load(std::memory_order_relaxed);
}
