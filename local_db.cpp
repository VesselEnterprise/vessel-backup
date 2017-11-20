#include "local_db.hpp"

using namespace Backup::Database;

LocalDatabase::LocalDatabase(const std::string& filename)
{

    m_err_code = this->open_db(filename);

    if ( m_err_code )
        this->close_db();

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
    std::string query = "SELECT value FROM backup_setting WHERE name='" + s + "'";

    if ( sqlite3_prepare_v2(m_db, query.c_str(), query.size(), &stmt, NULL ) != SQLITE_OK )
        return NULL;

    sqlite3_step(stmt);

    std::string val = (char*)sqlite3_column_text(stmt, 0);

    //Cleanup
    sqlite3_finalize(stmt);

    return val;

}

int LocalDatabase::get_setting_int(const std::string & s )
{

    try {
        return boost::lexical_cast<int>(this->get_setting_str(s));
    }
    catch ( boost::bad_lexical_cast::exception & e ) {
        return -1;
    }

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

bool LocalDatabase::is_ignore_dir(const std::string& dir_name, int level )
{

    sqlite3_stmt* stmt;
    std::string query = "SELECT ignore_id FROM backup_ignore_dir WHERE name=?1 AND level=?2";

    if ( sqlite3_prepare_v2(m_db, query.c_str(), query.size(), &stmt, NULL ) != SQLITE_OK )
        return false;

    sqlite3_bind_text(stmt, 1, dir_name.c_str(), dir_name.size(), 0 );
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
    std::string query = "INSERT OR REPLACE INTO backup_file (file_id,filename,file_ext,filesize,directory_id,last_modified) VALUES((SELECT file_id FROM backup_file WHERE filename=?1 AND directory_id=?3),?1,?5,?2,?3,?4)";

    if ( sqlite3_prepare_v2(m_db, query.c_str(), query.size(), &stmt, NULL ) != SQLITE_OK )
        return false;

    sqlite3_bind_text(stmt, 1, fd->filename.c_str(), fd->filename.size(), 0 );
    sqlite3_bind_int(stmt, 2, fd->filesize );
    sqlite3_bind_int(stmt, 3, fd->directory_id );
    sqlite3_bind_int(stmt, 4, fd->last_modified );
    sqlite3_bind_text(stmt, 1, fd->file_ext.c_str(), fd->file_ext.size(), 0 );

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
