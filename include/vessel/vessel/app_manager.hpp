#ifndef APPMANAGER_H
#define APPMANAGER_H

#include <iostream>
#include <string>
#include <atomic>
#include <boost/filesystem.hpp>
#include <boost/dll.hpp>

#ifdef _WIN32
    #include <winsock2.h>
    #include <cstdlib>
    #include <userenv.h>
#else
    #include <unistd.h>
    #include <sys/types.h>
    #include <pwd.h>
#endif

#include <vessel/global.hpp>
#include <vessel/log/log.hpp>

#define APP_NAME "Vessel"
#define DB_FILENAME "vessel.db"
#define APP_VERSION "1.0.0"

#define MAX_RUNTIME_ERRORS 100

namespace Vessel
{
    class AppManager
    {

        public:
            /*! \fn static AppManager& get_app_manager()
                \brief Static singleton factory constructor which returns an instance to AppManager
                \return Singleton instance to AppManager
            */
            static AppManager& get()
            {
                static AppManager instance;
                return instance;
            }

            /**
             ** No Assignment or Copies allowed
            **/
            AppManager(AppManager const&) = delete;
            void operator=(AppManager const&) = delete;

            /*! \fn void prepare();
                \brief Prepares app for execution. Routine tasks such as creating app data folder, etc
            */
            void prepare();

            /*! \fn void increment_errror();
                \brief Increments the runtime error count
            */
            void increment_error();

            /*! \fn std::string get_data_dir();
                \return Returns the user data directory for the application ( /.vessel )
            */
            std::string get_data_dir();

            /*! \fn std::string get_user_dir();
                \return Returns the user home folder path
            */
            std::string get_user_dir();

            /*! \fn std::string get_db_path();
                \return Returns the path to the database
            */
            std::string get_db_path();

            /*! \fn std::string get_working_dir();
                \return Returns the current working path
            */
            std::string get_working_dir();

            /*! \fn std::string get_exe_path();
                \return Returns the current executable path
            */
            std::string get_exe_path();

            /*! \fn int get_total_errors();
                \return Returns the total runtime errors for the current app context
            */
            int get_total_errors();

        private:
            std::string m_user_dir;
            std::string m_data_dir;
            std::string m_db_path;
            std::string m_log_dir;
            std::string m_exe_path;

            AppManager(); //Private constructor for singleton model

            void setup_app_folder();

            std::atomic<int> m_runtime_errors; // Total runtime errors

    };
}

#endif // APPMANAGER_H
