#ifndef LIBCASK_RWLOCK_H_
#define LIBCASK_RWLOCK_H_

#include "co_mutex.h"
#include "co_cond.h"

struct _co_rwlock_st {
    atomic_t reader;
    atomic_t writer;
    co_mutex_t mutex;
    co_cond_t writable;
};

void read_lock(co_rwlock_t *rwlock) {
   if(atomic_get(&rwlock->writer)) {
       co_mutex_lock(&rwlock->mutex);
   }
   atomic_inc(&rwlock->reader);
}

void read_unlock(co_rwlock_t *rwlock) {
    if(atomic_get(&rwlock->writer)) {
        co_mutex_unlock(&rwlock->mutex);
    }
    atomic_dec(&rwlock->reader);
}

void write_lock(co_rwlock_t *rwlock) {
    if(atomic_get(&rwlock->reader)) {
        co_mutex_lock(&rwlock->mutex);
        while(atomic_get(&rwlock->reader)){
            co_cond_wait(&rwlock->writable);
        }
        co_mutex_unlock(&rwlock->mutex);
    }

}


void_write_unlock(co_rwlock_t *rwlock) {
    

}

#endif
