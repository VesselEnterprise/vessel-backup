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

    //Clear the current queue
    clear_queue();

    sqlite3_stmt* stmt;
    std::string query = "SELECT a.file_id FROM backup_file AS a INNER JOIN backup_weight_ext AS b ON a.file_ext=b.file_ext WHERE a.last_modified >= a.last_backup_time ORDER BY a.last_modified DESC LIMIT ?1";

    if ( sqlite3_prepare_v2(m_database->get_handle(), query.c_str(), query.size(), &stmt, NULL ) != SQLITE_OK ) {
        throw DatabaseException(DatabaseException::InvalidStatement, "Error executing statement with query: " + query );
        return;
    }

    sqlite3_bind_int(stmt, 1, QUEUE_MAX_ROWS);

    int total_rows = 0;

    //Add files to the queue
    while ( sqlite3_step(stmt) == SQLITE_ROW )
    {
        unsigned char* file_id = (unsigned char*)sqlite3_column_blob(stmt, 0);
        push_file(file_id);
        total_rows++;

    }

    //Reset the statement
    sqlite3_reset(stmt);

    int remainder = QUEUE_MAX_ROWS - total_rows;

    //If there were less than QUEUE_MAX_ROWS total rows after using file weights, fill up the rest of the queue

    if ( remainder > 0 )
    {

        query = "SELECT a.file_id FROM backup_file AS a LEFT JOIN backup_weight_ext AS b ON a.file_ext = b.file_ext WHERE b.file_ext IS NULL ORDER BY a.last_modified DESC LIMIT ?1";

        if ( sqlite3_prepare_v2(m_database->get_handle(), query.c_str(), query.size(), &stmt, NULL ) != SQLITE_OK ) {
            throw DatabaseException(DatabaseException::InvalidStatement, "Error executing statement with query: " + query );
            return;
        }

        sqlite3_bind_int(stmt, 1, remainder);

        //Add files to the queue
        while ( sqlite3_step(stmt) == SQLITE_ROW )
        {
            unsigned char* file_id = (unsigned char*)sqlite3_column_blob(stmt, 0);
            push_file(file_id);
            total_rows++;

        }

    }

     //Cleanup
    sqlite3_finalize(stmt);

    //Apply weights
    apply_weights();

}

void QueueManager::clear_queue()
{

    sqlite3_stmt* stmt;
    std::string query = "DELETE FROM backup_upload";

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
    std::string query = "INSERT INTO backup_upload (file_id) VALUES(?1)";

    if ( sqlite3_prepare_v2(m_database->get_handle(), query.c_str(), query.size(), &stmt, NULL ) != SQLITE_OK ) {
        throw DatabaseException(DatabaseException::InvalidStatement, "Error executing statement with query: " + query );
        return;
    }

    //Bind file id
    sqlite3_bind_blob(stmt, 1, file_id, sizeof(file_id), 0 );

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
    std::string query = "DELETE FROM backup_upload WHERE file_id=?1";

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

void QueueManager::apply_weights()
{

    sqlite3_stmt* stmt;
    std::string query = "UPDATE backup_upload SET weight = weight + (SELECT weight FROM backup_weight_ext AS b INNER JOIN backup_file AS c ON file_id = c.file_id WHERE b.file_ext = c.file_ext)";

    if ( sqlite3_prepare_v2(m_database->get_handle(), query.c_str(), query.size(), &stmt, NULL ) != SQLITE_OK ) {
        throw DatabaseException(DatabaseException::InvalidStatement, "Error executing statement with query: " + query );
        return;
    }

    //Execute query
    if ( sqlite3_step(stmt) != SQLITE_DONE ) {
        throw DatabaseException(DatabaseException::InvalidQuery, "Error executing query: " + query);
        return;
    }

    //Cleanup
    sqlite3_finalize(stmt);

}

int QueueManager::get_total_pending()
{

    sqlite3_stmt* stmt;

    std::string query = "SELECT COUNT(*) FROM backup_upload";

    if ( sqlite3_prepare_v2(m_database->get_handle(), query.c_str(), query.size(), &stmt, NULL ) != SQLITE_OK ) {
        throw DatabaseException(DatabaseException::InvalidStatement, "Error executing statement with query: " + query );
        return -1;
    }

    int total_pending = 0;

    if ( sqlite3_step(stmt) == SQLITE_ROW )
    {
        total_pending = (int)sqlite3_column_int(stmt, 0);
    }

    return total_pending;

}

BackupFile QueueManager::get_next_file()
{

    //If there are no pending uploads, rebuild the queue
    if ( get_total_pending() == 0 )
    {
        rebuild_queue();
    }

    sqlite3_stmt* stmt;

    std::string query = "SELECT a.filename,c.path FROM backup_file AS a INNER JOIN backup_upload AS b ON a.file_id = b.file_id INNER JOIN backup_directory AS c ON a.directory_id = c.directory_id ORDER BY a.last_modified,b.weight DESC LIMIT 1";

    if ( sqlite3_prepare_v2(m_database->get_handle(), query.c_str(), query.size(), &stmt, NULL ) != SQLITE_OK ) {
        throw DatabaseException(DatabaseException::InvalidStatement, "Error executing statement with query: " + query );
    }

    if ( sqlite3_step(stmt) != SQLITE_ROW )
    {
        throw FileException(FileException::FileNotFound, "No files were found in the upload queue");
    }

    std::string base_path = (char*)sqlite3_column_text(stmt, 1);
    std::string file_name = (char*)sqlite3_column_text(stmt, 0);

    BackupFile bf((base_path + "/" + file_name));

    return bf;

}
