#ifndef STACK_TYPES_H_
#define STACK_TYPES_H_

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include "log.h"

#define MAX_ERROR_BUFFER_SIZE 1024

#define GLOBAL
#define LIKELY(con) (__builtin_expect(!!(con), 1))
#define UNLIKELY(con) (__builtin_expect(!!(con), 0))

//log 
#ifndef CO_DEBUG
#define DBG_LOG //
#else
#define DBG_LOG(fmt, ...) LOG_OUTPUT(LOG_FD_STDOUT, LOG_DEBUG, fmt, ##__VA_ARGS__)
#endif

#define INF_LOG(fmt, ...) LOG_OUTPUT(LOG_FD_STDOUT, LOG_INFO, fmt, ##__VA_ARGS__)
#define ERR_LOG(fmt, ...) LOG_OUTPUT(LOG_FD_STDERR, LOG_ERROR, fmt, ##__VA_ARGS__)

#define dperror(ret) do {\
    char _error_buffer[512] = {0};\
    strerror_r(errno, _error_buffer, MAX_ERROR_BUFFER_SIZE);\
    ERR_LOG("return value: %d, errno: %d, %s", ret, errno, _error_buffer);\
}while(0)

#define dfatal(ret) do{\
    dperror(ret);\
    exit(-1);\
}while(0)

#define PANIC(string) do {\
    ERR_LOG("[PANIC] message: \"%s\"", string);\
    exit(-1);\
}while(0)

#define  MY_ASSERT(cond, comment) do { if(UNLIKELY(!(cond))) { PANIC(comment); } } while(0)

#endif
