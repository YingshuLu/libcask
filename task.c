#include <stdio.h>
#include <string.h>
#include "task.h"
#include "task_sched.h"
#include "cid.h"

//when malloc size > 128KB, malloc will call mmap,
//and only if the space is read/writed, the physical memory will be mapped.
//by default, use 1MB stack size
#define RUN_STACK_SIZE (1<<20)
#define TASK_RUN(t) (t->context.func(t->context.iparam, t->context.oparam))
#define TASK_KILL(t) (t->status = KILLED)
extern void co_enable_hook();
__thread unsigned int g_task_alive = 0;

void _task_func() {
    co_enable_hook();
    task_t * current = current_task();
    TASK_RUN(current);
    task_exit();
}

int task_init(task_t *t, cofunc_t f, void *ip, void *op) {
    if(!t) return -1;

    t->context.func = f;
    t->context.iparam = ip? ip : NULL;
    t->context.oparam = op? op : NULL;
    
    rstack_t *s = &(t->context.rstack);
    s->stack_size = f? RUN_STACK_SIZE : 0;
    s->stack_ss = s->stack_size != 0? (char *)malloc(s->stack_size) : NULL;
    s->stack_sp = s->stack_ss;

    if(t->context.func) {
        getcontext(&(t->context.uctx));
        t->context.uctx.uc_stack.ss_sp = s->stack_ss;
        t->context.uctx.uc_stack.ss_size = s->stack_size;
        makecontext(&(t->context.uctx), _task_func, 0);
    }

    t->specs = NULL;
    t->cid = alloc_cid();
    DBG_LOG("task create: %lu", t->cid);
    list_init(&(t->run_link));
    t->hook = 0;
    t->errnu = 0;
    t->status = READY;
    t->sched = NULL;

    return 0;
}

task_t *task_create(cofunc_t f, void *ip, void *op) {
    task_t *t = (task_t *) malloc(sizeof(task_t));
    task_init(t, f, ip, op);
    ++g_task_alive;
    return t; 
}

void task_destory(task_t *t) {
    if(!t) return;
    DBG_LOG("destory task: %lu", t->cid);
    list_delete(&t->run_link);
    free(t->context.rstack.stack_ss);
    free_cid(t->cid);
    if(t->specs) free_co_specs(t->specs);
    free(t);
    --g_task_alive;
}

void task_switch(task_t *ot, task_t *nt) {
    swapcontext(&(ot->context.uctx), &(nt->context.uctx)); 
}

void task_exit() {
    task_t *current = current_task();
    if(NULL == current) return;
    TASK_KILL(current);
    task_switch(current, current_sched()->sched_task);
}

int task_count() {
    return g_task_alive;
}
