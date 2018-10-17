#ifndef VESSELDB_H
#define VESSELDB_H

#include <iostream>
#include <string>
#include <fstream>
#include <sqlite3.h>
#include <algorithm>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#include <vessel/global.hpp>
#include <vessel/database/db_exception.hpp>
#include <vessel/types.hpp>
#include <vessel/log/log.hpp>
#include <vessel/version.hpp>

#ifdef _WIN32
    #include <winsock2.h>
#elif __unix
    #include <unistd.h>
    #include <sys/types.h>
    #include <pwd.h>
    #include <sys/utsname.h>
#endif

#define VACUUM_ON_LOAD 1
#define DB_FILENAME "local.db"

using namespace Vessel::Types;
using namespace Vessel::Exception;
using namespace Vessel::Logging;

namespace Vessel{
    namespace Database {

        class LocalDatabase
        {

            public:

                /*! \constructor static LocalDatabase& get_database()
                    \brief Static singleton factory constructor which returns an instance to LocalDatabase
                    \return Singleton instance to LocalDatabase
                */
                static LocalDatabase& get_database()
                {
                    static LocalDatabase instance;
                    return instance;
                }

                /**
                 ** No Assignment or Copies allowed
                **/
                LocalDatabase(LocalDatabase const&) = delete;
                void operator=(LocalDatabase const&) = delete;

                /*! \fn sqlite3* get_handle();
                    \brief Returns a handle to the sqlite database
                    \return Returns a handle to the sqlite database
                */
                sqlite3* get_handle();

                /*! \fn std::string get_setting_str(const std::string& s);
                    \brief Returns string value of a setting from the Database
                    \param s Name of the setting to retrieve
                    \return Returns string value of a setting from the Database
                */
                std::string get_setting_str(const std::string& s);

                /*! \fn int get_setting_int(const std::string&s );
                    \brief Returns int value of a setting from the Database
                    \param s Name of the setting to retrieve
                    \return Returns int value of a setting from the Database
                */
                int get_setting_int(const std::string&s );

                /*! \fn bool update_setting(const std::string& key, const T& val );
                    \brief Updates a setting value in the database
                    \param key Name of the setting to update
                    \param val New value of the setting
                    \return Returns true if updated successfully
                */
                template <typename T>
                bool update_setting(const std::string& key, const T& val );

                /*! \fn static std::string get_sqlite_str(const void* data);
                    \brief Wrapper for sqlite3_column_text which prevents a segfault if NULL is returned from the database
                    \return Returns the string value from the database, or empty string if NULL
                */
                static std::string get_sqlite_str(const void* data);

                //TODO: Migrate to StatManager
                //Update file extension count
                bool update_ext_count( const std::string& ext, int total );

                /*! \fn std::string get_last_err();
                    \brief Returns the last error from the database (if any)
                    \return Returns the last error from the database (if any)
                */
                std::string get_last_err();

                /*! \fn void update_global_settings();
                    \brief Updates global environment settings
                */
                void update_global_settings();

                //TODO: Migrate to VesselClient
                /*! \fn void update_client_settings( const std::string& s );
                    \brief Parses client settings JSON from Vessel REST API and saves to database
                */
                void update_client_settings( const std::string& s );

                /*! \fn void clean();
                    \brief Scans the directory and file table for non-existent files/directories and removes them from the database
                */
                void clean();

                /*! \fn void start_transaction();
                    \brief Starts a database transaction
                */
                void start_transaction();

                /*! \fn void end_transaction();
                    \brief Ends a database transaction
                */
                void end_transaction();

                /*! \fn void purge_file( unsigned char* file_id );
                    \param file_id Binary File ID
                    \brief Removes a file from the database
                */
                void purge_file( unsigned char* file_id );

                /*! \fn void purge_upload(unsigned int upload_id);
                    \param upload_id Database Upload Id
                    \brief Removes an upload from the database
                */
                void purge_upload(unsigned int upload_id);

                /*! \fn static std::shared_ptr<unsigned char> get_binary_id(unsigned char* id);
                    \param id Some binary/raw id
                    \brief Makes a copy of a binary id and returns a shared_ptr of the copy
                */
                static std::shared_ptr<unsigned char> get_binary_id(unsigned char* id);

            private:
                sqlite3* m_db;
                int m_err_code;
                Log* m_log;

                /**
                 ** Constructor must be private for singleton factory model
                **/
                LocalDatabase();

                /*! \fn int vacuum_db();
                    \brief Performs a SQL VACUUM on the database
                */
                int vacuum_db();

                /*! \fn int open_db(const std::string& filename);
                    \brief Opens the sqlite database
                */
                int open_db(const std::string& filename);

                /*! \fn void close_db();
                    \brief Closes the sqlite database
                */
                void close_db();

                /*! \fn void clean_dirs();
                    \brief Removes non-existent directories from the database
                */
                void clean_dirs();

                /*! \fn void clean_files();
                    \brief Removes non-existent files from the database
                */
                void clean_files();

            protected:
                ~LocalDatabase();
        };

    }
}

#endif // VESSELDB_H
