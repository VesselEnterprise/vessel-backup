#include <vessel/filesystem/file_upload.hpp>

FileUpload::FileUpload(const std::string& upload_key)
{
    m_upload_key = upload_key;
    init();
}

FileUpload::FileUpload(unsigned int upload_id)
{
    m_upload_id = upload_id;
    init();
}

void FileUpload::init()
{

    std::string where = "upload_id";

    if ( !m_upload_key.empty() ) {
        where = "upload_key";
    }

    sqlite3_stmt* stmt;
    std::string query = "SELECT file_id,total_parts,byte_offset,chunk_size,hash,signature,weight,last_modified,upload_id,upload_key,vessel_id FROM backup_upload WHERE " + where + "=?1";

    if ( sqlite3_prepare_v2(LocalDatabase::get_database().get_handle(), query.c_str(), -1, &stmt, NULL ) != SQLITE_OK ) {
        Log::get_log().add_error("Failed to init FileUpload: " + m_upload_id, "FileUpload");
        return;
    }

    if ( where == "upload_id" ) {
        sqlite3_bind_int(stmt, 1, m_upload_id );
    }
    else {
        sqlite3_bind_text(stmt, 1, m_upload_key.c_str(), m_upload_key.size(), 0 );
    }

    if ( sqlite3_step(stmt) == SQLITE_ROW )
    {
        m_file_id.reset();
        m_file_id = LocalDatabase::get_binary_id( (unsigned char*)sqlite3_column_blob(stmt,0) );
        m_file = BackupFile(m_file_id);
        m_upload_id = sqlite3_column_int(stmt, 8);
        m_upload_key = LocalDatabase::get_sqlite_str( sqlite3_column_text(stmt, 9) );
        m_total_parts = sqlite3_column_int(stmt, 1);
        m_offset = sqlite3_column_int(stmt, 2);
        m_chunk_size = sqlite3_column_int(stmt, 3);
        m_file_hash = LocalDatabase::get_sqlite_str( sqlite3_column_text(stmt, 4) );
        m_signature = LocalDatabase::get_sqlite_str( sqlite3_column_text(stmt, 5) );
        m_weight = sqlite3_column_int(stmt, 6);
        m_last_modified = sqlite3_column_int(stmt, 7);
        m_vessel_id = LocalDatabase::get_sqlite_str( sqlite3_column_text(stmt, 10) );
        m_exists = true;

    }
    else {
        Log::get_log().add_error("Failed to init FileUpload: " + m_upload_id, "FileUpload");
    }

    //Cleanup
    sqlite3_finalize(stmt);

}

void FileUpload::update_key(const std::string& upload_key)
{

    sqlite3_stmt* stmt;
    std::string query = "UPDATE backup_upload SET upload_key=?1 WHERE upload_id=?2";

    if ( sqlite3_prepare_v2(LocalDatabase::get_database().get_handle(), query.c_str(), -1, &stmt, NULL ) != SQLITE_OK ) {
        Log::get_log().add_error("Failed to set upload_id: " + upload_key, "FileUpload");
        return;
    }

    sqlite3_bind_text(stmt, 1, upload_key.c_str(), upload_key.size(), 0);
    sqlite3_bind_int(stmt, 2, m_upload_id );

    if ( sqlite3_step(stmt) == SQLITE_DONE ) {
        m_upload_key = upload_key;
    }
    else {
        Log::get_log().add_error("Failed to set upload_id: " + upload_key, "FileUpload");
    }

    //Cleanup
    sqlite3_finalize(stmt);

}

int FileUpload::get_current_part() const
{

    int part_number = 1; //Default

    sqlite3_stmt* stmt;
    std::string query = "SELECT part_number FROM backup_upload_part WHERE upload_id=?1 ORDER BY part_number DESC LIMIT 1";

    if ( sqlite3_prepare_v2(LocalDatabase::get_database().get_handle(), query.c_str(), -1, &stmt, NULL ) != SQLITE_OK ) {
        Log::get_log().add_error("Query failed to return current part number: " + std::to_string(m_upload_id), "FileUpload");
        return part_number;
    }

    sqlite3_bind_int(stmt, 1, m_upload_id);

    if ( sqlite3_step(stmt) == SQLITE_ROW ) {
        part_number = sqlite3_column_int(stmt, 0) + 1; //Current part is the last uploaded plus one
    }
    //Cleanup
    sqlite3_finalize(stmt);

    return part_number;

}

void FileUpload::add_part(const FilePart& part) const
{

    std::string query = "INSERT INTO backup_upload_part (upload_id,part_number,bytes,tag,signature) VALUES(?1,?2,?3,?4,?5)";

    sqlite3_stmt* stmt;
    if ( sqlite3_prepare_v2(LocalDatabase::get_database().get_handle(), query.c_str(), -1, &stmt, NULL ) != SQLITE_OK ) {
        Log::get_log().add_error("Failed to add file part for upload id: " + std::to_string(part.upload_id), "File Upload");
        return;
    }

    sqlite3_bind_int(stmt, 1, part.upload_id );
    sqlite3_bind_int(stmt, 2, part.part_number );
    sqlite3_bind_int(stmt, 3, part.total_bytes );
    sqlite3_bind_text(stmt, 4, part.tag.c_str(), part.tag.size(), 0 );
    sqlite3_bind_text(stmt, 5, part.signature.c_str(), part.signature.size(), 0 );

    if ( sqlite3_step(stmt) != SQLITE_DONE ) {
        Log::get_log().add_error("Failed to add file part for upload id: " + std::to_string(part.upload_id), "File Upload");
    }

    //Cleanup
    sqlite3_finalize(stmt);

}

std::vector<UploadTagSet> FileUpload::get_part_tags() const
{

    std::vector<UploadTagSet> tags;

    sqlite3_stmt* stmt;
    std::string query = "SELECT part_number,tag FROM backup_upload_part WHERE upload_id=?1 ORDER BY part_number ASC";

    if ( sqlite3_prepare_v2(LocalDatabase::get_database().get_handle(), query.c_str(), -1, &stmt, NULL ) != SQLITE_OK ) {
        Log::get_log().add_error("Failed to retrieve upload tags: " + std::to_string(m_upload_id), "FileUpload");
        return tags;
    }

    sqlite3_bind_int(stmt, 1, m_upload_id);

    while ( sqlite3_step(stmt) == SQLITE_ROW ) {
        UploadTagSet tag_pair;
        tag_pair.part_number = sqlite3_column_int(stmt,0);
        tag_pair.tag = LocalDatabase::get_sqlite_str( sqlite3_column_text(stmt, 1) );
        tags.push_back( tag_pair );
    }

    //Cleanup
    sqlite3_finalize(stmt);

    return tags;

}

BackupFile FileUpload::get_file()
{
    return m_file;
}

int FileUpload::get_error_count() const
{

    int error_count = 0;

    sqlite3_stmt* stmt;
    std::string query = "SELECT error_count FROM backup_upload WHERE upload_id=?1";

    if ( sqlite3_prepare_v2(LocalDatabase::get_database().get_handle(), query.c_str(), -1, &stmt, NULL ) != SQLITE_OK ) {
        Log::get_log().add_error("Query failed to get upload error count: " + std::to_string(m_upload_id), "FileUpload");
        return 0;
    }

    sqlite3_bind_int(stmt, 1, m_upload_id);

    if ( sqlite3_step(stmt) == SQLITE_ROW ) {
        error_count = sqlite3_column_int(stmt, 0);
    }

    //Cleanup
    sqlite3_finalize(stmt);

    return error_count;

}

int FileUpload::increment_error()
{

    sqlite3_stmt* stmt;
    std::string query = "UPDATE backup_upload SET error_count = error_count+1 WHERE upload_id=?1";

    if ( sqlite3_prepare_v2(LocalDatabase::get_database().get_handle(), query.c_str(), -1, &stmt, NULL ) != SQLITE_OK ) {
        Log::get_log().add_error("Unable to increase upload error count: " + LocalDatabase::get_database().get_last_err(), "File Upload");
    }

    sqlite3_bind_int(stmt, 1, m_upload_id );

    if ( sqlite3_step(stmt) != SQLITE_DONE ) {
        Log::get_log().add_error("Unable to increase upload error count: " + LocalDatabase::get_database().get_last_err(), "File Upload");
    }

    //Cleanup
    sqlite3_finalize(stmt);

    return (get_error_count() + 1);

}

void FileUpload::update_vessel_id(const std::string& id)
{

    sqlite3_stmt* stmt;
    std::string query = "UPDATE backup_upload SET vessel_id = ?1 WHERE upload_id=?2";

    if ( sqlite3_prepare_v2(LocalDatabase::get_database().get_handle(), query.c_str(), -1, &stmt, NULL ) != SQLITE_OK ) {
        Log::get_log().add_error("Unable to set Vessel ID for FileUpload: " + LocalDatabase::get_database().get_last_err(), "File Upload");
    }

    sqlite3_bind_text(stmt, 1, id.c_str(), id.size(), 0 );
    sqlite3_bind_int(stmt, 2, m_upload_id );

    if ( sqlite3_step(stmt) != SQLITE_DONE ) {
        Log::get_log().add_error("Unable to set Vessel ID for FileUpload: " + LocalDatabase::get_database().get_last_err(), "File Upload");
    }

    m_vessel_id = id;

    //Cleanup
    sqlite3_finalize(stmt);

}

std::string FileUpload::get_vessel_id()
{
    return m_vessel_id;
}
