#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <iostream>
#include <string>
#include <exception>

class AwsException : public std::exception
{
    public:

        typedef enum ErrorCode
        {
            NoError = 0,
            InitFailed,
            UploadFailed,
            BadResponse,
            XmlParseError
        };

        AwsException(ErrorCode e, const std::string& msg) : _msg(msg),_code(e) {}

        ErrorCode get_code() { return _code; }

        virtual const char* what() const noexcept override
        {
            return _msg.c_str();
        }

    private:
        std::string _msg;
        ErrorCode _code;

};

#endif

