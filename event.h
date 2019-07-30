#ifndef LIBCASK_EVENT_H_
#define LIBCASK_EVENT_H_

#include <stddef.h>
#include <sys/epoll.h>
#include "time_wheel.h"
typedef struct _epoll_st epoll_t;

struct _epoll_st { 
    size_t maxevents;
    int timeout;
    int fd;
    int loop;
    struct epoll_event *events;
    time_wheel_t* timer;
};

epoll_t* new_epoll(size_t maxevents, int timeout);
void init_epoll_timer(epoll_t *ep, size_t sec_size, time_t interval);
void delete_epoll(epoll_t *ep);

int add_events(epoll_t *ep, int fd, int events);
int delete_events(epoll_t *ep, int fd, int events);
int modify_events(epoll_t *ep, int fd, int events);

int event_poll(epoll_t* ep, int fd, int events);
int event_wait(epoll_t *ep);
void stop_event_loop(epoll_t* ep);
    
#endif
