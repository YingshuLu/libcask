#ifndef LIBCASK_TIME_WHEEL_H_
#define LIBCASK_TIME_WHEEL_H_

#include <time.h>
#include "types.h"
#include "list.h"

struct _time_wheel_st {
    list_t *wheel;    
    int fd;
    size_t size;
    time_t interval; //ms
    unsigned long loc;
};
typedef struct _time_wheel_st time_wheel_t;

time_wheel_t* new_time_wheel(size_t size, time_t interval);
void wheel_rotate(time_wheel_t *tw);
void wheel_update_element(time_wheel_t *tw, list_t *elm, int timeout);
void wheel_delete_element(list_t *elm);
list_t* wheel_timeout_list(time_wheel_t *tw);
void delete_time_wheel(time_wheel_t *tw);

#endif

