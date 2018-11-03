#include <vessel/database/local_db.hpp>

using namespace Vessel::Database;

LocalDatabase::LocalDatabase()
{

    //Database logging
    m_log = &Log::get_log();

    //Verify database file exists
    if ( !boost::filesystem::exists(DB_FILENAME) )
    {
        Log::sql_logging(false);
        m_log->add_error( "SQLite Error: " + get_last_err(), "Database" );
        throw DatabaseException(DatabaseException::DatabaseNotFound, "Database file could not be opened");
    }

    m_err_code = this->open_db(DB_FILENAME);

    if ( m_err_code != SQLITE_OK ) {
        Log::sql_logging(false);
        m_log->add_error( "SQLite Error: " + get_last_err(), "Database" );
        this->close_db();
        throw DatabaseException(DatabaseException::DatabaseNotFound, "Database file could not be opened");
    }
    else
    {

        m_is_open=true;
        m_log->sql_logging(true);

        //Increase cache size
        sqlite3_exec(m_db, "PRAGMA cache_size = -10000", NULL, NULL, NULL);

        if ( VACUUM_ON_LOAD )
        {
            if ( vacuum_db() != SQLITE_OK ) {
                m_log->add_error( "SQLite Error: " + get_last_err(), "Database" );
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

    if ( sqlite3_prepare_v2(m_db, query.c_str(), -1, &stmt, NULL ) != SQLITE_OK )
    {
        m_log->add_error( "SQLite Error: " + get_last_err(), "Database" );
        return "";
    }

    sqlite3_bind_text(stmt, 1, s.c_str(), s.size(), 0 );

    std::string val = "";

    if ( sqlite3_step(stmt) == SQLITE_ROW )
    {
        val = get_sqlite_str( sqlite3_column_text(stmt,0) );
    }

    //Cleanup
    sqlite3_finalize(stmt);

    return val;

}

int LocalDatabase::get_setting_int(const std::string & s )
{

    sqlite3_stmt* stmt;
    std::string query = "SELECT value FROM backup_setting WHERE name=?1";

    if ( sqlite3_prepare_v2(m_db, query.c_str(), -1, &stmt, NULL ) != SQLITE_OK )
    {
        m_log->add_error( "SQLite Error: " + get_last_err(), "Database" );
        return -1;
    }

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

    if ( sqlite3_prepare_v2(m_db, query.c_str(), -1, &stmt, NULL ) != SQLITE_OK )
    {
        m_log->add_error( "SQLite Error: " + get_last_err(), "Database" );
        return false;
    }

    sqlite3_bind_text(stmt, 1, key.c_str(), key.size(), 0 );
    sqlite3_bind_text(stmt, 2, val_s.c_str(), val_s.size(), 0 );

    int rc = sqlite3_step(stmt);

    //Cleanup
    sqlite3_finalize(stmt);

    if ( rc != SQLITE_DONE )
    {
        m_log->add_error( "SQLite Error: " + get_last_err(), "Database" );
        return false;
    }

    return true;

}

template bool LocalDatabase::update_setting<std::string>(const std::string& key, const std::string& val );
template bool LocalDatabase::update_setting<int>(const std::string& key, const int& val );

bool LocalDatabase::update_ext_count(const std::string& ext, int total )
{

    sqlite3_stmt* stmt;
    std::string query = "INSERT OR REPLACE INTO backup_ext_count(`extension`,`total_count`) VALUES(?1,?2)";

    if ( sqlite3_prepare_v2(m_db, query.c_str(), -1, &stmt, NULL ) != SQLITE_OK )
    {
        m_log->add_error( "SQLite Error: " + get_last_err(), "Database" );
        return false;
    }

    sqlite3_bind_text(stmt, 1, ext.c_str(), ext.size(), 0 );
    sqlite3_bind_int(stmt, 2, total );

    int rc = sqlite3_step(stmt);

    //Cleanup
    sqlite3_finalize(stmt);

    if ( rc != SQLITE_DONE )
    {
        m_log->add_error( "SQLite Error: " + get_last_err(), "Database" );
        return false;
    }

    return true;

}

void LocalDatabase::clean()
{
    this->clean_dirs();
    this->clean_files();
}

void LocalDatabase::clean_dirs()
{
    //Cleanup Directories from database

    sqlite3_stmt* stmt;
    std::string query = "SELECT directory_id,path FROM backup_directory";

    if ( sqlite3_prepare_v2(m_db, query.c_str(), -1, &stmt, NULL ) != SQLITE_OK )
    {
        m_log->add_error( "SQLite Error: " + get_last_err(), "Database" );
        return;
    }

    while ( sqlite3_step(stmt) == SQLITE_ROW )
    {

        int directory_id = sqlite3_column_int(stmt,0);
        std::string dir = (char*)sqlite3_column_text(stmt, 1);

        if ( !boost::filesystem::exists(dir) )
        {

            sqlite3_stmt* st;

            //Mark file as deleted
            std::string q = "DELETE FROM backup_directory WHERE directory_id=?1";

            if ( sqlite3_prepare_v2(m_db, q.c_str(), -1, &st, NULL ) != SQLITE_OK ) {
                m_log->add_error("Failed to remove directory from database: " + dir, "Database Cleaner");
                continue;
            }

            sqlite3_bind_int(st, 1, directory_id );

            if ( sqlite3_step(st) != SQLITE_DONE ) {
                m_log->add_error("Failed to remove directory from database: " + dir, "Database Cleaner");
            }

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
    std::string query = "SELECT bd.path,bf.filename,bf.file_id FROM backup_file AS bf INNER JOIN backup_directory AS bd ON bf.directory_id = bd.directory_id";

    if ( sqlite3_prepare_v2(m_db, query.c_str(), -1, &stmt, NULL ) != SQLITE_OK ) {
        m_log->add_error("Failed to clean files", "Database Cleaner");
        return;
    }

    while ( sqlite3_step(stmt) == SQLITE_ROW )
    {

        std::string dir = (char*)sqlite3_column_text(stmt, 0);
        std::string filename = (char*)sqlite3_column_text(stmt, 1);
        unsigned char* file_id = (unsigned char*)sqlite3_column_blob(stmt,2);

        if ( !boost::filesystem::exists(dir + PATH_SEPARATOR() + filename) )
        {
            purge_file( file_id );
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
            //std::cout << "Success" << std::endl;
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
    this->update_setting("client_version", Vessel::Version::FULLVERSION_STRING);

}

void LocalDatabase::update_client_settings(const std::string& s )
{
    using namespace rapidjson;

    Document doc;
    doc.Parse( s.c_str() );

    if ( !doc.IsObject() )
    {
        m_log->add_error("Failed to update client settings", "Client");
        return;
    }

    const Value& settings = doc["response"]["settings"];

    if ( !settings.IsObject() )
    {
        m_log->add_error("Failed to update client settings", "Client");
        return;
    }

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

void LocalDatabase::start_transaction()
{
    sqlite3_exec(m_db, "BEGIN TRANSACTION", NULL, NULL, NULL);
}

void LocalDatabase::end_transaction()
{
    sqlite3_exec(m_db, "END TRANSACTION", NULL, NULL, NULL);
}

std::string LocalDatabase::get_sqlite_str(const void* data)
{
    return (data != NULL) ? (const char*)data : "";
}

void LocalDatabase::purge_file( unsigned char* file_id )
{

    std::vector<std::string> queries;
    queries.push_back("DELETE FROM backup_file WHERE file_id=?1");
    queries.push_back("DELETE FROM backup_upload WHERE file_id=?1");

    size_t file_id_size = sizeof(file_id);
    std::string file_id_s = (char*)file_id;

    for ( const auto &query : queries )
    {
        sqlite3_stmt* st;
        if ( sqlite3_prepare_v2(m_db, query.c_str(), -1, &st, NULL ) != SQLITE_OK ) {
            m_log->add_error("Failed to remove file from database: " + file_id_s, "Database Cleaner");
            continue;
        }

        sqlite3_bind_blob(st, 1, file_id, file_id_size, 0);

        if ( sqlite3_step(st) == SQLITE_DONE ) {
            m_log->add_message("File has been purged: " + file_id_s, "Database Cleaner");
        }
        else {
            m_log->add_error("File could not be purged: " + file_id_s, "Database Cleaner");
        }

        //Cleanup
        sqlite3_finalize(st);

    }

}

void LocalDatabase::purge_upload(unsigned int upload_id)
{

    std::vector<std::string> queries;
    queries.push_back("DELETE FROM backup_upload_part WHERE upload_id=?1");
    queries.push_back("DELETE FROM backup_upload WHERE upload_id=?1");

    for ( const auto &query : queries )
    {
        sqlite3_stmt* stmt;
        if ( sqlite3_prepare_v2(m_db, query.c_str(), -1, &stmt, NULL ) != SQLITE_OK ) {
            m_log->add_error("Failed to remove upload from database: " + std::to_string(upload_id), "Database Cleaner");
            continue;
        }

        sqlite3_bind_int(stmt, 1, upload_id );

        if ( sqlite3_step(stmt) != SQLITE_DONE ) {
            m_log->add_error("Failed to remove upload from database: " + std::to_string(upload_id), "Database Cleaner");
        }

        //Cleanup
        sqlite3_finalize(stmt);

    }

}

std::shared_ptr<unsigned char> LocalDatabase::get_binary_id(unsigned char* id)
{
    int bytes = sizeof(id);
    unsigned char* buffer = new unsigned char[bytes];
    memcpy(buffer, id, bytes );
    //std::copy ..?
    return std::shared_ptr<unsigned char>( buffer );
}

std::map<std::string,int> LocalDatabase::get_stats()
{

    std::map<std::string, int> stats;

    std::string query = "SELECT name,value FROM backup_stat";

    sqlite3_stmt* stmt;
    if ( sqlite3_prepare_v2(m_db, query.c_str(), -1, &stmt, NULL ) != SQLITE_OK ) {
        m_log->add_error("Failed to retrieve stats", "Stats");
        return stats;
    }

    while ( sqlite3_step(stmt) == SQLITE_ROW )
    {
        std::string name = get_sqlite_str( sqlite3_column_text(stmt, 0) );
        stats.insert( std::pair<std::string, int>(name, sqlite3_column_int(stmt, 1) ));
    }

    //Cleanup
    sqlite3_finalize(stmt);

    return stats;

}

bool LocalDatabase::is_open()
{
    return m_is_open;
}
