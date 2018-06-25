#ifndef FILEEXCEPTION_H
#define FILEEXCEPTION_H

#include <iostream>
#include <string>
#include <exception>

namespace Vessel {
    namespace Exception {

        class FileException : public std::exception
        {
            public:

                enum ErrorCode
                {
                    NoError = 0,
                    FileNotFound
                };

                FileException(ErrorCode e, const std::string& msg) : _msg(msg),_code(e) {}

                ErrorCode get_code() { return _code; }

                virtual const char* what() const noexcept override
                {
                    return _msg.c_str();
                }

            private:
                std::string _msg;
                ErrorCode _code;

        };

    }
}

#endif
