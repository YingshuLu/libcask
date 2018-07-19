#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <poll.h>
#include "co_inner_define.h"
#include "inner_fd.h"
#include "time_wheel.h"
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

unsigned int co_sleep(unsigned int seconds) {
    if(!seconds) return 0;
    seconds--;
    time_wheel_t* tw = current_sched()->epoll->timer;
    if(!tw) { PANIC("timer is NULL"); }

    inner_fd *ifd = new_inner_fd(-1);
    ifd->task = co_self();
    ifd->timeout = seconds;
 
    wheel_update_element(tw, &ifd->link, ifd->timeout);
    co_sys_yield();
    wheel_delete_element(&ifd->link);
 
    DBG_LOG("sleep timeout, wakeup");
    list_delete(&ifd->link);
    free(ifd);

    return 0;
}

#define CO_HOST_NAME_KEY "co_gethostbyname"
#define CO_HOST_BUFFER_SIZE 2048
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

int events_poll_to_epoll(int events) {
    int epev = 0;
    
    if(events & POLLIN) {
        epev |= EPOLLIN;
    }

    if(events & POLLOUT) {
        epev |= EPOLLOUT;
    }

    if(events & POLLRDHUP) {
        epev |= EPOLLRDHUP;
    }

    if(events & POLLHUP) {
        epev |= EPOLLHUP;
    }

    if(events | EPOLLERR) {
        epev |= EPOLLERR;
    }
    return epev;

}

extern int add_events(epoll_t *ep, int fd, int events);
extern int delete_events(epoll_t *ep, int fd, int events);

int co_poll(struct pollfd *fds, unsigned long int nfds, int timeout) {
    if(!nfds) return 0;

    inner_fd *ifd = NULL;
    int epevs = 0;
    epoll_t *ep = current_sched()->epoll;
    for(int i = 0; i < nfds; i++){
        ifd = get_inner_fd(fds[i].fd);
        if(ifd) {
            ifd->task = co_self();
            epevs = events_poll_to_epoll(fds[i].events);
            //sucess
            if(!add_events(ep, ifd->fd, epevs)) {
               break; 
            }
        }
    }

    wheel_update_element(ep->timer, &ifd->link, ifd->timeout);
    co_io_yield();
    wheel_delete_element(&ifd->link);

    delete_events(ep, ifd->fd, 0);

    fds[0].fd = ifd->fd;
    int events = 0;

    if(ifd->error & IEREAD) {
        events |= POLLIN;
    }

    if(ifd->error & IEWRITE) {
        events |= POLLOUT;
    }

    if(ifd->error & IEERR) {
        events |= POLLERR;
    }

    if(ifd->error & IERDHUP) {
        events |= POLLRDHUP;
    }

    if(ifd->error & IEHUP) {
        events |= POLLHUP;
    }
    fds[0].revents = events;

    return 1;
}

