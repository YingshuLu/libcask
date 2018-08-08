#ifndef LIBCASK_CO_BAR_H_
#define LIBCASK_CO_BAR_H_

#include "co_cond.h"

struct _co_bar_st {
    co_mutex_t mutex;
    co_cond_t cond;
    atomic_t  count;
};
typedef struct _co_bar_st co_bar_t;

void co_bar_init(co_bar_t *bar, unsigned int cnt) {
    co_mutex_init(&bar->mutex);
    co_cond_init(&bar->cond);
    atomic_set(&bar->count, cnt);
}

void co_bar_wait(co_bar_t *bar) {
    if(0 == atomic_get(&bar->count)) return;
    co_mutex_lock(&bar->mutex);
    if(0 == atomic_get(&bar->count)) {
        goto bar_end;
    }

    if(0 == atomic_sub_and_fetch(&bar->count, 1)) {
        co_cond_broadcast(&bar->cond);
        goto bar_end;
    }

    while(atomic_get(&bar->count) > 0) {
        co_cond_wait(&bar->cond, &bar->mutex);
    }

bar_end:
    co_mutex_unlock(&bar->mutex);
}

void co_bar_destory(co_bar_t *bar) {
    co_mutex_lock(&bar->mutex);
    atomic_set(&bar->count, 0);
    co_cond_broadcast(&bar->cond);
    co_mutex_unlock(&bar->mutex);
}

#endif
