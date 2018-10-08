#include <vessel/vessel/queue_manager.hpp>

QueueManager::QueueManager()
{
    m_database = &LocalDatabase::get_database();
}

void QueueManager::rebuild_queue()
{

    //Clear the current queue
    clear_queue();

    //Implement priority queing based on file types
    //Inner join will add files with extensions that match with the weighted extensions in the backup_weight_ext table

    sqlite3_stmt* stmt;
    std::string query = "SELECT a.file_id FROM backup_file AS a INNER JOIN backup_weight_ext AS b ON a.file_ext=b.file_ext WHERE a.last_modified > a.last_backup_time ORDER BY a.last_modified DESC LIMIT ?1";

    if ( sqlite3_prepare_v2(m_database->get_handle(), query.c_str(), -1, &stmt, NULL ) != SQLITE_OK ) {
        throw DatabaseException(DatabaseException::InvalidStatement, "Error executing statement with query: " + query + "(" + m_database->get_last_err() + ")" );
        return;
    }

    sqlite3_bind_int(stmt, 1, QUEUE_MAX_ROWS);

    int total_rows = 0;

    //Add files to the queue
    while ( sqlite3_step(stmt) == SQLITE_ROW )
    {
        std::shared_ptr<unsigned char> file_id = LocalDatabase::get_binary_id( (unsigned char*)sqlite3_column_blob(stmt, 0) );
        push_file(file_id);
        total_rows++;

    }

    //Reset the statement
    sqlite3_reset(stmt);

    int remainder = QUEUE_MAX_ROWS - total_rows;

    //If there were less than QUEUE_MAX_ROWS total rows after using file weights, fill up the rest of the queue

    if ( remainder > 0 )
    {

        query = "SELECT file_id FROM backup_file WHERE last_modified > last_backup_time ORDER BY last_modified DESC LIMIT ?1";

        if ( sqlite3_prepare_v2(m_database->get_handle(), query.c_str(), -1, &stmt, NULL ) != SQLITE_OK ) {
            throw DatabaseException(DatabaseException::InvalidStatement, "Error executing statement with query: " + query + "(" + m_database->get_last_err() + ")" );
            return;
        }

        sqlite3_bind_int(stmt, 1, remainder);

        //Add files to the queue
        while ( sqlite3_step(stmt) == SQLITE_ROW )
        {
            std::shared_ptr<unsigned char> file_id = LocalDatabase::get_binary_id( (unsigned char*)sqlite3_column_blob(stmt, 0) );
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

    if ( sqlite3_prepare_v2(m_database->get_handle(), query.c_str(), -1, &stmt, NULL ) != SQLITE_OK ) {
        throw DatabaseException(DatabaseException::InvalidStatement, "Error executing statement with query: " + query + "(" + m_database->get_last_err() + ")" );
        return;
    }

    //Execute query
    sqlite3_step(stmt);

    //Cleanup
    sqlite3_finalize(stmt);

}

void QueueManager::push_file(std::shared_ptr<unsigned char> file_id)
{

    BackupFile file(file_id);

    sqlite3_stmt* stmt;
    std::string query = "INSERT INTO backup_upload (file_id,total_parts,chunk_size,hash) VALUES(?1,?2,?3,?4)";

    if ( sqlite3_prepare_v2(m_database->get_handle(), query.c_str(), -1, &stmt, NULL ) != SQLITE_OK ) {
        throw DatabaseException(DatabaseException::InvalidStatement, "Error executing statement with query: " + query + "(" + m_database->get_last_err() + ")");
        return;
    }

    int total_parts = file.get_total_parts();
    int chunk_size = file.get_chunk_size();
    std::string file_hash = file.get_hash_sha1();

    //Bind file id
    sqlite3_bind_blob(stmt, 1, file_id.get(), sizeof(file_id.get()), 0 );
    sqlite3_bind_int(stmt, 2, total_parts );
    sqlite3_bind_int(stmt, 3, chunk_size );
    sqlite3_bind_text(stmt, 4, file_hash.c_str(), file_hash.size(), 0 );

    //Execute query
    if ( sqlite3_step(stmt) != SQLITE_DONE ) {
        throw DatabaseException(DatabaseException::InvalidQuery, "Error executing query: " + query + "(" + m_database->get_last_err() + ")");
        return;
    }

    //Cleanup
    sqlite3_finalize(stmt);

}

void QueueManager::pop_file(std::shared_ptr<unsigned char> file_id)
{

    sqlite3_stmt* stmt;
    std::string query = "DELETE FROM backup_upload WHERE file_id=?1";

    if ( sqlite3_prepare_v2(m_database->get_handle(), query.c_str(), -1, &stmt, NULL ) != SQLITE_OK ) {
        throw DatabaseException(DatabaseException::InvalidStatement, "Error executing statement with query: " + query + "(" + m_database->get_last_err() + ")" );
    }

    //Bind file id
    sqlite3_bind_blob(stmt, 1, file_id.get(), sizeof(file_id.get()), 0 );

    //Execute query
    if ( sqlite3_step(stmt) != SQLITE_DONE ) {
        throw DatabaseException(DatabaseException::InvalidQuery, "Error executing query: " + query + "(" + m_database->get_last_err() + ")" );
    }

    //Cleanup
    sqlite3_finalize(stmt);

}

void QueueManager::apply_weights()
{

    sqlite3_stmt* stmt;
    std::string query = "UPDATE backup_upload SET weight = weight + (SELECT weight FROM backup_weight_ext AS b INNER JOIN backup_file AS c ON file_id = c.file_id WHERE b.file_ext = c.file_ext)";

    if ( sqlite3_prepare_v2(m_database->get_handle(), query.c_str(), -1, &stmt, NULL ) != SQLITE_OK ) {
        throw DatabaseException(DatabaseException::InvalidStatement, "Error executing statement with query: " + query + "(" + m_database->get_last_err() + ")" );
    }

    //Execute query
    if ( sqlite3_step(stmt) != SQLITE_DONE ) {
        throw DatabaseException(DatabaseException::InvalidQuery, "Error executing query: " + query + "(" + m_database->get_last_err() + ")");
    }

    //Cleanup
    sqlite3_finalize(stmt);

}

int QueueManager::get_total_pending()
{

    sqlite3_stmt* stmt;

    std::string query = "SELECT COUNT(*) FROM backup_upload";

    if ( sqlite3_prepare_v2(m_database->get_handle(), query.c_str(), -1, &stmt, NULL ) != SQLITE_OK ) {
        throw DatabaseException(DatabaseException::InvalidStatement, "Error executing statement with query: " + query + "(" + m_database->get_last_err() + ")");
        return -1;
    }

    int total_pending = 0;

    if ( sqlite3_step(stmt) == SQLITE_ROW )
    {
        total_pending = (int)sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);

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

    std::string query = "SELECT file_id,upload_id,upload_key FROM backup_upload ORDER BY last_modified,weight DESC LIMIT 1";

    if ( sqlite3_prepare_v2(m_database->get_handle(), query.c_str(), -1, &stmt, NULL ) != SQLITE_OK ) {
        throw DatabaseException(DatabaseException::InvalidStatement, "Error executing statement with query: " + query + "(" + m_database->get_last_err() + ")" );
    }

    if ( sqlite3_step(stmt) != SQLITE_ROW )
    {
        throw FileException(FileException::FileNotFound, "No files were found in the upload queue");
    }

    std::shared_ptr<unsigned char> file_id = LocalDatabase::get_binary_id( (unsigned char*)sqlite3_column_blob(stmt, 0) );
    unsigned int upload_id = sqlite3_column_int(stmt, 1);
    std::string upload_key = LocalDatabase::get_sqlite_str( sqlite3_column_text(stmt, 2) );

    sqlite3_finalize(stmt);

    //Get File object by database id
    BackupFile bf( file_id );
    bf.set_upload_id(upload_id);
    bf.set_upload_key(upload_key);

    return bf;

}
FileUpload QueueManager::get_next_upload()
{

    //If there are no pending uploads, rebuild the queue
    if ( get_total_pending() == 0 )
    {
        rebuild_queue();
    }

    sqlite3_stmt* stmt;

    std::string query = "SELECT upload_id FROM backup_upload ORDER BY last_modified,weight DESC LIMIT 1";

    if ( sqlite3_prepare_v2(m_database->get_handle(), query.c_str(), -1, &stmt, NULL ) != SQLITE_OK ) {
        throw DatabaseException(DatabaseException::InvalidStatement, "Error executing statement with query: " + query + "(" + m_database->get_last_err() + ")" );
    }

    if ( sqlite3_step(stmt) != SQLITE_ROW )
    {
        throw FileException(FileException::FileNotFound, "No files were found in the upload queue");
    }

    int upload_id = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);

    //Return FileUpload by Id
    return FileUpload(upload_id);

}
