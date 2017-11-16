#include "local_db.hpp"
#include "file_iterator.hpp"

using std::cout;
using namespace Backup;

int main()
{

    Database::LocalDatabase* ldb = new Database::LocalDatabase("local.db");

    FileIterator* fi = new FileIterator( ldb->get_setting_str("home_folder") );
    fi->set_local_db(ldb);

    cout << fi->get_base_path() << std::endl;

    fi->scan();

    delete fi;

    return 0;
}
