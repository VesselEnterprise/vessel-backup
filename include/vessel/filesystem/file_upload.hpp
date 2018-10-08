#ifndef FILEUPLOAD_H
#define FILEUPLOAD_H

#include <iostream>
#include <string>
#include <memory>

#include <vessel/log/log.hpp>
#include <vessel/database/local_db.hpp>
#include <vessel/filesystem/file.hpp>

using namespace Vessel::Logging;
using namespace Vessel::Database;
using namespace Vessel::File;

namespace Vessel {
    namespace File {

        class FileUpload
        {

            public:
                FileUpload() {}
                FileUpload( const std::string& upload_key );
                FileUpload( unsigned int upload_id );

                unsigned int get_upload_id() const { return m_upload_id; }
                std::string get_upload_key() const { return m_upload_key; }
                std::string get_hash() const { return m_file_hash; }
                std::string get_signature() const { return m_signature; }
                int get_total_parts() const { return m_total_parts; }
                int get_offset() const { return m_offset; }
                int get_weight() const { return m_weight; }
                unsigned int get_chunk_size() const { return m_chunk_size; }
                unsigned long get_last_modified() const { return m_last_modified; }
                bool exists() const { return m_exists; }
                BackupFile get_file();
                std::shared_ptr<unsigned char> get_file_id() { return m_file_id; }
                void update_key(const std::string& upload_key);
                int get_current_part() const;
                void add_part(const FilePart& part) const;
                std::vector<UploadTagSet> get_part_tags() const;

                int get_error_count() const;
                int increment_error();


            private:
                BackupFile m_file;
                std::string m_upload_key;
                std::string m_file_hash;
                std::string m_signature;
                std::shared_ptr<unsigned char> m_file_id;
                unsigned int m_upload_id;
                int m_total_parts;
                int m_offset;
                int m_weight;
                unsigned int m_chunk_size;
                unsigned long m_last_modified;
                bool m_exists;

                void init();

        };

    }
}

#endif
