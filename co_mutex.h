#ifndef LIBCASK_MUTEX_H_
#define LIBCASK_MUTEX_H_

#include "list.h"
#include "sched.h"
#include "co_spin_lock.h"

struct _co_mutex_st {
    volatile long lock;
    spin_lock_t wait_lock;
    list_t wait_list;
};
typedef struct _co_mutex_st co_mutex_t;

int co_mutex_init(co_mutex_t *mutex);
int co_mutex_locked(co_mutex_t *mutex);
void co_mutex_lock(co_mutex_t *mutex);
int co_mutex_lock_wait(co_mutex_t *mutex, unsigned int seconds);
int co_mutex_try_lock(co_mutex_t *mutex);
void co_mutex_unlock(co_mutex_t *mutex);

#endif
