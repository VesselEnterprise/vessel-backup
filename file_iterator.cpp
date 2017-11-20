#include "file_iterator.hpp"

using std::cout;
using namespace Backup;
namespace fs = boost::filesystem;

FileIterator::FileIterator(const std::string& path) : m_base_path(path), m_skip_dir_periods(false)
{

    try
    {
        m_itr = fs::recursive_directory_iterator(m_base_path);
    }
    catch( const fs::filesystem_error& e )
    {
        cout << e.what() << '\n';
    }

    m_current_dir.path = path;

}

FileIterator::~FileIterator()
{

}

std::string FileIterator::get_base_path()
{
    return m_base_path.string();
}

void FileIterator::scan()
{

    if ( !fs::exists(m_current_dir.path) )
    {
        cout << "Error: Scan failed. " << m_current_dir.path << " does not exist" << '\n';
        return;
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

            if ( fs::is_regular_file(*m_itr) )
            {
                Types::file_data fd;
                fd.filename = m_itr->path().filename().string();
                fd.file_ext = m_itr->path().extension().string();
                fd.filesize = fs::file_size(*m_itr);
                fd.parent_path = m_itr->path().parent_path().string();
                fd.directory_id = m_current_dir.directory_id;

                //Sometimes last_write_time can result in an access denied error
                try
                {
                    fd.last_modified = fs::last_write_time(*m_itr);
                }
                catch (const fs::filesystem_error & e ) { cout << "Error: Could not get last write time" << '\n'; }

                m_ldb->add_file( &fd );

                cout << "Last error: " << m_ldb->get_last_err() << '\n';

            }
            else if ( fs::is_directory(*m_itr) )
            {

                if ( this->skip_dir(m_itr->path().filename().string(), m_itr.level() ) )
                {
                    cout << "Skipped directory exception: " << *m_itr << '\n';
                    m_itr.no_push();
                    //system("PAUSE");
                }

                //Update current directory information
                if ( m_itr->path().string() != m_current_dir.path )
                {
                    m_current_dir.folder_name = m_itr->path().filename().string();
                    m_current_dir.path = m_itr->path().string();

                    //Sometimes last_write_time can result in an access denied error
                    try
                    {
                        m_current_dir.last_modified = fs::last_write_time( *m_itr );
                    }
                    catch (const fs::filesystem_error & e ) { cout << "Error: Could not get last write time" << '\n'; }

                    m_ldb->add_directory(&m_current_dir);

                    cout << "Directory ID: " << m_current_dir.directory_id << '\n';

                }

                cout << *m_itr << " is a directory" << '\n';

            }
            else
                cout << *m_itr << " exists, but is not a regular file or directory\n";

        }

    }
    catch (const fs::filesystem_error& ex)
    {
        //ERROR LOGGING HERE
        cout << ex.what() << '\n';
        ++m_itr;
        //system("PAUSE");
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

bool FileIterator::skip_dir(const std::string& path, int level)
{

    if ( m_skip_dir_periods && path[0] == '.' )
        return true;

    if ( m_ldb->is_ignore_dir(path, level) )
        return true;

    return false;

}
