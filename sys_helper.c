#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <poll.h>
#include "co_inner_define.h"
#include "inner_fd.h"
#include "time_wheel.h"
#include "timer.h"
#include "co_mutex.h"

int get_msg_len(const struct msghdr *msg) {
    if(!msg) return -1;
    int len, i;
    len = i = 0;
    for(; i < msg->msg_iovlen; i++) {
        len += msg->msg_iov[i].iov_len;
    }
    return len;
}

int get_connect_error(int fd) {
    int error = 0;
    socklen_t error_len = sizeof(error);
    getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &error_len);
    return error;
}

int co_sleep(unsigned long mseconds) {
    if(!mseconds) return 0;
    int timerfd = open_timer();
    DBG_LOG("timerfd : %d", timerfd);
    if(timerfd < 0) { PANIC("timer fd open failed!"); }
    int res = wait_timer(timerfd, mseconds);

    DBG_LOG("timerfd : %d wakeup, return %d", timerfd, res);
    if (res < 0) return -1;
    close_timer(timerfd);
    return 0;
}

#define CO_HOST_NAME_KEY "co_gethostbyname"
#define CO_HOST_BUFFER_SIZE 4096
struct co_hostent_st {
    struct hostent host;
    char buffer[CO_HOST_BUFFER_SIZE];
    int herrno;
};

struct hostent *co_gethostbyname(const char *name) {
    struct co_hostent_st *hbuf = (struct co_hostent_st *)co_spec_get(CO_HOST_NAME_KEY);
    if(!hbuf) {
        struct co_hostent_st co_hbuf;
        co_hbuf.herrno = 0;
        co_spec_set(CO_HOST_NAME_KEY, &co_hbuf, sizeof(struct co_hostent_st));
        hbuf = (struct co_hostent_st *)co_spec_get(CO_HOST_NAME_KEY);
    }

    struct hostent *result = NULL;
    int ret, retry;
    ret = 0;
    retry = 3;
    while(retry) {
        ret = gethostbyname_r(name, &hbuf->host, hbuf->buffer, CO_HOST_BUFFER_SIZE, &result, &hbuf->herrno);
        switch(hbuf->herrno) {
            case HOST_NOT_FOUND:{
                ret = -1;
                retry = 0;
                ERR_LOG("host [%s] not find", name);
                break;
            }
            case NO_DATA: {
                ret = -1;
                retry = 0;
                ERR_LOG("request name [%s] is valid but does not have an IP address", name);
                break;
            }
            case NO_RECOVERY: {
                ret = -1;
                retry = 0;
                ERR_LOG("host [%s] meets a nonrecoverable error", name);
                break;
            }
            case TRY_AGAIN: {
                ERR_LOG("try again for [%s]", name);
                if(!--retry) ret = -1;
            }
            default: {
                retry = 0;
                break;
            }
        }
    }

    DBG_LOG("hook gethostbyname, return %d", ret);
    if(ret != 0) return NULL;
    return &hbuf->host;
}

extern int add_events(epoll_t *ep, int fd, int events);
extern int delete_events(epoll_t *ep, int fd, int events);

static const char* events_to_string(int events) {
    if ((events & EPOLLIN) && (events & EPOLLOUT)) return "READ & WRITE";
    else if (events & EPOLLIN) return "READ";
    else if (events & EPOLLOUT) return "WRITE";
    return "ERROR";
}

int co_poll(struct pollfd *fds, unsigned long int nfds, int timeout) {
    DBG_LOG("call co_poll, fds: %ld, nfds: %d, timeout: %d", fds, nfds, timeout);   
    inner_fd *ifd = NULL;
    epoll_t *ep = current_sched()->epoll;

    int use_timeout = 0;
    if (timeout != 0 && timeout != -1) use_timeout = 1;
    int timerfd = -1;

    for(int i = 0; i < nfds; i++){
        ifd = get_inner_fd(fds[i].fd);
        ifd->task = co_self();
        ifd->error = IENONE;
        //add interested list, note level triggered
        add_events(ep, ifd->fd, fds[i].events);
        DBG_LOG("co_poll wait socket: %d, %s event", fds[i].fd, events_to_string(fds[i].events));
        if(use_timeout == 0) wheel_update_element(ep->timer, &ifd->link, ifd->timeout);
    }

    if (use_timeout) {
        timerfd = open_timer();
        set_timer(timerfd, timeout);
        DBG_LOG("co_poll timer %d arms %d", timerfd, timeout);
    }

    co_io_yield();

    int timeout_event = 0;
    if (use_timeout) {
        if (is_timeout_timer(timerfd)) {
            timeout_event = 1;
            DBG_LOG("co_poll timer %d timeout", timerfd);
        }
        close_timer(timerfd);
        timerfd = -1;
    }

    int events = 0;
    int j = 0;
    for (int i = 0; i < nfds; i++) {
        events = 0;
        ifd = get_inner_fd(fds[i].fd);
        //remove interested list, any task's sockets wake up
        if(!use_timeout) wheel_delete_element(&ifd->link);
        delete_events(ep, ifd->fd, 0);
        fds[i].revents = events;

        if (IENONE == ifd->error) {
            if (use_timeout && timeout_event) {
                ifd->error = IETIMEOUT;
            }
            continue;
        }

        if(ifd->error & IEREAD) {
            events |= EPOLLIN;
        }

        if(ifd->error & IEWRITE) {
            events |= EPOLLOUT;
        }

        if(ifd->error & IEERR) {
            events |= EPOLLERR;
        }

        if(ifd->error & IERDHUP) {
            events |= EPOLLRDHUP;
        }

        if(ifd->error & IEHUP) {
            events |= EPOLLHUP;
        }

        fds[i].revents = events;
        DBG_LOG("co_poll return socket: %d, %d|%d, %s event", fds[i].fd, ifd->error, fds[i].revents, events_to_string(fds[i].revents));
        fds[j++] = fds[i];
    }

    return j;
}

