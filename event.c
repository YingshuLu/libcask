#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include "co_inner_define.h"
#include "inner_fd.h"
#include "event.h"
#include "time_wheel.h"
#include "list.h"

void _timeout_task(void *ip, void *op) {
    co_enable_hook();
    epoll_t *ep = (epoll_t *)ip;
    time_wheel_t *tw = ep->timer;
    inner_fd *ifd = NULL;
    const int buf_size = 8;
    char *buf[8] = {0};
    int ret = 0;

    while(1) {
        ret = read(tw->fd, buf, buf_size);
        //error, disable timer
        if(ret != buf_size) {
            DBG_LOG("timer error, now uninstall timer mod");
            close(tw->fd);
            dperror(ret);
            break;
        }
        
        wheel_rotate(ep->timer);
        list_t *timeout_list = wheel_timeout_list(ep->timer);
        list_t *ls = list_next(timeout_list);
        list_t *next = NULL;
        while(!list_empty(ls)) {
            ifd = list_to_inner_fd(ls);
            ifd->error |= IETIMEOUT;
            co_resume(ifd->task);
            next = list_next(ls);
            wheel_delete_element(ls);
            ls = next;
        }
    }
}

epoll_t* new_epoll(size_t maxevents, int timeout) {
    int epfd = epoll_create(1);
    if(epfd < 0) {
        dperror(epfd);
        return NULL;
    }
    epoll_t* ep = (epoll_t *)malloc(sizeof(epoll_t));
    ep->maxevents = maxevents == 0? 1 : maxevents;
    ep->timeout = timeout < 0? 0 : timeout; //epoll_wait can be blocked
    ep->fd = epfd;
    ep->events = (struct epoll_event *)malloc(sizeof(struct epoll_event) * ep->maxevents); 
    ep->loop = 1;
    ep->timer = NULL;
    return ep;
}

void init_epoll_timer(epoll_t *ep, size_t sec_size, size_t interval) {
    assert(ep && ep->fd >= 0);
    time_wheel_t *tw = new_time_wheel(sec_size, interval);
    ep->timer = tw;

    inner_fd *ifd = new_inner_fd(tw->fd);
    ifd->flags |= O_NONBLOCK;
    ifd->timeout = -1;
    ifd->task = task_create(_timeout_task, (void *)ep, NULL);
    DBG_LOG("timer task: %lu",ifd->task->cid);
    co_resume(ifd->task);
}

void delete_epoll(epoll_t *ep) {
    if(!ep) return;
    close(ep->fd);
    if(ep->timer) {
        inner_fd *ifd = get_inner_fd(ep->timer->fd);
        task_destory(ifd->task);
        delete_time_wheel(ep->timer);
    }
    free(ep->events);
    free(ep);
}

int cntl_events(epoll_t *ep, int op, int fd, int events) {
    if(!ep || !is_fd_valid(fd)) {
        errno = EBADF;
        return -1;  
    }
    struct epoll_event evst;
    evst.events = events;
    evst.data.fd = fd;
    return epoll_ctl(ep->fd, op, fd, &evst);
}

int add_events(epoll_t *ep, int fd, int events) {
    //DBG_LOG("fd [%d], add events\n", fd);
    int ret = cntl_events(ep, EPOLL_CTL_ADD, fd, events);
    if(ret < 0 && errno == EEXIST) {
        INF_LOG("[Warn] fd: was added in epoll list", fd);
        ret = 0;
    }
    return ret;
}

int delete_events(epoll_t *ep, int fd, int events) {
    //DBG_LOG("fd [%d], delete events\n", fd);
    int ret = cntl_events(ep, EPOLL_CTL_DEL, fd, events);
    if(ret < 0 && errno == ENOENT) { ret = 0; }
    return ret;
}

int modify_events(epoll_t *ep, int fd, int events) {
    return cntl_events(ep, EPOLL_CTL_MOD, fd, events);
}

int event_poll(epoll_t* ep, int fd, int events) {
    inner_fd* ifd = get_inner_fd(fd);
    if(!ifd) { PANIC("ifd is NULL"); };

    ifd->error = IENONE;
    ifd->task = co_self();
    if(add_events(ep, fd, events) != 0) {
        delete_events(ep, fd, events);
        dperror(-1);
        return -1;      
    }

    wheel_update_element(ep->timer, &ifd->link, ifd->timeout);
    co_io_yield();
    wheel_delete_element(&ifd->link);

    if(delete_events(ep, ifd->fd, 0) != 0) {
        dperror(-1);
        return -1;
    }
    
    //error handle
    if(ifd->error & IETIMEOUT) {
        ERR_LOG("fd: %d timeout", ifd->fd);   
        errno = ETIME;
        return -1;
    }

    if(ifd->error & IEERR) {
        DBG_LOG("fd[%d] error event", ifd->fd);
        errno = ENETDOWN;
        return -1;
    }

    if(ifd->error & IERDHUP) {
        DBG_LOG("fd[%d] RDHUP, peer seems be closed (maybe writing half close)", ifd->fd);
    }

    if(ifd->error & IEHUP) {
        errno = ENETDOWN;
        DBG_LOG("fd[%d] seems be closed", ifd->fd);
        return -1;
    }
    return 0;
}

void stop_event_loop(epoll_t* ep) {
    if(ep) ep->loop = 0;
}

int events_to_error(int events) {
    int error = IENONE;

    if(events & EPOLLIN) {
        error |= IEREAD;
    }

    if(events & EPOLLOUT) {
        error |= IEWRITE;
    }

    if(events & EPOLLERR) {
        error |= IEERR;
    }
    
    if(events & EPOLLRDHUP) {
        error |= IERDHUP;
    }

    if(events & EPOLLHUP) {
        error |= IEHUP;
    }

    return error;
}

int event_wait(epoll_t *ep) {
    assert(ep);
    int num, i;
    inner_fd* ifd = NULL;
    num = epoll_wait(ep->fd, ep->events, ep->maxevents, ep->timeout);
    switch(num) {
        case -1:
            break;
        case 0:
            break;
        default: {
            for(i = 0; i < num; i++) {
                ifd = get_inner_fd(ep->events[i].data.fd);
                ifd->error |= events_to_error(ep->events[i].events);
                delete_events(ep, ifd->fd, 0);

                if(ifd && ifd->task) co_resume(ifd->task);
                DBG_LOG("event loop wakeup task: %lu", ifd->task->cid);
            }
        }
    }
    //DBG_LOG("epoll wait : %d task", num);
    if(num < 0) dfatal(num);
    return num;
}
