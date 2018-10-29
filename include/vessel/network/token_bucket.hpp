#ifndef TOKENBUCKET_H
#define TOKENBUCKET_H

#include <iostream>
#include <string>
#include <ctime>

namespace Vessel
{
    namespace Networking
    {
        class TokenBucket
        {
            public:
                TokenBucket(size_t bytes_per_second, size_t total_bytes)
                {
                    _bytes_per_second = bytes_per_second;
                    _total_bytes = total_bytes;
                    _bytes_available = 0;
                    _last_transfer = 0;
                    _bytes_transferred = 0;
                    _transfer_start = (long)std::time(nullptr);
                }

                void transfer(size_t bytes)
                {
                    _bytes_available -= bytes;
                    _bytes_transferred += bytes;
                    _last_transfer = (long)std::time(nullptr);
                    refill_bucket();
                }

                size_t bytes_transferred()
                {
                    return _bytes_transferred;
                }

                size_t total_bytes()
                {
                    return _total_bytes;
                }

                size_t max_transfer_speed()
                {
                    return _bytes_per_second;
                }

                double transfer_rate()
                {
                    long seconds_elapsed = _last_transfer - _transfer_start;
                    if ( seconds_elapsed == 0 ) seconds_elapsed = 1;

                    return ( (_bytes_transferred*1.0) / (seconds_elapsed*1.0) );
                }

            private:
                size_t _bytes_per_second;
                size_t _bytes_available;
                size_t _bytes_transferred;
                size_t _total_bytes;
                long _last_transfer;
                long _transfer_start;

                void refill_bucket()
                {
                    _bytes_available += (_bytes_per_second - _bytes_available);
                }

        };
    }
}

#endif

