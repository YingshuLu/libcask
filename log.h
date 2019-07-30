#ifndef LIBCASK_LOG_H_
#define LIBCASK_LOG_H_

/*log APIs are thread-safty and coroutine-safty
 *log print automatically detect current environment 
 */

#include <stdio.h>
#include <stdarg.h>

#define LOG_FD_STDIN  fileno(stdin)
#define LOG_FD_STDOUT fileno(stdout)
#define LOG_FD_STDERR fileno(stderr)

enum {
    LOG_DEBUG = 0,
    LOG_INFO,
    LOG_ERROR    
};

int log_printf(int fd, int level,  const char *fmt, ...);
#define LOG_OUTPUT(fd, level, fmt, ...) log_printf(fd, level, fmt" <%s>@<%s:%d>\n", ##__VA_ARGS__, __func__, basename(__FILE__), __LINE__)

#endif
