#ifndef LIBCASK_SPIN_LOCK_H_
#define LIBCASK_SPIN_LOCK_H_

#include "types.h"
#include "atomic.h"

#ifndef CO_TICKET_SPINLOCK
//raw spin lock
#define SPIN_RELESAED 1
#define SPIN_ACQUIRED 0

struct _spin_lock_st {
    atomic_t lock;
};
typedef struct _spin_lock_st spin_lock_t;

#define spin_lock_init(sl)  (atomic_set(&((sl)->lock), SPIN_RELESAED))
#define spin_lock(sl)       while(! atomic_compare_and_swap(&((sl)->lock), SPIN_RELESAED, SPIN_ACQUIRED))
#define spin_unlock(sl)     do {\
    if(!spin_locked(sl)) { PANIC("spin lock in not locked"); }\
    atomic_compare_and_swap(&((sl)->lock), SPIN_ACQUIRED, SPIN_RELESAED);\
} while(0)
#define spin_locked(sl)     (atomic_get(&((sl)->lock)) == SPIN_ACQUIRED)
#define spin_trylock(sl)    ((spin_locked(ls)) || atomic_compare_and_swap(&((sl)->lock), SPIN_RELESAED, SPIN_ACQUIRED))

#else
//ticket spin lock
struct _co_spinlock_st {
    atomic_t next;
    atomic_t owner;
};
typedef struct _co_spinlock_st co_spinlock_t;

#define co_spinlock_init(sl) do {\
    atomic_set(&((sl)->next), 0);\
    atomic_set(&((sl)->owner), 0);\
} while(0)

#define co_spinlock_lock(sl) do {\
    int id = atomic_fetch_and_add(&((sl)->next), 1);\
    while(id != atomic_get(&((sl)->owner))){}\
} while(0)

#define co_spinlock_unlock(sl) atomic_inc(&((sl)->owner))
#define co_spinlock_locked(sl) (atomic_get(&sl->owner) != atomic_get(&((sl)->next)))

typedef struct _co_spinlock_st spin_lock_t;
#define spin_lock_init(sl) co_spinlock_init(sl)
#define spin_lock(sl) co_spinlock_lock(sl)
#define spin_unlock(sl) co_spinlock_unlock(sl)
#define spin_locked(sl) co_spinlock_locked(sl)

#endif //CO_TICKET_LOCK
    
#endif
