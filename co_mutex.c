#include <assert.h>
#include <unistd.h>
#include "types.h"
#include "co_mutex.h"
#include "co_inner_define.h"

#define MUTEX_LOCKED ((volatile long)(co_self()))
#define MUTEX_UNLOCKED 0

void task_wait_mutex(co_mutex_t *mutex, task_t *t) {
    //DBG_LOG("[mutex] add task: %d to wait list", t->cid);
    spin_lock(&mutex->wait_lock);
    //recheck if mutex is locked
    if(!co_mutex_locked(mutex)) {
        spin_unlock(&mutex->wait_lock);
    //    DBG_LOG("[mutex] warn, mutex unlocked, when add task: %d to wait list", t->cid);
        return;
    }
    list_add_before(&mutex->wait_list, &t->run_link);
    spin_unlock(&mutex->wait_lock);
    co_sync_yield();
}

void mutex_wakeup_task(co_mutex_t *mutex) {
    list_t* ls = &mutex->wait_list;
    task_t* t = NULL;

    spin_lock(&mutex->wait_lock);
    //relief competition
    if(co_mutex_locked(mutex)) {
        spin_unlock(&mutex->wait_lock);
        return;
    }
    ls = list_next(ls);
    while(!list_empty(ls)) {
        t = list_to_task(ls);
        list_delete(ls);
        break;
    }
    spin_unlock(&mutex->wait_lock);
    if(t) co_resume(t);
}

#define ATOMIC_READ_LOCK(mutex)  (*((volatile int *)(&(mutex)->lock)))
#define ATOMIC_READ_OWNER(mutex) (*((volatile task_t **)(&(mutex)->lock)))
#define ATOMIC_UNLOCK(mutex)     __sync_bool_compare_and_swap(&mutex->lock, MUTEX_LOCKED, MUTEX_UNLOCKED)
#define ATOMIC_LOCK(mutex)       __sync_bool_compare_and_swap(&mutex->lock, MUTEX_UNLOCKED, MUTEX_LOCKED)

int co_mutex_init(co_mutex_t *mutex) {
    mutex->lock = MUTEX_UNLOCKED;
    list_init(&mutex->wait_list);
    spin_lock_init(&mutex->wait_lock);
    return 0;
}

int co_mutex_locked(co_mutex_t *mutex) { return (ATOMIC_READ_OWNER(mutex) != NULL); }

void co_mutex_lock(co_mutex_t *mutex) {
    //not recusive mutex
    if(ATOMIC_READ_OWNER(mutex) == co_self()) { PANIC("recusive mutex lock"); }
    //DBG_LOG("[mutex] task: %d try lock", co_self()->cid);
    while(!ATOMIC_LOCK(mutex)) {
    //    DBG_LOG("[mutex] task: %d lock failed", co_self()->cid);
        task_wait_mutex(mutex, co_self());
    }
    //DBG_LOG("[mutex] task: %d locked", co_self()->cid);
    return;
}

int co_mutex_lock_wait(co_mutex_t *mutex, unsigned int seconds) {
    if(co_mutex_try_lock(mutex)) return 1;
    sleep(seconds);
    return co_mutex_locked(mutex);
}

int co_mutex_try_lock(co_mutex_t *mutex) {
    if(ATOMIC_READ_OWNER(mutex) == co_self()) return 1;
    return ATOMIC_LOCK(mutex);
}

void co_mutex_unlock(co_mutex_t *mutex) {
    if(ATOMIC_READ_OWNER(mutex) != co_self()) { PANIC("mutex is not locked by current task"); }
    if(!co_mutex_locked(mutex)) return;
    ATOMIC_UNLOCK(mutex);
    //DBG_LOG("[mutex] unlocked by task %d", co_self()->cid);
    mutex_wakeup_task(mutex);
}
