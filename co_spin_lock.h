#ifndef LIBCASK_SPIN_LOCK_H_
#define LIBCASK_SPIN_LOCK_H_

#include "types.h"
#include "atomic.h"

#ifndef CO_TICKET_SPINLOCK
struct _spin_lock_st {
    atomic_t lock;
};
typedef struct _spin_lock_st spin_lock_t;

#else
//ticket spin lock
struct _co_spinlock_st {
    atomic_t next;
    atomic_t owner;
};
typedef struct _co_spinlock_st spin_lock_t;

#endif

void spin_lock_init(spin_lock_t *sl);
void spin_lock(spin_lock_t* sl);
int  spin_locked(spin_lock_t* sl);
void spin_unlock(spin_lock_t* sl);

#endif
