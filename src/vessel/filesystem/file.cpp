#include <vessel/filesystem/file.hpp>

using namespace Vessel::File;

size_t BackupFile::m_chunk_size = BACKUP_CHUNK_SZ;

BackupFile::BackupFile(const fs::path& fp ) : m_file_path(fp)
{
    m_directory_id=-1;
    m_readable=true;
    update_attributes();
}

BackupFile::BackupFile( std::shared_ptr<unsigned char> file_id )
{

    //Set the file id
    m_file_id = file_id;

    sqlite3_stmt* stmt;

    std::string query = "SELECT a.filename,a.file_ext,a.filesize,a.directory_id,a.last_modified,b.path FROM backup_file AS a INNER JOIN backup_directory AS b ON a.directory_id=b.directory_id WHERE a.file_id=?1";

    if ( sqlite3_prepare_v2(LocalDatabase::get_database().get_handle(), query.c_str(), -1, &stmt, NULL ) != SQLITE_OK ) {
        throw FileException(FileException::FileNotFound, "Error querying for database file");
    }

    sqlite3_bind_blob(stmt, 1, file_id.get(), sizeof(file_id.get()), 0);

    if ( sqlite3_step(stmt) != SQLITE_ROW ) {
        throw FileException(FileException::FileNotFound, "File does not exist in database");
    }

    std::string parent_path = (char*)sqlite3_column_text(stmt, 5);
    m_file_attrs.file_name = (char*)sqlite3_column_text(stmt, 0);
    m_file_path = boost::filesystem::path( parent_path + PATH_SEPARATOR() + m_file_attrs.file_name );
    m_file_attrs.file_path = m_file_path.string();
    m_file_attrs.relative_path = m_file_path.relative_path().string();
    m_file_attrs.parent_path = m_file_path.parent_path().string();
    m_file_attrs.file_type = m_file_path.extension().string();
    m_file_attrs.mime_type = find_mime_type(m_file_attrs.file_type);
    m_file_attrs.file_size = (unsigned long)sqlite3_column_int(stmt,2);
    m_file_attrs.last_write_time = (unsigned long)sqlite3_column_int(stmt, 4);
    m_directory_id = (int)sqlite3_column_int(stmt, 3);

    try
    {
        m_file_attrs.canonical_path = boost::filesystem::canonical(m_file_path).string();
    }
    catch( const boost::filesystem::filesystem_error& ex)
    {
        m_readable=false;
        Log::get_log().add_error( std::string("File does not exist: ") + ex.what(), "Filesystem");
    }

    //Set string file id
    m_file_id_s = Hash::get_sha1_hash(get_canonical_path());

    //Cleanup
    sqlite3_finalize(stmt);

}

void BackupFile::update_attributes()
{
    if ( fs::exists(m_file_path) )
    {
        m_file_attrs.file_name = m_file_path.filename().string();
        m_file_attrs.file_path = m_file_path.string();
        m_file_attrs.relative_path = m_file_path.relative_path().string();
        m_file_attrs.parent_path = m_file_path.parent_path().string();
        m_file_attrs.canonical_path = boost::filesystem::canonical(m_file_path).string();
        m_file_attrs.file_type = m_file_path.extension().string();
        m_file_attrs.mime_type = find_mime_type(m_file_attrs.file_type);
        m_file_id = Hash::get_sha1_hash_ptr(get_canonical_path());
        m_file_id_s = Hash::get_sha1_hash(get_canonical_path());

        try
        {
            m_file_attrs.file_size = fs::file_size( m_file_path );
        }
        catch (const fs::filesystem_error& e )
        {
            m_file_attrs.file_size = 0;
            Log::get_log().add_error( std::string("Unable to determine file size: " + m_file_attrs.file_name + " (") + e.what() + ")", "Filesystem");
        }

        try
        {
             m_file_attrs.last_write_time = fs::last_write_time(m_file_path);
        }
        catch (const fs::filesystem_error & e )
        {
            m_file_attrs.last_write_time = 0;
            Log::get_log().add_error( std::string("Unable to get file last write time: " + m_file_attrs.file_name + " (") + e.what() + ")", "Filesystem");
        }

    }
    else
    {
        m_readable=false;
        //hrow FileException(FileException::FileNotFound, "File does not exist: " + m_file_path.string() );
    }
}

void BackupFile::set_path( const std::string& fp )
{
    m_file_path = fs::path(fp);
    update_attributes();
    m_content.clear(); //Clear file content if it's been read
}

void BackupFile::set_path( const fs::path& fp )
{
    m_file_path = fs::path(fp);
    update_attributes();
}

bool BackupFile::exists()
{
    return fs::exists(m_file_path);
}

std::string BackupFile::get_file_name() const
{
    return m_file_attrs.file_name;
}

size_t BackupFile::get_file_size() const
{
    return m_file_attrs.file_size;
}

std::string BackupFile::get_file_type() const
{
    return m_file_attrs.file_type;
}

std::string BackupFile::get_mime_type() const
{
    return m_file_attrs.mime_type;
}

std::string BackupFile::get_file_path() const
{
    return m_file_attrs.file_path;
}

std::string BackupFile::get_parent_path() const
{
    return m_file_attrs.parent_path;
}

std::string BackupFile::get_relative_path() const
{
    return m_file_attrs.relative_path;
}

std::string BackupFile::get_canonical_path() const
{
    return m_file_attrs.canonical_path;
}

unsigned long BackupFile::get_last_modified() const
{
    return m_file_attrs.last_write_time;
}

std::string BackupFile::get_hash_sha1()
{

    //If SHA-1 has already been generated, return the hash
    if ( !m_file_attrs.content_sha1.empty() )
        return m_file_attrs.content_sha1;

    SHA1 hash;
    std::string digest;

    //Use this method for larger files
    if ( get_file_size() > BACKUP_LARGE_SZ )
    {
        /**
        * This peforms about 20% slower than reading the file manually and passing to StringSource
        */
        try
        {
            FileSource s( get_file_path().c_str(), true, new HashFilter(hash, new HexEncoder( new StringSink(digest), false ) ) );
        }
        catch(const std::exception& ex)
        {
            m_readable=false;
        }
    }
    else
    {

        if ( m_content.empty() )
        {

            //Read file contents
            std::ifstream infile( get_file_path(), std::ios::in | std::ios::binary );
            if ( !infile.is_open() ) {
                m_readable = false;
                return ""; //No hash
            }

            infile.seekg( 0, std::ios::end );
            auto file_size = infile.tellg();
            m_content.resize( file_size ); //Pre-allocate memory
            infile.seekg( 0, std::ios::beg );
            infile.read( &m_content[0], m_content.size() ); //Read contents
            infile.close(); //Close file

        }

        StringSource s(m_content, true, new HashFilter(hash, new HexEncoder( new StringSink(digest), false ) ) );

    }

    return digest;

}

std::string BackupFile::get_hash_sha1() const
{

    //If SHA-1 has already been generated, return the hash
    if ( !m_file_attrs.content_sha1.empty() )
        return m_file_attrs.content_sha1;

    SHA1 hash;
    std::string digest;

    //Use this method for larger files
    if ( get_file_size() > BACKUP_LARGE_SZ )
    {
        /**
        * This peforms about 20% slower than reading the file manually and passing to StringSource
        */
        try
        {
            FileSource s( get_file_path().c_str(), true, new HashFilter(hash, new HexEncoder( new StringSink(digest), false ) ) );
        }
        catch ( const std::exception& ex )
        {
            //Log an error here?
        }
    }
    else
    {
        //Read file contents
        std::string content;
        std::ifstream infile( get_file_path(), std::ios::in | std::ios::binary );
        if ( !infile.is_open() ) {
            //Log an error?
            return ""; //No hash
        }

        infile.seekg( 0, std::ios::end );
        auto file_size = infile.tellg();
        content.resize( file_size ); //Pre-allocate memory
        infile.seekg( 0, std::ios::beg );
        infile.read( &content[0], content.size() ); //Read contents
        infile.close(); //Close file

        StringSource s(content, true, new HashFilter(hash, new HexEncoder( new StringSink(digest), false ) ) );

    }

    return digest;

}

std::string BackupFile::get_hash_sha256()
{

    //If SHA-256 has already been generated, return the hash
    if ( !m_file_attrs.content_sha256.empty() )
        return m_file_attrs.content_sha256;

    SHA256 hash;
    std::string digest;

    //Use this method for larger files
    if ( get_file_size() > BACKUP_LARGE_SZ )
    {
        /**
        * This peforms about 20% slower than reading the file manually and passing to StringSource
        */
        try
        {
            FileSource s( get_file_path().c_str(), true, new HashFilter(hash, new HexEncoder( new StringSink(digest), false ) ) );
        }
        catch( const std::exception& ex )
        {
            m_readable=false;
        }
    }
    else
    {

        if ( m_content.empty() )
        {

            //Read file contents
            std::ifstream infile( get_file_path(), std::ios::in | std::ios::binary );
            if ( !infile.is_open() ) {
                m_readable=false;
                return ""; //No hash
            }

            infile.seekg( 0, std::ios::end );
            auto file_size = infile.tellg();
            m_content.resize( file_size ); //Pre-allocate memory
            infile.seekg( 0, std::ios::beg );
            infile.read( &m_content[0], m_content.size() ); //Read contents
            infile.close(); //Close file

        }

        StringSource s(m_content, true, new HashFilter(hash, new HexEncoder( new StringSink(digest), false ) ) );

    }

    return digest;

}

std::string BackupFile::get_hash_sha1( const std::string& data ) const
{

    SHA1 hash;
    std::string digest;

    StringSource s(data, true, new HashFilter(hash, new HexEncoder( new StringSink(digest), false ) ) );

    return digest;

}

std::string BackupFile::get_hash_sha256( const std::string& data ) const
{

    SHA256 hash;
    std::string digest;

    StringSource s(data, true, new HashFilter(hash, new HexEncoder( new StringSink(digest), false ) ) );

    return digest;

}

std::string BackupFile::get_file_contents()
{

    //If file has already been read and content was stored, return the existing data
    if ( !m_content.empty() )
        return m_content;

    //Read file contents
    std::ifstream infile( get_file_path(), std::ios::in | std::ios::binary );
    if ( !infile.is_open() ) {
        m_readable=false;
        return "";
    }

    //infile.ignore(  std::numeric_limits<std::streamsize>::max(), '\0' );

    infile.seekg( 0, std::ios::end );
    auto file_size = infile.tellg();
    m_content.resize( file_size ); //Pre-allocate memory
    infile.seekg( 0, std::ios::beg );
    infile.read( &m_content[0], m_content.size() ); //Read contents

    infile.close(); //Close file

    return m_content;

}

unsigned int BackupFile::get_directory_id() const
{
    return m_directory_id;
}

void BackupFile::set_directory_id(unsigned int id)
{
    m_directory_id = id;
}

std::shared_ptr<unsigned char> BackupFile::get_file_id() const
{
    return m_file_id;
}

std::string BackupFile::get_file_id_text() const
{
    return m_file_id_s;
}

void BackupFile::set_chunk_size(size_t chunk_sz)
{
    m_chunk_size = chunk_sz;
}

unsigned int BackupFile::get_total_parts() const
{
    return std::ceil( get_file_size() / (double)m_chunk_size );
}

void BackupFile::set_upload_id(unsigned int upload_id)
{
    m_upload_id = upload_id;
}

void BackupFile::set_upload_key(const std::string& upload_key)
{
    m_upload_key = upload_key;
}

unsigned int BackupFile::get_upload_id() const
{
    return m_upload_id;
}

std::string BackupFile::get_upload_key() const
{
    return m_upload_key;
}

std::string BackupFile::get_file_part(unsigned int num) {

    size_t total_bytes = get_file_size();

    size_t start_pos = ((m_chunk_size * num) - m_chunk_size);
    size_t end_pos = (start_pos + m_chunk_size) -1;

    if ( start_pos >= total_bytes )
        start_pos = (total_bytes - m_chunk_size) >= 0 ? (total_bytes - m_chunk_size) : 0;

    if ( total_bytes <= end_pos )
        end_pos = get_file_size()-1;

    std::string file_part;

    //If file is > 50 MB, we do not want to store all the contents in memory
    if ( total_bytes > BACKUP_LARGE_SZ )
    {

        size_t bytes_to_read = (end_pos - start_pos) + 1;

        file_part.resize( bytes_to_read ); //Optimize string alloc

        //Read file contents
        std::ifstream infile( get_file_path(), std::ios::in | std::ios::binary );
        if ( !infile.is_open() ) {
            m_readable = false;
            return "";
        }

        infile.seekg( start_pos, std::ios::beg );
        infile.read( &file_part[0], bytes_to_read ); //Read contents

        infile.close(); //Close file

    }
    else
    {
        if ( m_content.empty() )
            get_file_contents();

        file_part = m_content.substr(start_pos, end_pos);
    }

    return file_part;

}

/*
std::shared_ptr<BackupFile> BackupFile::get_compressed_copy()
{
    const std::string tmp_file = ("tmp/" + get_file_name() + ".tmp");
    if ( !fs::exists(tmp_file) )
    {
        Compressor c;
        c.compress_file( get_file_path(), tmp_file );
    }

    return std::make_shared<BackupFile>(tmp_file);
}
*/

size_t BackupFile::get_chunk_size()
{
    return m_chunk_size;
}

std::string BackupFile::find_mime_type( const std::string& ext )
{

    LocalDatabase* ldb = &LocalDatabase::get_database();

    sqlite3_stmt* stmt;
    std::string query = "SELECT mime FROM backup_mime_type WHERE ext=LOWER(?1)";

    if ( sqlite3_prepare_v2(ldb->get_handle(), query.c_str(), -1, &stmt, NULL ) != SQLITE_OK )
        return "";

    sqlite3_bind_text(stmt, 1, ext.c_str(), ext.size(), 0 );

    std::string val="";

    if ( sqlite3_step(stmt) == SQLITE_ROW )
    {
        val = (char*)sqlite3_column_text(stmt,0);
    }

    //Cleanup
    sqlite3_finalize(stmt);

    return val;
}

bool BackupFile::is_readable()
{
    std::ifstream infile( get_canonical_path() );
    if ( !infile.is_open() )
    {
        m_readable=false;
        return false;
    }
    infile.close();
    m_readable=true;
    return true;
}

void BackupFile::update_last_backup()
{
    update_last_backup( get_file_id() );
}

void BackupFile::update_last_backup(std::shared_ptr<unsigned char> file_id)
{

    std::time_t now = std::time(nullptr);

    sqlite3_stmt* stmt;
    std::string query = "UPDATE backup_file SET last_backup_time=?1 WHERE file_id=?2";

    if ( sqlite3_prepare_v2(LocalDatabase::get_database().get_handle(), query.c_str(), -1, &stmt, NULL ) != SQLITE_OK ) {
        Log::get_log().add_error("Unable to set file last backup time: " + LocalDatabase::get_database().get_last_err(), "File Backup");
    }

    sqlite3_bind_int(stmt, 1, now );
    sqlite3_bind_blob(stmt, 2, file_id.get(), sizeof(file_id.get()), 0 );

    if ( sqlite3_step(stmt) != SQLITE_DONE ) {
        Log::get_log().add_error("Unable to set file last backup time: " + LocalDatabase::get_database().get_last_err(), "File Backup");
    }

    //Cleanup
    sqlite3_finalize(stmt);

}

std::string BackupFile::get_chunk(size_t offset, size_t length)
{

    size_t total_bytes = get_file_size();

    //Prevent exception
    if ( offset > total_bytes )
    {
        return "";
    }

    //If file has already been read and content was stored, return the existing data
    if ( !m_content.empty() )
    {
        return m_content.substr(offset, length);
    }

    //Read file contents
    std::ifstream infile( get_file_path(), std::ios::in | std::ios::binary );
    if ( !infile.is_open() ) {
        m_readable=false;
        return "";
    }

    std::string chunk;
    size_t bytes_to_read = ((offset+length) > total_bytes) ? (total_bytes-offset) : length;

    infile.seekg( offset, std::ios::beg );
    chunk.resize( length ); //Pre-allocate memory
    infile.read( &chunk[0], bytes_to_read ); //Read contents

    infile.close(); //Close file

    return chunk;

}

std::string BackupFile::trim_path(const std::string& str)
{

    std::string path = boost::trim_left_copy_if(str, boost::is_any_of("/\\"));
    boost::trim_right_if(path, boost::is_any_of("/\\"));

    return path;

}
