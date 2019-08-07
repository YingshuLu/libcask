#include <fcntl.h>
#include <unistd.h>
#include <sys/timerfd.h>
#include "timer.h"
#include "inner_fd.h"
#include "task.h"
#include "task_sched.h"
#include "event.h"

static int __set_timer(int timerfd, time_t ms) {
    if (timerfd <= 0) return -1;
    struct itimerspec ltv;
    ltv.it_value.tv_sec = (ms / 1000);
    ltv.it_value.tv_nsec = ((ms % 1000) * 1000000); // alarm after ms
    ltv.it_interval.tv_sec = 0;
    ltv.it_interval.tv_nsec = 0;
    return timerfd_settime(timerfd, 0, &ltv, NULL);
}

int open_timer() {
    int timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK|TFD_CLOEXEC);
    if (timerfd <= 0) return -1;
    inner_fd* ifd = get_inner_fd(timerfd);
    if (!ifd) ifd = new_inner_fd(timerfd);
    ifd->flags |= O_NONBLOCK;
    ifd->timeout = -1;
    ifd->error = IENONE;
    return timerfd;
}

int set_timer(int timerfd, time_t ms) {
    int res = __set_timer(timerfd, ms);
    if (res < 0) return res;
    inner_fd *ifd = get_inner_fd(timerfd);
    ifd->task = current_task();
    ifd->error = IENONE;
    epoll_t *ep = current_sched()->epoll;
    add_events(ep, timerfd, EPOLLIN | EPOLLET);
    return 0;   
}

int cancel_timer(int timerfd) {
    __set_timer(timerfd, 0);
    epoll_t *ep = current_sched()->epoll;
    delete_events(ep, timerfd, 0);
    return 0;
}

int handle_timer(int timerfd) {
    char buffer[8];
    int res = read(timerfd, buffer, 8);
    return res;
}

int wait_timer(int timerfd, time_t ms) {
    int res = set_timer(timerfd, ms);
    if (res != 0) return res;
    res = handle_timer(timerfd);
    return res;
}

int is_timeout_timer(int timerfd) {
    if (timerfd <= 0) return 0;
    inner_fd *ifd = get_inner_fd(timerfd);
    if (ifd->error & IEREAD) {
        return 1;
    }   
    return 0;
}

int close_timer(int timerfd) {
    return close(timerfd);
}
