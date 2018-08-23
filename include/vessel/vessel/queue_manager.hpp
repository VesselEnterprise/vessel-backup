#ifndef QUEUEMGR_H
#define QUEUEMGR_H

#include <iostream>

#include <vessel/database/local_db.hpp>
#include <vessel/filesystem/file.hpp>

using namespace Vessel::Exception;
using namespace Vessel::Database;
using namespace Vessel::File;

#define QUEUE_MAX_ROWS 100

class QueueManager
{

    public:
        QueueManager();
        ~QueueManager();

        BackupFile get_next_file();
        int get_total_pending();
        void rebuild_queue();
        void pop_file(unsigned char* file_id);

    private:
        LocalDatabase* m_database;

        void push_file(unsigned char* file_id);
        void apply_weights();

    protected:
        void clear_queue();

};

#endif
