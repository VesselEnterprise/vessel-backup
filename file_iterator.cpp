#include "file_iterator.hpp"

using std::cout;
using namespace Backup;
namespace fs = boost::filesystem;

FileIterator::FileIterator(const std::string& path) : m_base_path(path), m_skip_dir_periods(false)
{

    //Init log
    m_log = new Backup::Logging::Log("scan");

    //If Path is blank, try to find home folder
    if ( path.empty() )
    {
        //Update user home folder
        std::string home_drive = std::getenv("HOMEDRIVE");
        std::string home_path = std::getenv("HOMEPATH");

        m_base_path = home_drive + home_path;
    }

    try
    {
        m_itr = fs::recursive_directory_iterator(m_base_path);
    }
    catch( const fs::filesystem_error& e )
    {
        m_log->add_message( "Scanner Error: " + std::string(e.what()), "File Scanner");
    }

    //Set current path
    m_current_dir.path = m_base_path.string();
    m_current_dir.folder_name = m_base_path.filename().string();

}

FileIterator::~FileIterator()
{
    delete m_log;
}

std::string FileIterator::get_base_path()
{
    return m_base_path.string();
}

void FileIterator::scan()
{

    if ( !fs::exists(m_current_dir.path) )
    {
        m_log->add_message("Error: Scan failed. " + m_current_dir.path + " does not exist", "File Scanner");
        return;
    }
    else
    {
        //Always ensure directory has been added to DB
        m_ldb->add_directory(&m_current_dir);
    }

    try
    {

        for ( ; m_itr != m_itr_end; ++m_itr )
        {

            cout << "Testing: " << *m_itr << '\n';
            cout << "Level: " << m_itr.level() << '\n';
            cout << "Filename:" << m_itr->path().filename() << '\n';
            cout << "Stem:" << m_itr->path().stem() << '\n';
            cout << "Extension:" << m_itr->path().extension() << '\n';
            cout << "Generic Path:" << m_itr->path().generic_path() << '\n';
            cout << "Relative Path:" << m_itr->path().relative_path() << '\n';
            cout << "Root Path:" << m_itr->path().root_path() << '\n';
            cout << "Parent Path:" << m_itr->path().parent_path() << '\n';

            if ( fs::is_directory(*m_itr) )
            {

                //Check if this directory needs to be skipped
                if ( this->skip_dir(*m_itr, m_itr.level() ) )
                {
                    m_log->add_message("Skipped directory exception: " + m_itr->path().string(), "File Scanner");
                    m_itr.no_push();
                    continue;
                }

                //Update current directory information
                if ( (m_itr->path().string() != m_current_dir.path) || m_current_dir.directory_id < 0 )
                {

                    cout << "New path is: " << m_itr->path().string() << '\n';
                    cout << "Old path is: " << m_current_dir.path << '\n';

                    m_current_dir.folder_name = m_itr->path().filename().string();
                    m_current_dir.path = m_itr->path().string();
                    m_current_dir.last_modified = get_last_write_t( *m_itr );

                    m_ldb->add_directory(&m_current_dir);

                    cout << "Directory ID is: " << m_current_dir.directory_id << '\n';

                    //system("PAUSE");

                }

                cout << *m_itr << " is a directory" << '\n';

            }
            else if ( fs::is_regular_file(*m_itr) )
            {

                //Make sure we are in the right directory
                if ( m_current_dir.path != m_itr->path().parent_path().string() )
                {
                    m_current_dir.folder_name = m_itr->path().parent_path().filename().string();
                    m_current_dir.path = m_itr->path().parent_path().string();
                    m_current_dir.last_modified = get_last_write_t(*m_itr);
                    m_ldb->add_directory(&m_current_dir);
                }

                Types::file_data fd;
                fd.filename = m_itr->path().filename().string();
                fd.file_ext = m_itr->path().extension().string();
                fd.filesize = fs::file_size(*m_itr);
                fd.parent_path = m_itr->path().parent_path().string();
                fd.directory_id = m_current_dir.directory_id;
                fd.last_modified = get_last_write_t(*m_itr);

                cout << "Directory ID is: " << fd.directory_id << '\n';

                //system("PAUSE");

                m_ldb->add_file( &fd );

            }
            else
                cout << *m_itr << " exists, but is not a regular file or directory\n";

        }

    }
    catch (const fs::filesystem_error& ex)
    {
        //ERROR LOGGING HERE
        m_log->add_message("Scan Error: " + std::string(ex.what()), "File Scanner");
        ++m_itr;
        this->scan();
        return;

    }

}

void FileIterator::set_local_db(Database::LocalDatabase* db)
{
    m_ldb = db;

    if ( m_ldb->get_setting_int("skip_period_dirs") > 0 )
        m_skip_dir_periods=true;
}

bool FileIterator::skip_dir(const fs::path& p, int level)
{

    if ( m_skip_dir_periods && p.filename().string()[0] == '.' )
        return true;

    if ( m_ldb->is_ignore_dir(p, level) )
        return true;

    return false;

}

unsigned long FileIterator::get_last_write_t( const fs::path& p )
{

    unsigned long t;

    //Sometimes last_write_time can result in an access denied error
    try
    {
        t = fs::last_write_time(p);
    }
    catch (const fs::filesystem_error & e )
    {
        m_log->add_message("Error: Could not get last write time" + p.string(), "File Scanner");
    }

    return t;

}
