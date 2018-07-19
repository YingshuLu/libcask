#ifndef LIBCASK_HEAP_H_
#define LIBCASK_HEAP_H_
#include "array.h"

typedef int (*comp_t)(void *l, void *r);
typedef struct _heap_st heap_t;
struct _heap_st {
    array_t array;
    comp_t cmp;
};

static int _default_cmp(void *first, void *second) {
    if(first > second) return 0;
    return 1;
}
#define heap_init(hp, comp) do { \
    array_init(&((hp)->array));\
    (hp)->cmp = comp? comp : _default_cmp;\
}while(0)
    
#define heap_destory(hp) do {\
    array_destory(&((hp)->array));\
    (hp)->cmp = NULL;\
}while(0)

#define heap_size(hp) array_size(&((hp)->array))
#define heap_empty(hp) (heap_size(hp) == 0)
int heap_push(heap_t *hp, void *elm);
int heap_pop(heap_t *hp, void **elm);

#endif

