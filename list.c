#include "list.h"

#define _list_add(elm, p, n) do { \
    elm->prev = p;\
    elm->next = n;\
    p->next = n->prev = elm;\
} while(0)

#define _list_delete(p, n) do {\
    p->next = n;\
    n->prev = p;\
} while(0)

void list_init(list_t *ls) {
    ls->prev = ls->next = ls;
}

void list_add_before(list_t *ls, list_t *elm) {
    _list_add(elm, ls->prev, ls);
}

void list_add_after(list_t *ls, list_t *elm) {
    _list_add(elm, ls, ls->next);
}

void list_add(list_t *ls, list_t *elm) {
    return list_add_after(ls, elm);
}

void list_delete(list_t *elm) {
   _list_delete(elm->prev, elm->next); 
   list_init(elm);
}

list_t* list_next(list_t *ls) {
    return ls->next;
}

list_t* list_prev(list_t *ls) {
    return ls->prev;
}

int list_empty(list_t *ls) {
    return (ls->prev == ls->next && ls->prev == ls);
}

