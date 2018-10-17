#ifndef VESSELSTAT_H
#define VESSELSTAT_H

#include <iostream>
#include <string>
#include <map>

#include <vessel/database/local_db.hpp>

using namespace Vessel::Database;
using namespace Vessel::Logging;

namespace Vessel
{
  class StatManager
  {

    public:
      StatManager();

      void build_stats();
      int get_stat(const std::string& name);
      void update_stat(const std::string& name, int value);

    private:
      LocalDatabase* m_database;

      int get_total_files();
      int get_total_backed_up();
      int get_total_dirs();
      int get_total_errors();

  };
}


#endif //VESSELSTAT_H
