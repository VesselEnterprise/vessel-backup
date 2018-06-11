#ifndef HTTPSTREAM_H
#define HTTPSTREAM_H

#include <iostream>
#include <string>

namespace Backup {
    namespace Networking {

        class HttpRequestStream
        {

            public:
                HttpRequestStream(){}
                HttpRequestStream(const std::string& str) { m_string = str; }
                ~HttpRequestStream(){}

                std::string str() const
                {
                    return m_string;
                }

                std::string& str()
                {
                    return m_string;
                }

                friend HttpRequestStream& operator<< (HttpRequestStream& rs, const std::string & str )
                {
                    rs.append(str);
                    return rs;
                }

                friend HttpRequestStream& operator<< (HttpRequestStream& rs, int num )
                {
                    rs.append(std::to_string(num));
                    return rs;
                }

                friend std::ostream& operator << (std::ostream& os, const std::string& str)
                {
                    os << str;
                    return os;

                }
                friend std::ostream& operator << (std::ostream& os, HttpRequestStream& rs)
                {
                    os << rs.str();
                    return os;

                }
                friend std::istream& operator >> (std::istream& is, std::string& str)
                {
                    is >> str;
                    return is;

                }

                void append( const std::string& str ) { m_string.append(str); }
                void append( int num ) { m_string.append(std::to_string(num)); }

            private:
                std::string m_string;

        };

    }

}

#endif
