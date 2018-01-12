#include "file.hpp"

using namespace Backup::File;
namespace fs = boost::filesystem;

BackupFile::BackupFile(const std::string& fp ) : m_file_path(fp)
{
    update_attributes();
}

BackupFile::BackupFile(const fs::path& fp ) : m_file_path(fp)
{
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
        m_file_attrs.file_type = m_file_path.extension().string();
        m_file_attrs.file_size = fs::file_size( m_file_path );
        m_unique_id = calculate_unique_id();

        try
        {
             m_file_attrs.last_write_time = fs::last_write_time(m_file_path);
        }
        catch (const fs::filesystem_error & e )
        {
            //Add error log here
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

std::string BackupFile::get_file_name()
{
    return m_file_attrs.file_name;
}

size_t BackupFile::get_file_size()
{
    return m_file_attrs.file_size;
}

std::string BackupFile::get_file_type()
{
    return m_file_attrs.file_type;
}

std::string BackupFile::get_file_path()
{
    return m_file_attrs.file_path;
}

std::string BackupFile::get_parent_path()
{
    return m_file_attrs.parent_path;
}

std::string BackupFile::get_relative_path()
{
    return m_file_attrs.relative_path;
}

unsigned long BackupFile::get_last_modified()
{
    return m_file_attrs.last_write_time;
}

std::string BackupFile::get_hash(bool use_file_source)
{

    using namespace CryptoPP;

    SHA1 hash;
    std::string digest;

    if ( use_file_source )
    {
        /**
        * This peforms about 20% slower than reading the file manually and passing to StringSource
        */
        FileSource s( get_file_path().c_str(), true, new HashFilter(hash, new HexEncoder( new StringSink(digest) ) ) );
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

        StringSource s(m_content, true, new HashFilter(hash, new HexEncoder( new StringSink(digest) ) ) );

    }

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

std::string BackupFile::calculate_unique_id()
{
        using namespace CryptoPP;

        SHA1 hash;
        std::string digest;

        StringSource s(m_file_path.string(), true, new HashFilter(hash, new HexEncoder( new StringSink(digest) ) ) );

        return digest;
}

std::string BackupFile::get_unique_id()
{
    return m_unique_id;
}

unsigned int BackupFile::get_directory_id()
{
    return m_directory_id;
}

void BackupFile::set_directory_id(unsigned int id)
{
    m_directory_id = id;
}

unsigned int BackupFile::get_file_id()
{
    return m_file_id;
}

void BackupFile::set_file_id(unsigned int id)
{
    m_file_id = id;
}
