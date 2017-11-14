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
    sqlite3_close(m_db);
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
        return NULL;
    }

}

void LocalDatabase::update_ext_count(const std::string& ext, int total )
{

    sqlite3_stmt* stmt;
    std::string query = "INSERT OR REPLACE INTO backup_count_ext(`extension`,`total_count`) VALUES('?',?)";

    if ( sqlite3_prepare_v2(m_db, query.c_str(), query.size(), &stmt, NULL ) != SQLITE_OK )
        return NULL;

    sqlite3_bind_text(stmt, )

    sqlite3_step(stmt);

    std::string val = (char*)sqlite3_column_text(stmt, 0);

    //Cleanup
    sqlite3_finalize(stmt);

    return val;

}
