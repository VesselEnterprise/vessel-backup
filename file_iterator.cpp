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

    }

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

    cout << "Check 1" << '\n';

    try
    {

        if ( !fs::exists(m_itr->path()) ) {
            cout << "Error: Could not scan " <<  m_itr->path().string() << ". Directory does not exist" << '\n';
            return;
        }

    }
    catch(const fs::filesystem_error& e)
    {

    }

    cout << "Check 2" << '\n';

    try
    {

        for ( ; m_itr != m_itr_end; ++m_itr )
        {

            cout << "Testing: " << *m_itr << '\n';
            cout << "Level: " << m_itr.level() << '\n';
            cout << "Filename:" << m_itr->path().filename() << '\n';
            cout << "Stem:" << m_itr->path().stem() << '\n';
            cout << "Extension:" << m_itr->path().extension() << '\n';

            if ( fs::is_regular_file(*m_itr) )
                cout << *m_itr << " size is " << fs::file_size(*m_itr) << '\n';

            else if ( fs::is_directory(*m_itr) )
            {

                cout << *m_itr << " is a directory" << '\n';

                if ( this->skip_dir(m_itr->path().filename().string(), m_itr.level() ) )
                {
                    cout << "Skipped directory exception: " << *m_itr << '\n';
                    m_itr.no_push();
                    system("PAUSE");
                }


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
        system("PAUSE");
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

    std::cout << "Skip periods: " << m_skip_dir_periods << std::endl;

    if ( m_skip_dir_periods && path[0] == '.' )
        return true;

    if ( m_ldb->is_ignore_dir(path, level) )
        return true;

    return false;

}
