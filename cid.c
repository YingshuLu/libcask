#include <unistd.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include "heap.h"
#include "cid.h"
#include "types.h"

#define THREAD_MAX_TASK_SIZE 100000

typedef struct _id_dispatch_st id_dispatch_t;
struct _id_dispatch_st {
    heap_t recycle;
    int base;
    size_t max_id;
    int high_tid;
};

static __thread id_dispatch_t* _cid_allocator = NULL;
__thread int _cached_tid = 0;

static int _id_cmp(void *l, void *r) {
   int lv = (cid_t)l;
   int rv = (cid_t)r;
   return lv < rv;
}

int id_dispatch_init(id_dispatch_t *idp, size_t max) {
    if(!idp) return -1;
    heap_init(&(idp->recycle), _id_cmp); 
    idp->max_id = max;
    idp->base = 0;
    idp->high_tid = tid() * max;
    return 0;
}

int id_dispatch_destory(id_dispatch_t *idp) {
    if(!idp) return -1;
    heap_destory(&(idp->recycle));
    return 0;
}

int cid_allocator_init(size_t max) {
    if (_cid_allocator) return -1;
    _cid_allocator = (id_dispatch_t *)malloc(sizeof(id_dispatch_t));
    id_dispatch_init(_cid_allocator, max);
    return 0;
}

void cid_allocator_destory() {
    if(!_cid_allocator) return;
    id_dispatch_destory(_cid_allocator);
    free(_cid_allocator);
    _cid_allocator = NULL;
}

cid_t alloc_cid() {
    if(!_cid_allocator) cid_allocator_init(THREAD_MAX_TASK_SIZE); 
    cid_t id = 0;
    //first loop completed, now reuse freed id
    if(_cid_allocator->base >= _cid_allocator->max_id) {
        void *elm;
        if(heap_pop(&(_cid_allocator->recycle), &elm) < 0) return -1;
        id = (int)elm;
    }
    else id = _cid_allocator->base++;
//    return id + _cid_allocator->high_tid;
    return id;
}

void free_cid(cid_t cid) {
    if(!_cid_allocator) return;
    cid %= _cid_allocator->max_id;
    heap_push(&(_cid_allocator->recycle), (void *)cid);
}

int tid() {
    if(UNLIKELY(_cached_tid == 0)) _cached_tid = syscall(SYS_gettid);
    return _cached_tid;
}
