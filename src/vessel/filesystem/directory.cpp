#include <vessel/filesystem/directory.hpp>

using namespace Vessel::File;

BackupDirectory::BackupDirectory(const boost::filesystem::path& p) :  m_dir_path(p)
{
    update_attributes();
}

BackupDirectory::BackupDirectory(const BackupDirectory& bd) :  m_dir_path(bd.get_path())
{
    update_attributes();
}

BackupDirectory::~BackupDirectory()
{

}

void BackupDirectory::update_attributes()
{
    if ( fs::exists(m_dir_path) )
    {
        m_dir_attrs.directory_name = m_dir_path.filename().string();
        m_dir_attrs.path = m_dir_path.string();
        m_dir_attrs.relative_path = m_dir_path.relative_path().string();
        m_dir_attrs.parent_path = m_dir_path.parent_path().string();
        m_dir_attrs.canonical_path = boost::filesystem::canonical(m_dir_path).string();
        m_unique_id = calculate_unique_id();

        try
        {
            m_dir_attrs.file_size = fs::file_size( m_dir_path );
        }
        catch ( boost::filesystem::filesystem_error& e )
        {
            m_dir_attrs.file_size = 0;
        }

        try
        {
             m_dir_attrs.last_write_time = fs::last_write_time(m_dir_path);
        }
        catch (const fs::filesystem_error & e )
        {
            //Add error log here
        }

    }
}

void BackupDirectory::set_path( const std::string& fp )
{
    m_dir_path = fs::path(fp);
    update_attributes();
}

void BackupDirectory::set_path( const fs::path& fp )
{
    m_dir_path = fs::path(fp);
    update_attributes();
}

bool BackupDirectory::exists()
{
    return fs::exists(m_dir_path);
}

std::string BackupDirectory::get_dir_name() const
{
    return m_dir_attrs.directory_name;
}

size_t BackupDirectory::get_file_size() const
{
    return m_dir_attrs.file_size;
}

std::string BackupDirectory::get_path() const
{
    return m_dir_attrs.path;
}

std::string BackupDirectory::get_parent_path() const
{
    return m_dir_attrs.parent_path;
}

std::string BackupDirectory::get_relative_path() const
{
    return m_dir_attrs.relative_path;
}

std::string BackupDirectory::get_canonical_path() const
{
    return m_dir_attrs.canonical_path;
}

unsigned long BackupDirectory::get_last_modified() const
{
    return m_dir_attrs.last_write_time;
}

std::string BackupDirectory::calculate_unique_id() const
{
    using namespace CryptoPP;

    SHA1 hash;
    std::string digest;

    StringSource s(get_canonical_path(), true, new HashFilter(hash, new HexEncoder( new StringSink(digest) ) ) );

    return digest;
}

std::string BackupDirectory::get_unique_id() const
{
    return m_unique_id;
}

std::unique_ptr<unsigned char*> BackupDirectory::get_unique_id_raw() const
{
    using namespace Vessel::Utilities;
    return Hash::get_sha1_hash_raw(get_canonical_path());
}

unsigned int BackupDirectory::get_directory_id() const
{
    return m_directory_id;
}

void BackupDirectory::set_directory_id(unsigned int id)
{
    m_directory_id = id;
}
