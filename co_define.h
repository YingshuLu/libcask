#ifndef LIBCASK_CODEFINES_H_
#define LIBCASK_CODEFINES_H_

#include "task.h"
#include "task_sched.h"

//user api
#define co_create(f, ip, op) do {\
    current_sched();\
    task_t *t = task_create(f, ip, op);\
    sched_rq_enqueue(t);\
} while(0)

#define getcid()     (current_task()? current_task()->cid : 0)
#define co_self()    current_task()
#define co_yield()   task_switch(co_self(), current_sched()->sched_task)
#define co_exit()    task_exit()
#define co_errno()   (co_self()? (co_self()->errnu) : 0)

//note: key should be char*, vaule should be a pointer
#define co_spec_set(key, value, len) do {\
    if(!(co_self()->specs)) co_self()->specs = alloc_co_specs();\
    co_specs_set(co_self()->specs, key, value, len);\
} while(0)

//note: key should be char*, vaule should be a pointer-pointer
#define co_spec_get(key) ({\
   void *value = NULL;\
   if(!(co_self()->specs)) co_self()->specs = alloc_co_specs();\
   co_specs_get(co_self()->specs, key, &value);\
   value;\
 })
#define co_key_del(key) co_specs_del(co_self()->specs, key)

void co_enable_hook();
void co_disable_hook();
#define co_hooked()  (co_self() && co_self()->hook == 1)

#endif
