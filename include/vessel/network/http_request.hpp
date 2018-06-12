#ifndef HTTP_H
#define HTTP_H

#include <string>
#include <vector>

namespace Vessel {
    namespace Networking {

        class HttpRequest
        {
            public:

                HttpRequest(){};
                ~HttpRequest(){};

                void set_uri ( const std::string& str );

                void set_method( const std::string& str);

                void add_header( const std::string& str);

                void set_content_type( const std::string& str);
                void set_body( const std::string& str);

                std::string get_uri();

                std::string get_method();

                std::vector<std::string> get_headers();

                std::string get_content_type();

                std::string get_body();

                size_t get_body_length();

            private:
                std::string m_uri;
                std::string m_http_method;
                std::string m_content_type;
                std::string m_body;
                std::vector<std::string> m_headers;


        };

    }
}

#endif // HTTP_H
