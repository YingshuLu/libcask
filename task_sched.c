#include <unistd.h>
#include <fcntl.h>
#include "task_sched.h"
#include "list.h"
#include "task.h"
#include "event.h"
#include "types.h"
#include "sched_RR.h"
#include "co_mutex.h"
#include "inner_fd.h"
#include "cid.h"

static __thread task_t *g_current = NULL;
static __thread sched_t *g_sched = NULL;
__thread co_mutex_t g_dns_mutex; 
static run_queue_t g_rq;

#define SCHED_ACTIVE_FDS (current_sched()->active)
int sched_active_event(sched_t *sched) {
    if((sched)->active[1] >= 0) {
       return write((sched)->active[1], "1", 1);
    }
    return -1; 
}

static void _active_task(void *ip, void *op) {
    if(pipe(SCHED_ACTIVE_FDS) == -1) { PANIC("failed open pipe"); }
    int flags = 0;
    inner_fd *ifd = NULL;
    for(int i = 0; i < 2; i++) {
        flags = fcntl(SCHED_ACTIVE_FDS[i], F_GETFL);
        fcntl(SCHED_ACTIVE_FDS[i], F_SETFL, flags | O_NONBLOCK);
        ifd = get_inner_fd(SCHED_ACTIVE_FDS[i]);
        ifd->timeout = -1;
    }

    char buffer[256];
    int ret = 1;
    while(ret > 0) {
        ret = read(SCHED_ACTIVE_FDS[0], buffer, 256); 
        DBG_LOG("active task read return: %d", ret);
    }

    for(int i = 0; i < 2; i++) {
        close(SCHED_ACTIVE_FDS[i]);
        SCHED_ACTIVE_FDS[i] = -1;
    }
    DBG_LOG("active task destory");
}

static int init_active_task() {
    task_t *t = task_create(_active_task, NULL, NULL);
    DBG_LOG("active task: %lu", t->cid);
    sched_rq_enqueue(t);
    return 0;
}

inline task_t *current_task() { return g_current; }

sched_t *current_sched() {
    if(UNLIKELY(!g_sched)) {
        DBG_LOG("init sched");
        g_sched = sched_create();
        if(!g_sched->epoll->timer) { 
            init_epoll_timer(g_sched->epoll, 60 * 10, 1);
            init_active_task();
        }
        co_mutex_init(&g_dns_mutex);
    }
    return g_sched;
}

void schedule() {
    current_task();
    current_sched();

    task_t *prev_task = NULL;
    task_t *next_task = NULL;
    g_sched->in_sched = 1;

    while(1) {
        if(LIKELY(prev_task)) {
            switch(prev_task->status) {
                case RUNNABLE: {
                    sched_rq_enqueue(prev_task);
                    break;
                }
                case KILLED: {
                    //recycle task
                    task_destory(prev_task);
                    break;
                }
                default:
                    break;
            }
            prev_task = NULL;
        }

        next_task = sched_rq_picknext();
        if(next_task) {
            sched_rq_dequeue(next_task);
            g_current = next_task;
            {
                g_sched->in_sched = 0;
                gettimeofday(&g_sched->tv, NULL);
                g_current->status = RUNNABLE;
                task_switch(g_sched->sched_task, g_current);
                g_sched->in_sched = 1;
            }
            prev_task = g_current;
            g_current = NULL;
            next_task = NULL;
        }
        DBG_LOG("alive task num: %d", task_count());
#ifdef CO_SCHED_STOP
        //sched 3 task: sched, timer, activer
        if(no_task()) {
            if(g_sched->active[0] >= 0) {
                close(g_sched->active[1]);
                continue;
            }
            if(can_destory()) {
                break;
            }
        }
#endif
    }

    //at this point, thread is exiting
    //thread_specific_destory();
    sched_destory(g_sched);
}

static void sched_task(void *ip, void *op) {
    schedule();
}

sched_t *sched_create() {
    sched_t *sched = (sched_t *)malloc(sizeof(sched_t));
    sched->sched_task = task_create(NULL, NULL, NULL);
    DBG_LOG("sched task: %lu", sched->sched_task->cid);
    sched->epoll = new_epoll(1024, 0);
    sched->policy = &sched_RR_policy;
    sched->policy->init(&sched->rq);
    sched->in_sched = 0;
    sched->force_sched = 0;
    sched->tid = tid();
    sched->active[0] = sched->active[1] = -1;
    return sched;
}

void sched_destory(sched_t *sched) {
   if(!sched) return;
   task_destory(sched->sched_task);
   delete_epoll(sched->epoll);
   free(sched);
   g_sched = NULL;

   /*Bug Fix:
    *thread should not close GLOBAL fds & cid-allocator 
    *fds & cid are thread race condition
    *leave them to system clean up when process exited
    */
   //close_all_inner_fd();
   //cid_allocator_destory();
}

#define global_rq() (&(global_rq.run_list))
#define local_rq() (&(current_sched()->rq))
void sched_rq_enqueue(task_t *t) {
    t->status = RUNNABLE;
    sched_t *sched = current_sched();
    if(!(t->sched) || !(*(t->sched))) t->sched = &g_sched;
    sched = *(t->sched);

    //INF_LOG("current sched: %d, task sched: %d, task: %lu",  g_sched->tid, sched->tid, t->cid);
    //run_queue_t* rq = &sched->rq;
    sched->policy->enqueue(&sched->rq, t);
    if(g_sched->tid != sched->tid) {
        DBG_LOG("[CROSS] task: %lu active, cross thread sched: %d", t->cid, sched->tid);
        sched_active_event(sched);
    }
}
    
void sched_rq_dequeue(task_t *t) {
    current_sched()->policy->dequeue(local_rq(), t);
}
    
task_t *sched_rq_picknext() {
    return current_sched()->policy->picknext(local_rq());
}

void __cyg_profile_func_enter(void *func, void *caller) {
    DBG_LOG("in func enter: %p, caller: %p", func, caller);

    if(g_sched
      && g_current
      && !(g_sched->in_sched)
      && g_sched->force_sched) {
        task_switch(g_current, g_sched->sched_task);
        g_sched->force_sched = 0;
    }
}
