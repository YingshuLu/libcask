#include <stdlib.h>
#include <assert.h>
#include "deque.h"

struct _queue_node_st {
    void* elm;
    list_t link;
};

typedef struct _queue_node_st node_t;

#define list_to_elm(l) (list_to_struct(l, node_t, link)->elm)

node_t* new_node(void* elm) {
    node_t *n = (node_t *)malloc(sizeof(node_t));
    n->elm = elm;
    list_init(&(n->link));
    return n;
}

void delete_node(node_t *n) {
    if(n) {
        list_delete(&(n->link));
        free(n);
    }
}

void deque_push_front(deque_t* d, void* elm) {
    assert(d);
    node_t* n = new_node(elm);
    list_add_after(&d->head, &(n->link));
    d->size++;
}

void deque_push_back(deque_t* d, void* elm) {
    assert(d);
    node_t* n = new_node(elm);
    list_add_before(&d->head, &(n->link));
    d->size++;
}

void* deque_pop(deque_t *d, int op) {
    assert(d);
    if(deque_empty(d)) return NULL;
    list_t* l = op == 1? list_next(&d->head) : list_prev(&d->head);
    list_delete(l);
    void* elm = list_to_elm(l);
    delete_node(list_to_struct(l, node_t, link));
    d->size--;
    return elm;
}

void* deque_pop_front(deque_t *d) {
    return deque_pop(d, 1);
}

void* deque_pop_back(deque_t *d) {
    return deque_pop(d, 0);
}

void deque_clear(deque_t *d) {
    while(!deque_empty(d)) {
      deque_pop_front(d);
    }
}
