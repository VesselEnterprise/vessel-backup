#ifndef AZUREEXCEPTION_H
#define AZUREEXCEPTION_H

#include <iostream>
#include <string>
#include <exception>

namespace Vessel {
    namespace Exception {

        class AzureException : public std::exception
        {
            public:

                enum ErrorCode
                {
                    NoError = 0,
                    BadSignature,
                    FileNotInitialized,
                    InitFailed,
                    UploadFailed
                };

                AzureException(ErrorCode e, const std::string& msg) : _code(e)
                {
                    _msg = "AzureException: " + msg + " (ErrorCode: " + std::to_string((int)e) + ")";
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

