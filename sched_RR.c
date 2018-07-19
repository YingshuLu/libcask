#include <sched.h>
#include <unistd.h>
#include <fcntl.h>
#include "task_sched.h"
#include "sched_RR.h"
#include "co_spin_lock.h"
#include "co_define.h"

#define delay_event(time) (current_sched()->epoll->timeout = time)
#define RQ_EMPTY_LOOP_LIMIT 10

/*
#define SCHED_ACTIVE_FDS (current_sched()->active)
#define SCHED_ACTIVE_FD SCHED_ACTIVE_FDS[1]

static void active(void *ip, void *op) {
    if(pipe(SCHED_ACTIVE_FDS) == -1) { PANIC("failed open pipe"); }
    int flags = 0;
    for(int i = 0; i < 2; i++) {
        flags = fcntl(SCHED_ACTIVE_FDS[i], F_GETFL);
        fcntl(SCHED_ACTIVE_FDS[i], F_SETFL, flags | O_NONBLOCK);
    }

    char buffer[3];
    int ret = 1;
    while(ret == 1) {
        ret = read(SCHED_ACTIVE_FDS[0], buffer, 3); 
        DBG_LOG("active task read: %s", buffer);
    }

    for(int i = 0; i < 2; i++) {
        close(SCHED_ACTIVE_FDS[i]);
    }
    DBG_LOG("active task destory");
}

int init_active_task() {
    co_create(active, NULL, NULL);
    return 0;
}
*/

static void RR_init(run_queue_t *rq) {
    list_init(&rq->run_list);
    rq->task_num = 0;
    spin_lock_init(&rq->lock);
}

void RR_enqueue(run_queue_t *rq, task_t * t) {
    //INF_LOG("RR enqueue, rq : %p, task: %p", rq, t);
    list_delete(&t->run_link);

    spin_lock(&rq->lock);
    list_add_before(&rq->run_list, &t->run_link);
    rq->task_num++;
    spin_unlock(&rq->lock);

    DBG_LOG("enqueue num: %lu", rq->task_num);
}

void RR_dequeue(run_queue_t *rq, task_t * t) {
    list_delete(&t->run_link);
    rq->task_num--;
    DBG_LOG("dequeue num: %lu", rq->task_num);
}

task_t *RR_picknext(run_queue_t *rq) {
    epoll_t *ep = current_sched()->epoll;
    /*
    if(UNLIKELY(!ep->timer)) { 
        init_epoll_timer(ep, 60 * 10, 1);
        init_active_task();
    }
    */

    list_t *ls = rq->run_list.next;

    do {
        //no runable tasks, block thread on epoll_wait for CPU saving
        if(!event_wait(current_sched()->epoll) && list_empty(ls)) { 
            delay_event(1000 * 60);
        }
        else delay_event(0);
        ls = rq->run_list.next;
    } while(list_empty(ls));

    ls = rq->run_list.next;
    task_t *t = list_to_task(ls);
    DBG_LOG("pick up task: %lu", t->cid);
    return t;
}

sched_policy_t sched_RR_policy = {
    .name = "sched_round_robin",
    .init = RR_init,
    .enqueue = RR_enqueue,
    .dequeue = RR_dequeue,
    .picknext = RR_picknext,
};
