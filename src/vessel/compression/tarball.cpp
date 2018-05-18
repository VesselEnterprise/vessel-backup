#include <vessel/compression/tarball.hpp>

using namespace Backup::Compression;

Tarball::Tarball(const std::string& filename )
{
    create_tar(filename);
}

Tarball::~Tarball()
{

}

void Tarball::create_tar(const std::string& filename)
{

    std::string opts = "compression-level=" + std::to_string(Z_COMP_LEVEL);

    m_archive = archive_write_new();
    archive_write_add_filter_gzip( m_archive );
    archive_write_set_options(m_archive, opts.c_str() );
    archive_write_set_format_pax_restricted( m_archive );
    archive_write_open_filename( m_archive, filename.c_str() );
    m_total_files=0;

}

void Tarball::add_file(const std::string& filename)
{

    struct archive_entry * entry = archive_entry_new();
    char buff[8192];

    size_t fs = boost::filesystem::file_size(filename);

    archive_entry_set_pathname(entry, filename.c_str());
    archive_entry_set_size(entry, fs);
    archive_entry_set_filetype(entry, AE_IFREG);
    archive_entry_set_perm(entry, 0644);
    archive_write_header(m_archive, entry);

    std::ifstream infile(filename, std::ifstream::in | std::ifstream::binary );
    if ( !infile.is_open() )
        return;

    while ( !infile.eof() && infile.good() ) {

        infile.read(buff,sizeof(buff));
        archive_write_data(m_archive, buff, infile.gcount());

    }

    infile.close();
    archive_entry_free(entry);

    m_total_files++;

}

void Tarball::save_tar()
{
    archive_write_close(m_archive);
    archive_write_free(m_archive);
}
