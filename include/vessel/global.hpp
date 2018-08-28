#ifndef GLOBAL_H
#define GLOBAL_H

    inline char PATH_SEPARATOR()
    {
        #ifdef _WIN32
            return '\\';
        #else
            return '/';
        #endif
    }

#endif
