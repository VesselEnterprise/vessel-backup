#include <vessel/database/local_db.hpp>

using namespace Backup::Database;
using namespace Backup::File;

LocalDatabase::LocalDatabase()
{

    //Database logging
    m_log = new Backup::Logging::Log("db");

    m_err_code = this->open_db(DB_FILENAME);

    if ( m_err_code ) {
        m_log->add_message( "SQLite Error: " + get_last_err(), "Database" );
        this->close_db();
    }
    else
    {
        //Increase cache size
        sqlite3_exec(m_db, "PRAGMA cache_size = 100000", NULL, NULL, NULL);

        if ( VACUUM_ON_LOAD )
        {
            if ( vacuum_db() != SQLITE_OK ) {
                m_log->add_message( "SQLite Error: " + get_last_err(), "Database" );
            }
        }
    }

}

LocalDatabase::~LocalDatabase()
{
    this->close_db();
}

sqlite3* LocalDatabase::get_handle()
{
    return m_db;
}

int LocalDatabase::open_db(const std::string& filename)
{
    return sqlite3_open(filename.c_str(), &m_db );
}

int LocalDatabase::vacuum_db()
{
    return sqlite3_exec(m_db, "VACUUM", NULL, NULL, NULL );
}

void LocalDatabase::close_db()
{
    sqlite3_close(m_db);
}

std::string LocalDatabase::get_last_err()
{
    return sqlite3_errmsg(m_db);
}

std::string LocalDatabase::get_setting_str(const std::string & s )
{

    sqlite3_stmt* stmt;
    std::string query = "SELECT value FROM backup_setting WHERE name=?1";

    if ( sqlite3_prepare_v2(m_db, query.c_str(), query.size(), &stmt, NULL ) != SQLITE_OK )
        return "";

    sqlite3_bind_text(stmt, 1, s.c_str(), s.size(), 0 );

    std::string val="";

    if ( sqlite3_step(stmt) == SQLITE_ROW )
    {
        val = (char*)sqlite3_column_text(stmt,0);
    }

    //Cleanup
    sqlite3_finalize(stmt);

    return val;

}

int LocalDatabase::get_setting_int(const std::string & s )
{

    sqlite3_stmt* stmt;
    std::string query = "SELECT value FROM backup_setting WHERE name=?1";

    if ( sqlite3_prepare_v2(m_db, query.c_str(), query.size(), &stmt, NULL ) != SQLITE_OK )
        return -1;

    sqlite3_bind_text(stmt, 1, s.c_str(), s.size(), 0 );

    sqlite3_step(stmt);

    int val = sqlite3_column_int(stmt, 0);

    //Cleanup
    sqlite3_finalize(stmt);

    return val;

}

template <typename T>
bool LocalDatabase::update_setting(const std::string& key, const T& val )
{

    std::string val_s = boost::lexical_cast<std::string>(val);

    sqlite3_stmt* stmt;
    std::string query = "UPDATE backup_setting SET value = ?2 WHERE name=?1";

    if ( sqlite3_prepare_v2(m_db, query.c_str(), query.size(), &stmt, NULL ) != SQLITE_OK )
        return false;

    sqlite3_bind_text(stmt, 1, key.c_str(), key.size(), 0 );
    sqlite3_bind_text(stmt, 2, val_s.c_str(), val_s.size(), 0 );

    int rc = sqlite3_step(stmt);

    //Cleanup
    sqlite3_finalize(stmt);

    if ( rc != SQLITE_DONE )
        return false;

    return true;

}

template bool LocalDatabase::update_setting<std::string>(const std::string& key, const std::string& val );
template bool LocalDatabase::update_setting<int>(const std::string& key, const int& val );

bool LocalDatabase::update_ext_count(const std::string& ext, int total )
{

    sqlite3_stmt* stmt;
    std::string query = "INSERT OR REPLACE INTO backup_ext_count(`extension`,`total_count`) VALUES(?1,?2)";

    if ( sqlite3_prepare_v2(m_db, query.c_str(), query.size(), &stmt, NULL ) != SQLITE_OK )
        return false;

    sqlite3_bind_text(stmt, 1, ext.c_str(), ext.size(), 0 );
    sqlite3_bind_int(stmt, 2, total );

    int rc = sqlite3_step(stmt);

    //Cleanup
    sqlite3_finalize(stmt);

    if ( rc != SQLITE_DONE )
        return false;

    return true;

}

void LocalDatabase::clean()
{
    this->clean_dirs();
    this->clean_files();
}

void LocalDatabase::clean_dirs()
{
    //Cleanup Files from database

    sqlite3_stmt* stmt;
    std::string query = "SELECT directory_id,path FROM backup_directory";

    if ( sqlite3_prepare_v2(m_db, query.c_str(), query.size(), &stmt, NULL ) != SQLITE_OK )
        return;

    while ( sqlite3_step(stmt) == SQLITE_ROW )
    {

        int directory_id = sqlite3_column_int(stmt,0);
        std::string dir = (char*)sqlite3_column_text(stmt, 1);

        if ( !boost::filesystem::exists(dir) )
        {

            sqlite3_stmt* st;

            //Mark file as deleted
            std::string q = "UPDATE backup_directory SET deleted=1 WHERE directory_id=?1";

            if ( sqlite3_prepare_v2(m_db, q.c_str(), q.size(), &st, NULL ) != SQLITE_OK )
                return;

            sqlite3_bind_int(st, 1, directory_id );

            if ( sqlite3_step(st) != SQLITE_DONE )
                m_log->add_message("Failed to mark directory deleted: " + dir, "Database Cleaner");
            else
                m_log->add_message("File no longer exists: " + dir + " (Marked for deletion)", "Database Cleaner");

            //Cleanup
            sqlite3_finalize(st);

        }


    }

    //Cleanup
    sqlite3_finalize(stmt);
}

void LocalDatabase::clean_files()
{

    //Cleanup Files from database

    sqlite3_stmt* stmt;
    std::string query = "SELECT bd.path,bf.filename,bf.file_id FROM backup_file AS bf INNER JOIN backup_directory AS bd ON bf.directory_id = bd.directory_id WHERE bf.deleted=0";

    if ( sqlite3_prepare_v2(m_db, query.c_str(), query.size(), &stmt, NULL ) != SQLITE_OK )
        return;

    while ( sqlite3_step(stmt) == SQLITE_ROW )
    {

        std::string dir = (char*)sqlite3_column_text(stmt, 0);
        std::string filename = (char*)sqlite3_column_text(stmt, 1);
        //int file_id = sqlite3_column_int(stmt,2);
        unsigned char* file_id = (unsigned char*)sqlite3_column_blob(stmt,2);

        #ifdef _WIN32
            char cslash = '\\';
        #else
            char cslash = '/';
        #endif

        if ( !boost::filesystem::exists(dir + cslash + filename) )
        {

            sqlite3_stmt* st;

            //Mark file as deleted
            std::string q = "UPDATE backup_file SET deleted=1 WHERE file_id=?1";

            if ( sqlite3_prepare_v2(m_db, q.c_str(), q.size(), &st, NULL ) != SQLITE_OK )
                return;

            //sqlite3_bind_int(st, 1, file_id );
            sqlite3_bind_blob(st, 1, reinterpret_cast<const char*>(file_id), sizeof(file_id), 0);

            if ( sqlite3_step(st) != SQLITE_DONE )
                m_log->add_message("Failed to mark file deleted: " + dir + cslash + filename, "Database Cleaner");
            else
                m_log->add_message("File no longer exists: " + dir + cslash + filename + " (Marked for deletion)", "Database Cleaner");

            std::cout << get_last_err() << "\n";

            //Cleanup
            sqlite3_finalize(st);

        }

    }



    //Cleanup
    sqlite3_finalize(stmt);

}

void LocalDatabase::update_global_settings()
{

    //Update user home folder
    #ifdef _WIN32
        /* This sometimes returns incorrect results in Windows systems
        std::string home_drive = std::getenv("HOMEDRIVE");
        std::string home_path = std::getenv("HOMEPATH");
        */
        std::string user_folder = std::getenv("USERPROFILE");
        this->update_setting("home_folder", user_folder );
    #elif __unix
        this->update_setting("home_folder", std::getenv("HOME") );
    #endif

    //Update host name
    char hbuf[128];
    gethostname( hbuf,sizeof(hbuf) );

    std::string hostname(hbuf);

    this->update_setting("hostname", hostname );

    //Update username

    #ifdef _WIN32
        char username_buf[257];
        unsigned long userlen = sizeof(username_buf);
        GetUserName(username_buf, static_cast<unsigned long*>(&userlen) );
        std::string username(username_buf);
    #elif __unix
        struct passwd *pws;
        pws = getpwuid(geteuid());
        std::string username = pws->pw_name;
    #endif

    this->update_setting("username", username);

    //Update OS Version
    #ifdef _WIN32

        char domain_buf[257];
        unsigned long domainlen = sizeof(domain_buf);

        GetComputerNameEx( ComputerNameDnsDomain, domain_buf, static_cast<unsigned long*>(&domainlen) );

        std::string domain(domain_buf);
        std::string os;
        OSVERSIONINFOEXA os_bits;
        memset(&os_bits, 0, sizeof(OSVERSIONINFOEXA));
        os_bits.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXA);

        if ( GetVersionEx((POSVERSIONINFOA)&os_bits) != 0 )
        {
            std::cout << "Success" << std::endl;
            switch (os_bits.dwMajorVersion)
            {
                case 10:
                    if ( os_bits.wProductType == VER_NT_WORKSTATION )
                        os = "Windows 10";
                    else
                        os = "Windows Server 2016";
                    break;
                case 6:
                    switch ( os_bits.dwMinorVersion )
                    {
                        case 3:
                            if ( os_bits.wProductType == VER_NT_WORKSTATION)
                                os = "Windows 8.1";
                            else
                                os = "Windows Server 2012 RC2";
                            break;
                        case 2:
                            if ( os_bits.wProductType == VER_NT_WORKSTATION)
                                os = "Windows 8";
                            else
                                os = "Windows Server 2012";
                            break;
                        case 1:
                            if ( os_bits.wProductType == VER_NT_WORKSTATION)
                                os = "Windows 7";
                            else
                                os = "Windows 2008 R2";
                            break;
                        default:
                            os = "Windows NT";
                            break;
                    }
                    break;
                default:
                    os = "Windows NT";
                    break;
            }
        }

        /*
        std::cout << "Major version: " << os_bits.dwMajorVersion << std::endl;
        std::cout << "Minor version: " << os_bits.dwMinorVersion << std::endl;
        std::cout << "OS: " << os << std::endl;
        */

    #elif __unix
        utsname os_bits;
        uname(&os_bits);
        std::string domain = os_bits.domainname;
        std::string os = os_bits.sysname;
        os += " ";
        os += os_bits.machine;
        os += " ";
        os += os_bits.release;
    #endif

    this->update_setting("host_os", os);
    this->update_setting("host_domain", domain);

    //Update client application version
    this->update_setting("client_version", Backup::Version::FULLVERSION_STRING);

}

void LocalDatabase::update_client_settings(const std::string& s )
{
    using namespace rapidjson;

    Document doc;
    doc.Parse( s.c_str() );

    if ( !doc.IsObject() )
        return;

    const Value& settings = doc["response"]["settings"];

    if ( !settings.IsObject() )
        return;

    //Iterate and update settings
    for (Value::ConstMemberIterator itr = settings.MemberBegin(); itr != settings.MemberEnd(); ++itr )
    {
        const Value& obj = itr->value;

        if ( obj["value"].IsNull() )
            continue;

        //std::cout << v.name.GetString() << " => " << obj["value"].GetType() << std::endl;

        if ( obj["value"].IsString() )
            this->update_setting( itr->name.GetString(), obj["value"].GetString() );
        else if ( obj["value"].IsBool() ) {
            if ( obj["value"].GetBool() )
                this->update_setting( itr->name.GetString(), "true" );
            else
                this->update_setting( itr->name.GetString(), "false" );
        }
        else if ( obj["value"].IsNumber() )
            this->update_setting( itr->name.GetString(), obj["value"].GetInt() );

    }

}

void LocalDatabase::build_queue()
{

}

void LocalDatabase::start_transaction()
{
    sqlite3_exec(m_db, "BEGIN TRANSACTION", NULL, NULL, NULL);
}

void LocalDatabase::end_transaction()
{
    sqlite3_exec(m_db, "END TRANSACTION", NULL, NULL, NULL);
}