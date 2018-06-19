#ifndef QUEUEMGR_H
#define QUEUEMGR_H

#include <iostream>

#include <vessel/database/local_db.hpp>
#include <vessel/filesystem/file.hpp>

using namespace Vessel::Exception;
using namespace Vessel::Database;
using namespace Vessel::File;

class QueueManager
{

    public:
        QueueManager();
        ~QueueManager();

        BackupFile get_next_file();

    private:
        LocalDatabase* m_database;

        void push_file(unsigned char* file_id);
        void pop_file(unsigned char* file_id);

    protected:

        void rebuild_queue();
        void clear_queue();

};

#endif
