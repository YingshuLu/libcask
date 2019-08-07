#ifndef LIBCASK_TIMER_H_
#define LIBCASK_TIMER_H_

#include <time.h>

int open_timer();

int set_timer(int timerfd, time_t ms);

int cancel_timer(int timerfd);

int handle_timer(int timerfd);

int wait_timer(int timefd, time_t ms);

int is_timeout_timer(int timerfd);

int close_timer(int timerfd);

#endif
