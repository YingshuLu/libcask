#include "co_inner_define.h"
#include "co_await.h"

struct _co_await_st {
    async_task_t func;
    void *param;
    int ret;
    task_t *task;
};

typedef struct _co_await_st co_await_t;

void *_await_task(void *ip) {
    co_await_t *pwait = (co_await_t *)ip;
    if(!pwait) return NULL;
    if(pwait->task) { pthread_detach(pthread_self()); }
    if(pwait->func) {
        pwait->ret = pwait->func(pwait->param);
    }

    if(pwait->task) {
        if(pwait->task->sched) {
            DBG_LOG("co_await wakeup task: %lu", pwait->task->cid);
            sched_t *sched = *(pwait->task->sched);
            sched->policy->enqueue(&sched->rq, pwait->task);
            sched_active_event(sched);
            //DO NOT use pwait anymore, pwait is not safe now
        }
    }
    return NULL;
}

int async_wait(async_task_t func, void *ip) {
    if(!func) return -1;
    co_await_t await;

    await.func = func;
    await.param = ip;
    await.ret = -1;
    await.task = co_self();

    //fix me with thread-pool
    pthread_t tid;
    pthread_create(&tid, NULL, _await_task,  &await);
    if(!await.task) { pthread_join(tid, NULL); }
    else { co_sys_yield(); }
    return await.ret;
}
