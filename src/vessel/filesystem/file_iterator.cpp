#include <vessel/filesystem/file_iterator.hpp>

using namespace Vessel::File;

FileIterator::FileIterator(const BackupDirectory& dir) : m_base_dir(dir), m_current_dir(dir)
{

    //Get Local Database Instance
    m_ldb = &Vessel::Database::LocalDatabase::get_database();

    //Update some setting(s)
    if ( m_ldb->get_setting_int("skip_period_dirs") > 0 )
        m_skip_dir_periods=true;
    else
        m_skip_dir_periods=false;

    //Init log
    m_log = &Log::get_log();

    //If Path is blank, try to find home folder
    if ( dir.get_path().empty() )
    {

        std::string tmp_path;

        #ifdef _WIN32
            /* This sometimes returns incorrect results in Windows systems
            std::string home_drive = std::getenv("HOMEDRIVE");
            std::string home_path = std::getenv("HOMEPATH");
            */
           tmp_path = std::getenv("USERPROFILE");
        #elif __unix
            tmp_path = std::getenv("HOME");
        #endif

        m_base_dir = BackupDirectory(tmp_path);
        m_current_dir = m_base_dir;

    }

    try
    {
        m_itr = fs::recursive_directory_iterator( m_base_dir.get_path() );
    }
    catch( const fs::filesystem_error& e )
    {
        m_log->add_message( "Scanner Error: " + std::string(e.what()), "File Scanner");
    }


}

std::string FileIterator::get_base_path()
{
    return m_base_dir.get_path();
}

void FileIterator::scan()
{

    if ( !fs::exists( m_current_dir.get_path() ) )
    {
        m_log->add_error("Error: Scan failed. " + m_current_dir.get_path() + " does not exist", "File Scanner");
        return;
    }
    else
    {
        //Always ensure directory has been added to DB
        m_current_dir.set_directory_id ( add_directory(m_current_dir) );
    }

    try
    {

        //Get Last Scan Time
        auto last_scan_time = m_ldb->get_setting_int("last_file_scan");

        //Insert 1000 records at a time to limit memory consumption of the page file
        int max_inserts = 1000;
        int total_inserts = 0;

        m_ldb->start_transaction(); //Speed up processing

        for ( ; m_itr != m_itr_end; ++m_itr )
        {

            const fs::path& p = m_itr->path();

            std::cout << "Scanning " << *m_itr << "..." << std::endl;

            try
            {

                if ( fs::is_directory(*m_itr) )
                {

                    //Check if this directory needs to be skipped
                    if ( this->skip_dir(*m_itr, m_itr.level() ) )
                    {
                        //m_log->add_message("Skipped directory exception: " + p.string(), "File Scanner");
                        std::cout << "Skipped directory exception: " << p.string() << '\n';
                        m_itr.no_push();
                        continue;
                    }

                    //Update current directory information
                    if ( (p.string() != m_current_dir.get_path()) )
                    {

                        //Reset to current directory
                        m_current_dir = BackupDirectory(p);

                        //Add to database
                        m_current_dir.set_directory_id( add_directory(m_current_dir) );

                    }

                    std::cout << "Added directory " << p.filename().string() << "..." << std::endl;

                }
                else if ( fs::is_regular_file(*m_itr) )
                {

                    //Create new file object
                    BackupFile bf(p);

                    //If Last Write Time is before last scan, skip
                    if ( bf.get_last_modified() <= last_scan_time )
                    {
                        //m_log->add_message("Skipped file. Last modified is before last scan time: " + p.string(), "File Scanner");
                        std::cout << "Skipped file. Last modified is before last scan time: " << p.string() << '\n';
                        continue;
                    }

                    //Skip empty files
                    if ( bf.get_file_size() == 0 )
                    {
                        //m_log->add_message("Skipped file. File is empty: " + p.string(), "File Scanner");
                        std::cout << "Skipped file. File is empty: " << p.string() << '\n';
                        continue;
                    }

                    //Check if we should ignore the file
                    if ( is_ignore_ext( bf.get_file_type() ) )
                    {
                        //m_log->add_message("Skipped file. File extension is excluded: " + p.string(), "File Scanner");
                        std::cout << "Skipped file. File extension is excluded: " << p.string() << '\n';
                        continue;
                    }

                    //Ensure that the current directory is still valid
                    if ( m_current_dir.get_path() != p.parent_path().string() )
                    {
                        m_current_dir = BackupDirectory(p.parent_path());
                        m_current_dir.set_directory_id ( add_directory(m_current_dir) );
                    }

                    //Set directory ID
                    //bf.set_directory_id( m_current_dir.directory_id );

                    //Add the file to the database
                    bf.set_directory_id( m_current_dir.get_directory_id() );
                    add_file(bf);

                    std::cout << "Added file " << p.filename().string() << " to database" << '\n';

                    //Start a new transaction
                    if ( ++total_inserts >= max_inserts )
                    {
                        m_ldb->end_transaction();
                        m_ldb->start_transaction();
                        total_inserts = 0;
                    }

                }
                else
                {
                    //m_log->add_message( p.string() + " exists , but is not a regular file or directory", "File Scanner");
                    std::cout << p.string() << " exists , but is not a regular file or directory" << '\n';
                }

            }
            catch ( const fs::filesystem_error& e )
            {
                m_log->add_error("Scan Error: " + std::string(e.what()), "File Scanner");
            }

        }

        //Always end the transaction
        m_ldb->end_transaction();

    }
    catch (const fs::filesystem_error& e)
    {
        //ERROR LOGGING HERE
        m_log->add_error("Scan Error: " + std::string(e.what()), "File Scanner");
        m_itr.no_push();
        ++m_itr;
        m_ldb->end_transaction();
        scan();
        return;

    }

    //Update file last scan time
    m_ldb->update_setting<int>("last_file_scan", (int)std::time(nullptr) );

}

bool FileIterator::skip_dir(const fs::path& p, int level)
{

    if ( m_skip_dir_periods && p.filename().string()[0] == '.' )
        return true;

    if ( is_ignore_dir(p, level) )
        return true;

    return false;

}

unsigned long FileIterator::get_last_write_t( const fs::path& p )
{

    unsigned long t=0;

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

bool FileIterator::is_ignore_dir(const boost::filesystem::path& p, int level )
{

    std::string folder_name = p.filename().string();

    sqlite3_stmt* stmt;
    std::string query = "SELECT ignore_id FROM backup_ignore_dir WHERE name=?1 AND level=?2";

    if ( sqlite3_prepare_v2(m_ldb->get_handle(), query.c_str(), -1, &stmt, NULL ) != SQLITE_OK )
        return false;

    sqlite3_bind_text(stmt, 1, folder_name.c_str(), folder_name.size(), 0 );
    sqlite3_bind_int(stmt, 2, level );

    int rc = sqlite3_step(stmt);

    //Cleanup
    sqlite3_finalize(stmt);

    if ( rc != SQLITE_ROW )
        return false;

    return true;

}

bool FileIterator::is_ignore_ext(const std::string& ext )
{

    sqlite3_stmt* stmt;
    std::string query = "SELECT extension FROM backup_ignore_ext WHERE extension=LOWER(?1)";

    if ( sqlite3_prepare_v2(m_ldb->get_handle(), query.c_str(), -1, &stmt, NULL ) != SQLITE_OK )
        return false;

    sqlite3_bind_text(stmt, 1, ext.c_str(), ext.size(), 0 );

    int rc = sqlite3_step(stmt);

    //Cleanup
    sqlite3_finalize(stmt);

    if ( rc != SQLITE_ROW )
        return false;

    return true;

}

bool FileIterator::add_file( const BackupFile& bf )
{

    sqlite3_stmt* stmt;
    std::string query = "REPLACE INTO backup_file (file_id,filename,file_ext,filesize,directory_id,last_modified) VALUES(?1,?2,?3,?4,?5,?6)";

    if ( sqlite3_prepare_v2(m_ldb->get_handle(), query.c_str(), -1, &stmt, NULL ) != SQLITE_OK )
    {
      m_log->add_error("Failed to add file to database: " + bf.get_file_name(), "File Scan");
      return false;
    }


    std::shared_ptr<unsigned char> file_id = bf.get_file_id();
    std::string file_name = bf.get_file_name();
    std::string file_type = bf.get_file_type();

    sqlite3_bind_blob(stmt, 1, file_id.get(), sizeof(file_id.get()), 0 );
    sqlite3_bind_text(stmt, 2, file_name.c_str(), file_name.size(), 0 );
    sqlite3_bind_text(stmt, 3, file_type.c_str(), file_type.size(), 0 );
    sqlite3_bind_int(stmt, 4, bf.get_file_size() );
    sqlite3_bind_int(stmt, 5, bf.get_directory_id() );
    sqlite3_bind_int(stmt, 6, bf.get_last_modified() );

    if ( sqlite3_step(stmt) != SQLITE_DONE )
    {
      m_log->add_error("Failed to add file to database: " + bf.get_file_name(), "File Scan");
      return false;
    }

    //Cleanup
    sqlite3_finalize(stmt);

    return true;

}

int FileIterator::add_directory( const BackupDirectory& bd )
{

    sqlite3_stmt* stmt;
    std::string query = "REPLACE INTO backup_directory (directory_id,directory_hash,path,filesize,last_modified) VALUES((SELECT directory_id FROM backup_directory WHERE directory_hash=?1),?1,?2,?3,?4)";

    if ( sqlite3_prepare_v2(m_ldb->get_handle(), query.c_str(), -1, &stmt, NULL ) != SQLITE_OK )
    {
      m_log->add_error("Failed to add directory to database: " + bd.get_dir_name(), "File Scan");
      return -1;
    }

    std::shared_ptr<unsigned char> unique_id = bd.get_unique_id_ptr();
    std::string dirpath = bd.get_canonical_path();

    sqlite3_bind_blob(stmt, 1, unique_id.get(), sizeof(unique_id.get()), 0 );
    sqlite3_bind_text(stmt, 2, dirpath.c_str(), dirpath.size(), 0 );
    sqlite3_bind_int(stmt, 3, bd.get_file_size() );
    sqlite3_bind_int(stmt, 4, bd.get_last_modified() );

    if ( sqlite3_step(stmt) != SQLITE_DONE)
    {
      m_log->add_error("Failed to add directory to database: " + bd.get_dir_name(), "File Scan");
      return -1;
    }

    //Cleanup
    sqlite3_finalize(stmt);

    return sqlite3_last_insert_rowid(m_ldb->get_handle());

}
