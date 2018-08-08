#ifndef LIBCASK_DEQUE_H_
#define LIBCASK_DEQUE_H_

#include "list.h"

typedef struct _deque_st {
    int size;
    list_t head;
} deque_t;

#define deque_init(d) do {\
    list_init(&d->head);\
    d->size = 0;\
}while(0)

#define deque_empty(d) (deque_size(d) == 0)
#define deque_size(d) ((d)->size)
#define deque_destory(d) do { deque_pop_back(d); }while(!deque_empty((d)))

void deque_push_front(deque_t* d, void* elm);
void deque_push_back(deque_t* d, void* elm);
void* deque_pop_front(deque_t *d);
void* deque_pop_back(deque_t *d);
void  deque_clear(deque_t *d);

#endif
