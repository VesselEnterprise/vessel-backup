#include <iostream>
#include <string>
#include <fstream>

#include "local_db.hpp"

/**
 ** Read and parses the mime.types file
*/
int main(int argc, char* argv[] )
{

    std::ifstream infile;
    infile.open( "/etc/mime.types", std::ios::in );

    if ( !infile.is_open() )
    {
        std::cout << "Error: Failed to open file";
        return 1;
    }

    //Get Local Database Handle
    Vessel::Database::LocalDatabase* ldb = &Vessel::Database::LocalDatabase::get_database();
    sqlite3* db = ldb->get_handle();

    std::string tmp_line;

    while ( std::getline( infile, tmp_line, '\n') )
    {
        if ( tmp_line.empty() || tmp_line[0] == '#' ) //Ignore empty lines and comments
            continue;

        std::string mime_type;
        std::string delim = "\t";
        std::vector<std::string> exts;
        int pos = tmp_line.find(delim);

        if ( pos == std::string::npos )
        {
            mime_type = tmp_line;
            tmp_line.clear();
        }

        while ( (pos = tmp_line.find(delim) ) != std::string::npos )
        {

            if ( mime_type.empty() )
                mime_type = tmp_line.substr(0, pos);

            tmp_line.erase(0, pos + delim.length() );

        }

        if ( !tmp_line.empty() )
        {
            //Extensions are separated by spaces
            delim = " ";
            pos = tmp_line.find(delim);
            if ( pos == std::string::npos )
            {
                exts.push_back(tmp_line);
            }
            else
            {
                while ( (pos = tmp_line.find(delim)) != std::string::npos )
                {
                    exts.push_back( tmp_line.substr(0, pos) );
                    tmp_line.erase(0, pos + delim.length() );
                }
            }
        }

        std::cout << mime_type << " => " << exts.size() << "\n";

        //Insert MIME types into database
        for ( int i=0; i < exts.size(); i++ )
        {

            sqlite3_stmt* stmt;
            std::string query = "INSERT OR REPLACE INTO backup_mime_type VALUES (?1,?2)";

            if ( sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, NULL ) != SQLITE_OK )
                return false;

            sqlite3_bind_text(stmt, 1, mime_type.c_str(), mime_type.size(), 0 );
            sqlite3_bind_text(stmt, 2, exts[i].c_str(), exts[i].size(), 0 );

            int rc = sqlite3_step(stmt);

            //Cleanup
            sqlite3_finalize(stmt);

        }

    }

    infile.close();

    //delete ldb;

    return 0;

}
