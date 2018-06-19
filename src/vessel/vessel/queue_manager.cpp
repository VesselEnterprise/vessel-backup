#include <vessel/vessel/queue_manager.hpp>

QueueManager::QueueManager()
{
    m_database = &LocalDatabase::get_database();
}

QueueManager::~QueueManager()
{

}

void QueueManager::rebuild_queue()
{

    sqlite3_stmt* stmt;
    std::string query = "SELECT file_id FROM backup_file ORDER BY last_modified DESC LIMIT 100";

    if ( sqlite3_prepare_v2(m_database->get_handle(), query.c_str(), query.size(), &stmt, NULL ) != SQLITE_OK ) {
        throw DatabaseException(DatabaseException::InvalidStatement, "Error executing statement with query: " + query );
        return;
    }

    //Add files to the queue
    while ( sqlite3_step(stmt) == SQLITE_ROW )
    {
        unsigned char* file_id = (unsigned char*)sqlite3_column_blob(stmt, 0);
        push_file(file_id);

    }

    //Cleanup
    sqlite3_finalize(stmt);

}

void QueueManager::clear_queue()
{

    sqlite3_stmt* stmt;
    std::string query = "DELETE FROM backup_upload_queue";

    if ( sqlite3_prepare_v2(m_database->get_handle(), query.c_str(), query.size(), &stmt, NULL ) != SQLITE_OK ) {
        throw DatabaseException(DatabaseException::InvalidStatement, "Error executing statement with query: " + query );
        return;
    }

    //Execute query
    sqlite3_step(stmt);

    //Cleanup
    sqlite3_finalize(stmt);

}

void QueueManager::push_file(unsigned char* file_id)
{

    sqlite3_stmt* stmt;
    std::string query = "INSERT INTO backup_upload_queue (file_id) VALUES(?1)";

    if ( sqlite3_prepare_v2(m_database->get_handle(), query.c_str(), query.size(), &stmt, NULL ) != SQLITE_OK ) {
        throw DatabaseException(DatabaseException::InvalidStatement, "Error executing statement with query: " + query );
        return;
    }

    //Bind file id
    sqlite3_bind_blob(stmt, 0, file_id, sizeof(file_id), 0 );

    //Execute query
    if ( sqlite3_step(stmt) != SQLITE_DONE ) {
        throw DatabaseException(DatabaseException::InvalidQuery, "Error executing query: " + query);
        return;
    }

    //Cleanup
    sqlite3_finalize(stmt);

}

void QueueManager::pop_file(unsigned char* file_id)
{

    sqlite3_stmt* stmt;
    std::string query = "DELETE FROM backup_upload_queue WHERE file_id=?1";

    if ( sqlite3_prepare_v2(m_database->get_handle(), query.c_str(), query.size(), &stmt, NULL ) != SQLITE_OK ) {
        throw DatabaseException(DatabaseException::InvalidStatement, "Error executing statement with query: " + query );
        return;
    }

    //Bind file id
    sqlite3_bind_blob(stmt, 0, file_id, sizeof(file_id), 0 );

    //Execute query
    if ( sqlite3_step(stmt) != SQLITE_DONE ) {
        throw DatabaseException(DatabaseException::InvalidQuery, "Error executing query: " + query);
        return;
    }

    //Cleanup
    sqlite3_finalize(stmt);

}
