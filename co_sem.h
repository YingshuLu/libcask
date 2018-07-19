#ifndef LIBCASK_SEM_H_
#define LIBCASK_SEM_H_

#include "atomic.h"
#include "co_spin_lock.h"
#include "list.h"

struct _co_sem_st {
   atomic_t sem;
   spin_lock_t wait_lock;
   list_t wait_list;
};
typedef struct _co_sem_st co_sem_t;

void co_sem_init(co_sem_t *s, unsigned int count);
void co_sem_wait(co_sem_t *s);
int co_sem_try_wait(co_sem_t *s); 
void co_sem_post(co_sem_t *s);
void co_sem_destory(co_sem_t *s);

#endif
