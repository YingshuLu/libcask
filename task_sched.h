#ifndef LIBCASK_SCHED_H_
#define LIBCASK_SCHED_H_

#include <sys/time.h>
#include "list.h"
#include "task.h"
#include "event.h"
#include "co_spin_lock.h"

typedef struct _sched_st sched_t;
typedef struct _run_queue_st run_queue_t;
typedef struct _sched_policy_st sched_policy_t;

struct _sched_policy_st {
    const char* name;
    void (*init)(run_queue_t *rq);
    void (*enqueue)(run_queue_t *rq, task_t* t);
    void (*dequeue)(run_queue_t *rq, task_t* t);
    task_t *(*picknext)(run_queue_t *rq);
};

void sched_rq_enqueue(task_t *t);
void sched_rq_dequeue(task_t *t);
task_t *sched_rq_picknext(); 

struct _run_queue_st {
    spin_lock_t lock;
    list_t run_list;
    size_t task_num;
}; 

struct _sched_st {
    run_queue_t rq; 
    task_t* sched_task;
    epoll_t *epoll;
    sched_policy_t* policy;
    int in_sched;
    int force_sched;
    int tid;
    struct timeval tv;
    int active[2];
};

task_t *current_task();
sched_t *current_sched();

sched_t *sched_create(); 
void sched_destory(sched_t* s); 
void schedule();
int sched_active_event(sched_t* s);

void __attribute__((__no_instrument_function__))
__cyg_profile_func_enter(void *func, void *caller); 

#define IS_IN_THREAD() (getpid() != tid())
//sched_task, timer_task
#define no_task() (task_count() <= 3)
#define can_destory() (task_count() <= 2)

#endif
