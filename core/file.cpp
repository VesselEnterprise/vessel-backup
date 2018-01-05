#include "file.hpp"

using namespace Backup::File;
namespace fs = boost::filesystem;

BackupFile::BackupFile(const std::string& fp ) : m_file_path(fp)
{
    update_attributes();
}

void BackupFile::update_attributes()
{
    if ( fs::exists(m_file_path) )
    {
        m_file_name = m_file_path.filename().string();
        m_file_type = m_file_path.extension().string();
        m_file_size = fs::file_size( m_file_path );
    }
}

void BackupFile::set_path( const std::string& fp )
{
    m_file_path = fs::path(fp);
    update_attributes();
}

bool BackupFile::exists()
{
    return fs::exists(m_file_path);
}
