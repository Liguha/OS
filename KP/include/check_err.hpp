#ifndef CHECK_ERR_HPP
#define CHECK_ERR_HPP

#include <iostream>

#define CHECK_ERROR(expr, stream, act) \
    do \
    { \
        int res = (expr); \
        if (res == -1) \
        { \
            std::cerr << stream; \
            act; \
        } \
    } while (0)

#define CHECK_ERROR_PTHREAD(expr, stream) \
    do \
    { \
        int res = (expr); \
        if (res != 0) \
        { \
            std::cerr << stream; \
            return -1; \
        } \
    } while (0)

#endif