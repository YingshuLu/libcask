#ifndef LIBCASK_INNER_DEFINE_H_
#define LIBCASK_INNER_DEFINE_H_

#include "co_define.h"

#define co_io_yield() do {\
    co_self()->status = IO_BLOCK;\
    co_yield();\
} while(0)

#define co_sync_yield() do {\
    co_self()->status = SYS_BLOCK;\
    co_yield();\
} while(0)

#define co_sys_yield() do {\
    co_self()->status = SYS_BLOCK;\
    co_yield();\
} while(0)

#define co_resume(t) sched_rq_enqueue(t)

#endif
