#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <string>
#include <sstream>
#include <vector>

namespace Vessel {
    namespace Networking {

        class HttpRequest
        {
            public:

                HttpRequest(){};
                ~HttpRequest(){};

                void set_url ( const std::string& str );
                void set_method( const std::string& str);
                void add_header( const std::string& str);
                void set_content_type( const std::string& str);
                void set_body( const std::string& str);
                void set_auth_header( const std::string& str);
                void accept(const std::string& str);

                std::string get_url() const;
                std::string get_method() const;
                std::vector<std::string> get_headers() const;
                std::string get_content_type() const;
                std::string get_body() const;
                std::string get_accept() const;
                std::string get_auth() const;
                size_t get_body_length() const;

            private:
                std::string m_url;
                std::string m_accept;
                std::string m_hostname;
                std::string m_authorization;
                std::string m_http_method;
                std::string m_content_type;
                std::string m_body;
                std::vector<std::string> m_headers;


        };

    }
}

#endif // HTTPREQUEST_H
