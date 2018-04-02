#include "local_db.hpp"
#include "file_iterator.hpp"
#include "log.hpp"

using std::cout;
using namespace Backup::File;
using namespace Backup::Logging;
using namespace Backup::Database;

int main()
{

    Log* log = new Log("output");
    LocalDatabase* ldb = &LocalDatabase::get_database();

    BackupDirectory bd( "test_files" );

    FileIterator* fi = new FileIterator(bd);

    cout << fi->get_base_path() << std::endl;

    fi->scan();
    ldb->clean();

    log->set_level( Backup::Logging::info );
    log->add_message("This is some test text", "File Backup");

    delete fi;
    delete log;

    return 0;

}
