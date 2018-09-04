#ifndef AWSEXCEPTION_H
#define AWSEXCEPTION_H

#include <iostream>
#include <string>
#include <exception>

namespace Vessel {
    namespace Exception {

        class AwsException : public std::exception
        {
            public:

                enum ErrorCode
                {
                    NoError = 0,
                    InitFailed,
                    UploadFailed,
                    BadResponse,
                    XmlParseError,
                    InvalidCredentials,
                    BadSigningKey,
                    BadUploadId
                };

                AwsException(ErrorCode e, const std::string& msg) : _code(e)
                {
                    _msg = "AwsException: " + msg + " (ErrorCode: " + std::to_string((int)e) + ")";
                }

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

