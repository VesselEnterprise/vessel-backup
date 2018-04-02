#include "file.hpp"

using namespace Backup::File;
using namespace Backup::Compression;

BackupFile::BackupFile(const fs::path& fp ) : m_file_path(fp), m_chunk_size(BACKUP_CHUNK_SZ)
{
    m_directory_id=-1;
    m_upload_id=-1;
    update_attributes();
}

BackupFile::~BackupFile() {}

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
        m_unique_id = calculate_unique_id();

        try
        {
            m_file_attrs.file_size = fs::file_size( m_file_path );
        }
        catch (const fs::filesystem_error& e )
        {
            m_file_attrs.file_size = 0;
        }

        try
        {
             m_file_attrs.last_write_time = fs::last_write_time(m_file_path);
        }
        catch (const fs::filesystem_error & e )
        {
            m_file_attrs.last_write_time = 0;
            //Add error log here?
        }

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

    using namespace CryptoPP;

    SHA1 hash;
    std::string digest;

    //Use this method for larger files
    if ( get_file_size() > BACKUP_LARGE_SZ )
    {
        /**
        * This peforms about 20% slower than reading the file manually and passing to StringSource
        */
        FileSource s( get_file_path().c_str(), true, new HashFilter(hash, new HexEncoder( new StringSink(digest), false ) ) );
    }
    else
    {

        if ( m_content.empty() )
        {

            //Read file contents
            std::ifstream infile( get_file_path(), std::ios::in | std::ios::binary );
            if ( !infile.is_open() )
                return ""; //No hash

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

std::string BackupFile::get_hash_sha256()
{

    //If SHA-256 has already been generated, return the hash
    if ( !m_file_attrs.content_sha256.empty() )
        return m_file_attrs.content_sha256;

    using namespace CryptoPP;

    SHA256 hash;
    std::string digest;

    //Use this method for larger files
    if ( get_file_size() > BACKUP_LARGE_SZ )
    {
        /**
        * This peforms about 20% slower than reading the file manually and passing to StringSource
        */
        FileSource s( get_file_path().c_str(), true, new HashFilter(hash, new HexEncoder( new StringSink(digest), false ) ) );
    }
    else
    {

        if ( m_content.empty() )
        {

            //Read file contents
            std::ifstream infile( get_file_path(), std::ios::in | std::ios::binary );
            if ( !infile.is_open() )
                return ""; //No hash

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

    using namespace CryptoPP;

    SHA1 hash;
    std::string digest;

    StringSource s(data, true, new HashFilter(hash, new HexEncoder( new StringSink(digest), false ) ) );

    return digest;

}

std::string BackupFile::get_hash_sha256( const std::string& data ) const
{

    using namespace CryptoPP;

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
    if ( !infile.is_open() )
        return "";

    //infile.ignore(  std::numeric_limits<std::streamsize>::max(), '\0' );

    infile.seekg( 0, std::ios::end );
    auto file_size = infile.tellg();
    m_content.resize( file_size ); //Pre-allocate memory
    infile.seekg( 0, std::ios::beg );
    infile.read( &m_content[0], m_content.size() ); //Read contents

    infile.close(); //Close file

    return m_content;

}

std::string BackupFile::calculate_unique_id() const
{
        using namespace CryptoPP;

        SHA1 hash;
        std::string digest;

        StringSource s(get_canonical_path(), true, new HashFilter(hash, new HexEncoder( new StringSink(digest), false ) ) );

        return digest;
}

std::string BackupFile::get_unique_id() const
{
    return m_unique_id;
}

std::unique_ptr<unsigned char*> BackupFile::get_unique_id_raw() const
{
    using namespace Backup::Utilities;
    return Hash::get_sha1_hash_raw(get_canonical_path());
}

unsigned int BackupFile::get_directory_id() const
{
    return m_directory_id;
}

void BackupFile::set_directory_id(unsigned int id)
{
    m_directory_id = id;
}

unsigned int BackupFile::get_file_id() const
{
    return m_file_id;
}

void BackupFile::set_file_id(unsigned int id)
{
    m_file_id = id;
}

void BackupFile::set_chunk_size(size_t chunk_sz)
{
    m_chunk_size = chunk_sz;
}

unsigned int BackupFile::get_total_parts() const
{
    return std::ceil( get_file_size() / (double)m_chunk_size );
}

void BackupFile::set_upload_id(unsigned int id)
{
    m_upload_id = id;
}

unsigned int BackupFile::get_upload_id() const
{
    return m_upload_id;
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
    file_part.resize( m_chunk_size ); //Optimize string alloc

    //If file is > 50 MB, we do not want to store all the contents in memory
    if ( total_bytes > BACKUP_LARGE_SZ )
    {

        //Read file contents
        std::ifstream infile( get_file_path(), std::ios::in | std::ios::binary );
        if ( !infile.is_open() )
            return "";

        infile.seekg( start_pos, std::ios::beg );
        infile.read( &file_part[0], m_chunk_size ); //Read contents

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

std::shared_ptr<BackupFile> BackupFile::get_compressed_copy()
{
    const std::string tmp_file = ("tmp/" + get_file_name() + ".tmp");
    if ( !fs::exists(tmp_file) )
    {
        Compressor* c = new Compressor();
        c->compress_file( get_file_path(), tmp_file );
    }

    return std::make_shared<BackupFile>(tmp_file);
}

size_t BackupFile::get_chunk_size() const
{
    return m_chunk_size;
}

std::string BackupFile::find_mime_type( const std::string& ext )
{

    using namespace Backup::Database;

    LocalDatabase* ldb = &LocalDatabase::get_database();

    sqlite3_stmt* stmt;
    std::string query = "SELECT mime FROM backup_mime_type WHERE ext=LOWER(?1)";

    if ( sqlite3_prepare_v2(ldb->get_handle(), query.c_str(), query.size(), &stmt, NULL ) != SQLITE_OK )
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
