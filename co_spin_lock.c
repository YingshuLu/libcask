
#include "types.h"
#include "atomic.h"
#include "co_spin_lock.h"

#ifndef CO_TICKET_SPINLOCK

//raw spin lock
#define SPIN_RELESAED 1
#define SPIN_ACQUIRED 0

void spin_lock_init(spin_lock_t* sl) {
    atomic_set(&((sl)->lock), SPIN_RELESAED);
}

void spin_lock(spin_lock_t* sl) {
    while(!atomic_compare_and_swap(&((sl)->lock), SPIN_RELESAED, SPIN_ACQUIRED)) {}
}

int spin_locked(spin_lock_t* sl) {
    return  (atomic_get(&((sl)->lock)) == SPIN_ACQUIRED);
}

void spin_unlock(spin_lock_t* sl) {
    do {
        if(!spin_locked(sl)) { PANIC("spin lock in not locked"); }
        atomic_compare_and_swap(&((sl)->lock), SPIN_ACQUIRED, SPIN_RELESAED);
    } while(0);
}

int spin_trylock(spin_lock_t* sl) {
    return ((spin_locked(sl)) || atomic_compare_and_swap(&((sl)->lock), SPIN_RELESAED, SPIN_ACQUIRED));
}

#else

void spin_lock_init(spin_lock_t* sl) {
    do {
        atomic_set(&((sl)->next), 0);
        atomic_set(&((sl)->owner), 0);
    } while(0);
}

void spin_lock(spin_lock_t* sl) {
    do {
        int id = atomic_fetch_and_add(&((sl)->next), 1);
        while(id != atomic_get(&((sl)->owner))){}
    } while(0);
}

int spin_locked(spin_lock_t* sl) {
    return (atomic_get(&sl->owner) != atomic_get(&((sl)->next)));
}
void spin_unlock(spin_lock_t* sl) {
    return atomic_inc(&((sl)->owner));
}

#endif

