#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <iostream>
#include <string>
#include <exception>

class AwsException : public std::exception
{

    enum ErrorCodes
    {
        NoError = 0,
        InitFailed,
        UploadFailed,
        XmlParseError
    };

    std::string _msg;

    public:

        AwsException(const std::string& msg) : _msg(msg){}

    virtual const char* what() const noexcept override
    {
        return _msg.c_str();
    }

};

#endif

