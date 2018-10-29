#ifndef QUEUEMGR_H
#define QUEUEMGR_H

#include <iostream>
#include <memory>

#include <vessel/database/local_db.hpp>
#include <vessel/filesystem/file.hpp>
#include <vessel/filesystem/file_upload.hpp>

using namespace Vessel;
using namespace Vessel::Exception;
using namespace Vessel::Database;
using namespace Vessel::File;

#define QUEUE_MAX_ROWS 100

namespace Vessel
{
  class QueueManager
  {

      public:
          QueueManager();

          BackupFile get_next_file();
          FileUpload get_next_upload();
          int get_total_pending();
          void rebuild_queue();
          void pop_file(std::shared_ptr<unsigned char> file_id);

      private:
          LocalDatabase* m_database;

          void push_file(std::shared_ptr<unsigned char> file_id);
          void apply_weights();

      protected:
          void clear_queue();

  };
}

#endif
