#ifndef LIBCASK_CO_AWAIT_H_
#define LIBCASK_CO_AWAIT_H_

#include <pthread.h>
#include "co_define.h"

#define co_await async_wait

typedef int (*async_task_t)(void *ip);

int async_wait(async_task_t func, void *ip);

#endif
