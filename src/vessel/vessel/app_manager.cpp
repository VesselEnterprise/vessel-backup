#include <vessel/vessel/app_manager.hpp>

using namespace Vessel;

AppManager::AppManager() : m_runtime_errors(0)
{

}

void AppManager::prepare()
{

    //Set the executable path
    m_exe_path = get_exe_path();

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

    //If the DB hasn't been copied to the user dir, find and copy it
    if ( !boost::filesystem::exists(m_db_path) )
    {
        //Verify DB exists in user data directory
        bool db_copied=false;
        std::string db_file = DB_FILENAME;
        std::string last_copy_err;
        std::vector<std::string> dirs_to_check { "/var/lib/vessel/" + db_file, (m_exe_path + PATH_SEPARATOR() + db_file), db_file };

        for ( std::vector<std::string>::iterator itr = dirs_to_check.begin(); itr != dirs_to_check.end(); ++itr )
        {
            //Copy database template to data dir
            if ( boost::filesystem::exists(*itr) )
            {
                //DB File Exists in Working Dir
                try
                {
                    //First, try to copy the DB from exe path
                    boost::filesystem::copy(*itr, m_db_path);
                    Log::get_log().add_message("Copied Vessel database file to: " + m_db_path, "AppManager");
                    db_copied = true;
                    break;
                }
                catch(const boost::filesystem::filesystem_error& ex)
                {
                    Log::file_logging(true); //Force file logging here
                    last_copy_err = "Fatal error: Failed to copy database file to user data dir (" + std::string(ex.what()) + ")";
                    Log::get_log().add_error(last_copy_err, "AppManager");
                }
            }
        }

        if ( !db_copied ) //Fatal error
        {
            throw std::runtime_error(last_copy_err.c_str());
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

std::string AppManager::get_exe_path()
{
    return boost::dll::program_location().parent_path().string();
}

int AppManager::get_total_errors()
{
    return m_runtime_errors.load(std::memory_order_relaxed);
}
