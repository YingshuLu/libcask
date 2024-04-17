#ifndef LIBCASK_ATOMIC_H_
#define LIBCASK_ATOMIC_H_

struct _atomic_st {
    volatile int v;
};
typedef struct _atomic_st  atomic_t;
typedef atomic_t atomic_int_t;

#define ACCESS_ONCE(v) (*((volatile typeof((v))*)&(v)))

#define LOCK "lock;"
#define INLINE __inline__ __attribute__((always_inline))
#define atomic_init(t) (atomic_set(t, 0))
#define atomic_get(t) ACCESS_ONCE((t)->v)
#define atomic_set(t, n) (((t)->v) = (n))
#define atomic_add_and_fetch(t, n) __sync_add_and_fetch(&((t)->v), n)
#define atomic_sub_and_fetch(t, n) __sync_sub_and_fetch(&((t)->v), n)
#define atomic_fetch_and_add(t, n) __sync_fetch_and_add(&((t)->v), n)
#define atomic_fetch_and_sub(t, n) __sync_fetch_and_sub(&((t)->v), n)
#define atomic_add(t, n) atomic_add_and_fetch(t, n)
#define atomic_sub(t, n) atomic_sub_and_fetch(t, n)
#define atomic_compare_and_swap(t, o, n) __sync_bool_compare_and_swap(&((t)->v), (o), (n))

INLINE void atomic_inc(atomic_t *t) {
    __atomic_fetch_add(&t->v, 1, __ATOMIC_SEQ_CST);
}

INLINE void atomic_dec(atomic_t *t) {
    __atomic_fetch_sub(&t->v, 1, __ATOMIC_SEQ_CST);
}

INLINE int atomic_dec_and_negative(atomic_t *t) {
    return atomic_sub_and_fetch(t, 1) < 0? 1 : 0;
}

INLINE int atomic_inc_and_negative(atomic_t *t) {
   return atomic_add_and_fetch(t, 1) < 0? 1 : 0;
}

#endif