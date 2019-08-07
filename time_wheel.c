#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/timerfd.h>
#include "list.h"
#include "time_wheel.h"

// 60s 
#define MIN_TIME_WHEEL_SIZE 60
#define MIN_TIME_WHEEL_INTERVAL 1

time_wheel_t* new_time_wheel(size_t size, time_t interval) {
    time_wheel_t *tw = (time_wheel_t *) malloc(sizeof(time_wheel_t));
    tw->interval = interval < MIN_TIME_WHEEL_INTERVAL? MIN_TIME_WHEEL_INTERVAL : interval; 

    tw->fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK|TFD_CLOEXEC);
    struct itimerspec ltv;
    ltv.it_value.tv_sec = 1;
    ltv.it_value.tv_nsec = 0; // start after 1 s
    ltv.it_interval.tv_sec = 1;
    ltv.it_interval.tv_nsec = 0;
    timerfd_settime(tw->fd, 0, &ltv, NULL);

    tw->size = size > MIN_TIME_WHEEL_SIZE? size : MIN_TIME_WHEEL_SIZE;
    tw->wheel = (list_t *)malloc(sizeof(list_t) * tw->size);
    tw->loc = 0;
    int i;
    for(i = 0; i < tw->size; i++) {
        list_init(&(tw->wheel[i]));
    }
    return tw;
}

void wheel_rotate(time_wheel_t *tw) {
    assert(tw);
    tw->loc = (++(tw->loc)) % tw->size;
}

//timeout = -1 means never timeout
void wheel_update_element(time_wheel_t *tw, list_t *elm, int timeout) {
    assert(tw && elm);
    if(timeout < 0) return;
    int tv = (tw->loc + timeout + 1) % tw->size;
    wheel_delete_element(elm);
    list_add(&(tw->wheel[tv]), elm);
}

void wheel_delete_element(list_t *elm) {
    list_delete(elm);
}

list_t* wheel_timeout_list(time_wheel_t *tw) {
    return &(tw->wheel[tw->loc]);
}

void delete_time_wheel(time_wheel_t *tw) {
    if(tw) {
        close(tw->fd);
        free(tw->wheel);
        free(tw);
    }
}
