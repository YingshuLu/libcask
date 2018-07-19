
#include "co_sem.h"
#include "co_inner_define.h"

void co_sem_init(co_sem_t *s, unsigned int count) {
    atomic_set(&s->sem, count);
    list_init(&s->wait_list);
    spin_lock_init(&s->wait_lock);
}

void co_sem_wait(co_sem_t *s) {
    while(atomic_dec_and_negative(&s->sem)) {
        spin_lock(&s->wait_lock);
        if(atomic_get(&s->sem) >= 0) {
            spin_unlock(&s->wait_lock);
            break;
        }
        list_add_before(&s->wait_list, &(co_self()->run_link));
        spin_unlock(&s->wait_lock);
        co_sync_yield();
    }
}

int co_sem_try_wait(co_sem_t *s) {
    if(atomic_dec_and_negative(&s->sem)) {
        spin_lock(&s->wait_lock);
        if(atomic_get(&s->sem) >= 0) {
            spin_unlock(&s->wait_lock);
            return 1;
        }
        atomic_inc(&s->sem);
        spin_unlock(&s->wait_lock);
        return 0;
    }
    return 1;
}

void co_sem_post(co_sem_t *s) {
    if(atomic_inc_and_negative(&s->sem)) {
        task_t *t = NULL;
        spin_lock(&s->wait_lock);
        if(atomic_get(&s->sem) >= 0) {
            spin_unlock(&s->wait_lock);
            return;
        }
        list_t *ls = list_next(&s->wait_list);
        while(!list_empty(ls)) {
            t = list_to_task(ls);
            list_delete(ls);
            break;
        }
        spin_unlock(&s->wait_lock);
        if(t) co_resume(t); 
    }
}

void co_sem_destory(co_sem_t *s) {
    spin_lock(&s->wait_lock);
    spin_unlock(&s->wait_lock);
}

