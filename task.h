#ifndef LIBCASK_TASK_H_
#define LIBCASK_TASK_H_

#include <ucontext.h>
#include "list.h"
#include "array.h"
#include "co_specs.h"

typedef unsigned long cid_t;
typedef struct _run_stack_st rstack_t;
typedef struct _task_st task_t;
typedef struct _context_st context_t;
typedef void(*cofunc_t)(void *ip, void *op);

typedef enum {
   READY,
   RUNNABLE,
   IO_BLOCK,
   SYS_BLOCK,
   SLEEP,
   KILLED,
   DEAD
}task_status_t;

struct _run_stack_st {
    char *stack_ss;
    char *stack_sp;
    size_t stack_size;
};

struct _context_st {
    ucontext_t uctx;
    void *iparam;
    void *oparam;
    cofunc_t func;
    rstack_t rstack;
};

struct _task_st {
  context_t context;
  list_t run_link;
  co_specs_t *specs;
  cid_t cid;
  int hook;
  int events;
  int errnu;
  task_status_t status;
  struct _sched_st **sched;
};

static const char *_status_str[] = {"TASK_READY", "TASK_RUNNABLE", "TASK_IO_BLOCK", "TASK_SYS_BLOCK", "TASK_SLEEP", "TASK_KILLED", "TASK_DEAD"}; 
#define STATUS_STRING(t) _status_str[t->status]
#define list_to_task(ls) list_to_struct(ls, task_t, run_link)

//inner build api
task_t *task_create(cofunc_t f, void *ip, void *op)
__attribute__((__no_instrument_function__));

void task_destory(task_t *t)
__attribute__((__no_instrument_function__));

void task_switch(task_t *ot, task_t *nt)
__attribute__((__no_instrument_function__));

void task_exit() 
__attribute__((__no_instrument_function__));

int task_count();

#endif
