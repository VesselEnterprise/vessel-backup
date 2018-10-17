#include <vessel/vessel/stat_manager.hpp>

StatManager::StatManager()
{
  m_database = &LocalDatabase::get_database();
}

void StatManager::build_stats()
{

  //Create a map of stats and save to database
  std::map<std::string,int> stat_map;
  stat_map.insert( std::pair<std::string,int>("total_files", get_total_files() ) );
  stat_map.insert( std::pair<std::string,int>("total_backed_up", get_total_backed_up() ) );
  stat_map.insert( std::pair<std::string,int>("total_dirs", get_total_dirs() ) );
  stat_map.insert( std::pair<std::string,int>("total_errors", get_total_errors() ) );

  for ( std::map<std::string,int>::iterator it=stat_map.begin(); it!=stat_map.end(); ++it )
  {
    update_stat( it->first, it->second );
  }

}

void StatManager::update_stat(const std::string& name, int value)
{
  sqlite3_stmt* stmt;
  std::string query = "REPLACE INTO backup_stat (name,value) VALUES(?1,?2)";

  if ( sqlite3_prepare_v2(m_database->get_handle(), query.c_str(), -1, &stmt, NULL ) != SQLITE_OK )
  {
    Log::get_log().add_error("Failed to update stat: " + name, "Stats");
    return;
  }

  sqlite3_bind_text(stmt, 1, name.c_str(), -1, 0);
  sqlite3_bind_int(stmt, 2, value);

  if ( sqlite3_step(stmt) != SQLITE_DONE )
  {
    Log::get_log().add_error("Failed to update stat: " + name, "Stats");
  }

}

int StatManager::get_stat(const std::string& name)
{

  int value = -1;

  //Get Total File Count
  sqlite3_stmt* stmt;
  std::string query = "SELECT value FROM backup_stat WHERE name=?1";

  if ( sqlite3_prepare_v2(m_database->get_handle(), query.c_str(), -1, &stmt, NULL ) != SQLITE_OK )
  {
    return value;
  }

  sqlite3_bind_text(stmt, 1, name.c_str(), -1, 0);
  if ( sqlite3_step(stmt) == SQLITE_ROW )
  {
    value = sqlite3_column_int(stmt, 0);
  }

  //Cleanup
  sqlite3_finalize(stmt);

  return value;

}

int StatManager::get_total_files()
{
  int value = -1;
  sqlite3_stmt* stmt;
  std::string query = "SELECT COUNT() FROM backup_file";

  if ( sqlite3_prepare_v2(m_database->get_handle(), query.c_str(), -1, &stmt, NULL ) != SQLITE_OK )
  {
    return value;
  }

  if ( sqlite3_step(stmt) == SQLITE_ROW )
  {
    value = sqlite3_column_int(stmt, 0);
  }

  //Cleanup
  sqlite3_finalize(stmt);

  return value;

}

int StatManager::get_total_backed_up()
{
  int value = -1;
  sqlite3_stmt* stmt;
  std::string query = "SELECT COUNT() FROM backup_file WHERE last_backup_time <> 0";

  if ( sqlite3_prepare_v2(m_database->get_handle(), query.c_str(), -1, &stmt, NULL ) != SQLITE_OK )
  {
    return value;
  }

  if ( sqlite3_step(stmt) == SQLITE_ROW )
  {
    value = sqlite3_column_int(stmt, 0);
  }

  //Cleanup
  sqlite3_finalize(stmt);

  return value;

}

int StatManager::get_total_dirs()
{
  int value = -1;
  sqlite3_stmt* stmt;
  std::string query = "SELECT COUNT() FROM backup_directory";

  if ( sqlite3_prepare_v2(m_database->get_handle(), query.c_str(), -1, &stmt, NULL ) != SQLITE_OK )
  {
    return value;
  }

  if ( sqlite3_step(stmt) == SQLITE_ROW )
  {
    value = sqlite3_column_int(stmt, 0);
  }

  //Cleanup
  sqlite3_finalize(stmt);

  return value;

}

int StatManager::get_total_errors()
{
  int value = -1;
  sqlite3_stmt* stmt;
  std::string query = "SELECT COUNT() FROM backup_log WHERE error=1";

  if ( sqlite3_prepare_v2(m_database->get_handle(), query.c_str(), -1, &stmt, NULL ) != SQLITE_OK )
  {
    return value;
  }

  if ( sqlite3_step(stmt) == SQLITE_ROW )
  {
    value = sqlite3_column_int(stmt, 0);
  }

  //Cleanup
  sqlite3_finalize(stmt);

  return value;

}
