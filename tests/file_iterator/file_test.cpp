#include "local_db.hpp"
#include "file_iterator.hpp"
#include "log.hpp"

using std::cout;
using namespace Backup;

int main()
{

    Logging::Log* log = new Logging::Log("output");
    Database::LocalDatabase* ldb = new Database::LocalDatabase("local.db");

    FileIterator* fi = new FileIterator( ldb->get_setting_str("home_folder") );
    fi->set_local_db(ldb);

    cout << fi->get_base_path() << std::endl;

    fi->scan();
    ldb->clean();

    log->set_level( Logging::info );
    log->add_message("This is some test text", "File Backup");

    delete fi;
    delete log;

    return 0;

}
