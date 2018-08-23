#ifndef VESSELEXCEPTION_H
#define VESSELEXCEPTION_H

#include <iostream>
#include <string>
#include <exception>

namespace Vessel {
namespace Exception {

        class VesselException : public std::exception
        {
            public:

                enum ErrorCode
                {
                    NoError = 0,
                    NotInstalled,
                    ProviderError
                };

                VesselException(ErrorCode e, const std::string& msg) : _code(e)
                {
                    _msg = "VesselException: " + msg + " (ErrorCode: " + std::to_string((int)e) + ")";
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


