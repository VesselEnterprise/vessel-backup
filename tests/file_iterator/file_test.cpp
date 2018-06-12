#include "local_db.hpp"
#include "file_iterator.hpp"
#include "log.hpp"

using std::cout;
using namespace Vessel::File;
using namespace Vessel::Logging;
using namespace Vessel::Database;

int main()
{

    Log* log = new Log("output");
    LocalDatabase* ldb = &LocalDatabase::get_database();

    BackupDirectory bd( "test_files" );

    FileIterator* fi = new FileIterator(bd);

    cout << fi->get_base_path() << std::endl;

    fi->scan();
    ldb->clean();

    log->set_level( Vessel::Logging::info );
    log->add_message("This is some test text", "File Backup");

    delete fi;
    delete log;

    return 0;

}
