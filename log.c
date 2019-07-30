#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>
#include "log.h"
#include "cid.h"
#include "co_inner_define.h"

#define LOG_BUFFER_SIZE (1024 * 4)
const char *_log_levels[] = {"D", "I", "E"};
const char *_log_fmt = "[%s]>%s<[%d-%d-%lu] %s";
#define _log_fmt_tail " <%s>@<%s:%d>\n", ##__VA_ARGS__, __func__, basename(__FILE__), __LINE__

#define TIME_STR_LEN 25
__thread char _time_strf[20] = {0};
__thread time_t _old_time = 0;
__thread char _log_buf[LOG_BUFFER_SIZE] = {0};

const char *get_time_str() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    if(_old_time != tv.tv_sec) {
        strftime(_time_strf, TIME_STR_LEN, "%T", localtime(&tv.tv_sec));
        _old_time = tv.tv_sec;
    }
    snprintf(_time_strf + 8, TIME_STR_LEN - 8, ".%ld", tv.tv_usec);
    return _time_strf;
}

int log_printf(int fd, int level, const char *fmt, ...) {
    char *buf = NULL;
    if (co_self()) {
        #define CO_LOG_KEY "co_log_key"
        buf = (char *)co_spec_get(CO_LOG_KEY);
        if(!buf) {
            co_spec_set(CO_LOG_KEY, _log_buf, LOG_BUFFER_SIZE);
            buf = (char *)co_spec_get(CO_LOG_KEY);
        }
    }
    else {
        buf = _log_buf;
    }

	char new_fmt[1024] = {0};
    int len = snprintf(new_fmt, 1024, _log_fmt, _log_levels[level], get_time_str(), getpid(), tid(), getcid(), fmt);

    if(len < 0) {
        dprintf(LOG_FD_STDERR, "[log_porintf] error: snprintf return %d\n", len);
        abort();
    }
    new_fmt[len] = '\0';

    va_list args;
    va_start(args, fmt);
    len = vsnprintf(buf, LOG_BUFFER_SIZE, new_fmt, args);
    va_end(args);

	int n = write(fd, buf, len);
	if (n <= 0) {
		dprintf(LOG_FD_STDERR, "[log_printf] error: write fd[%d] return: %d, cause: %s\n", fd, n, strerror(errno));
		return -1;
	}
	return 0;
}
