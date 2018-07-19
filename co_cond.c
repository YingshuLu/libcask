#include "co_mutex.h"
#include "co_inner_define.h"
#include "co_cond.h"

void co_cond_init(co_cond_t *cond) {
    list_init(&cond->wait_list);
}

void co_cond_wait(co_cond_t *cond, co_mutex_t *lock) {
    if(!co_mutex_locked(lock)) {
        PANIC("cond_wait mutex is not locked");
    }

    list_add_before(&cond->wait_list, &((co_self())->run_link));
    co_mutex_unlock(lock);
    co_sync_yield();
    co_mutex_lock(lock);
}

//con signal should be protected by mutex
void _co_cond_signal_all(co_cond_t *cond, int all) {
    list_t *ls = list_next(&cond->wait_list);
    task_t *t = NULL;
    while(!list_empty(ls)) {
        t = list_to_task(ls);
        ls = list_next(ls);
        list_delete(&t->run_link);
        co_resume(t);
        if(!all) break;
    }
}

