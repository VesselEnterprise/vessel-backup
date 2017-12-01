#include "local_db.hpp"

using namespace Backup::Database;

LocalDatabase::LocalDatabase(const std::string& filename)
{

    m_err_code = this->open_db(filename);

    if ( m_err_code )
        this->close_db();

    m_log = new Backup::Logging::Log("db");

}

LocalDatabase::~LocalDatabase()
{
    this->close_db();
}

int LocalDatabase::open_db(const std::string& filename)
{
    return sqlite3_open(filename.c_str(), &m_db );
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

bool LocalDatabase::is_ignore_dir(const boost::filesystem::path& p, int level )
{

    std::string folder_name = p.filename().string();

    sqlite3_stmt* stmt;
    std::string query = "SELECT ignore_id FROM backup_ignore_dir WHERE name=?1 AND level=?2";

    if ( sqlite3_prepare_v2(m_db, query.c_str(), query.size(), &stmt, NULL ) != SQLITE_OK )
        return false;

    sqlite3_bind_text(stmt, 1, folder_name.c_str(), folder_name.size(), 0 );
    sqlite3_bind_int(stmt, 2, level );

    int rc = sqlite3_step(stmt);

    //Cleanup
    sqlite3_finalize(stmt);

    if ( rc != SQLITE_ROW )
        return false;

    return true;

}

bool LocalDatabase::is_ignore_ext(const std::string& ext )
{

    sqlite3_stmt* stmt;
    std::string query = "SELECT ignore_id FROM backup_ignore_ext WHERE extension=?1";

    if ( sqlite3_prepare_v2(m_db, query.c_str(), query.size(), &stmt, NULL ) != SQLITE_OK )
        return false;

    sqlite3_bind_text(stmt, 1, ext.c_str(), ext.size(), 0 );

    int rc = sqlite3_step(stmt);

    //Cleanup
    sqlite3_finalize(stmt);

    if ( rc != SQLITE_ROW )
        return false;

    return true;

}

unsigned int LocalDatabase::add_file( Backup::Types::file_data* fd )
{

    sqlite3_stmt* stmt;
    std::string query = "INSERT OR REPLACE INTO backup_file (file_id,filename,file_ext,filesize,directory_id,last_modified,deleted) VALUES((SELECT file_id FROM backup_file WHERE filename=?1 AND directory_id=?3),?1,?5,?2,?3,?4,0)";

    if ( sqlite3_prepare_v2(m_db, query.c_str(), query.size(), &stmt, NULL ) != SQLITE_OK )
        return false;

    sqlite3_bind_text(stmt, 1, fd->filename.c_str(), fd->filename.size(), 0 );
    sqlite3_bind_int(stmt, 2, fd->filesize );
    sqlite3_bind_int(stmt, 3, fd->directory_id );
    sqlite3_bind_int(stmt, 4, fd->last_modified );
    sqlite3_bind_text(stmt, 5, fd->file_ext.c_str(), fd->file_ext.size(), 0 );

    int rc = sqlite3_step(stmt);

    //Cleanup
    sqlite3_finalize(stmt);

    fd->file_id = sqlite3_last_insert_rowid(m_db);

    return fd->file_id;

}

unsigned int LocalDatabase::add_directory( Backup::Types::file_directory* fd )
{

    sqlite3_stmt* stmt;
    std::string query = "INSERT OR REPLACE INTO backup_directory (directory_id,path,last_modified) VALUES((SELECT directory_id FROM backup_directory WHERE path=?1),?1,?2)";

    if ( sqlite3_prepare_v2(m_db, query.c_str(), query.size(), &stmt, NULL ) != SQLITE_OK )
        return false;

    sqlite3_bind_text(stmt, 1, fd->path.c_str(), fd->path.size(), 0 );
    sqlite3_bind_int(stmt, 2, fd->last_modified );

    int rc = sqlite3_step(stmt);

    //Cleanup
    sqlite3_finalize(stmt);

    fd->directory_id = sqlite3_last_insert_rowid(m_db);

    return fd->directory_id;

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
    std::string query = "SELECT bd.path,bf.filename,bf.file_id FROM backup_file AS bf LEFT JOIN backup_directory AS bd ON bf.directory_id = bd.directory_id WHERE bf.deleted=0";

    if ( sqlite3_prepare_v2(m_db, query.c_str(), query.size(), &stmt, NULL ) != SQLITE_OK )
        return;

    while ( sqlite3_step(stmt) == SQLITE_ROW )
    {

        std::string dir = (char*)sqlite3_column_text(stmt, 0);
        std::string filename = (char*)sqlite3_column_text(stmt, 1);
        int file_id = sqlite3_column_int(stmt,2);

        if ( !boost::filesystem::exists(dir + "\\" + filename) )
        {

            sqlite3_stmt* st;

            //Mark file as deleted
            std::string q = "UPDATE backup_file SET deleted=1 WHERE file_id=?1";

            if ( sqlite3_prepare_v2(m_db, q.c_str(), q.size(), &st, NULL ) != SQLITE_OK )
                return;

            sqlite3_bind_int(st, 1, file_id );

            if ( sqlite3_step(st) != SQLITE_DONE )
                m_log->add_message("Failed to mark file deleted: " + dir + "\\" + filename, "Database Cleaner");
            else
                m_log->add_message("File no longer exists: " + dir + "\\" + filename + " (Marked for deletion)", "Database Cleaner");

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
        std::string home_drive = std::getenv("HOMEDRIVE");
        std::string home_path = std::getenv("HOMEPATH");
        this->update_setting("home_folder", home_drive.append(home_path) );
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

}
