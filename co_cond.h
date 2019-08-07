#ifndef LIBCASK_CO_COND_H_
#define LIBCASK_CO_COND_H_

#include <time.h>
#include "list.h"

struct _co_cond_st {
    list_t wait_list;
};
typedef struct _co_cond_st co_cond_t;

void co_cond_init(co_cond_t *cond);
void co_cond_wait(co_cond_t *cond, co_mutex_t *lock);
int  co_cond_wait_till(co_cond_t *cond, co_mutex_t *lock, time_t ms);

//con signal should be protected by mutex
void _co_cond_signal_all(co_cond_t *cond, int all);
#define co_cond_signal(cond) _co_cond_signal_all(cond, 0)
#define co_cond_broadcast(cond) _co_cond_signal_all(cond, 1)

#endif
