#ifndef LIBCASK_LIST_H_
#define LIBCASK_LIST_H_

struct _list_st {
    struct _list_st *prev, *next;
};
typedef struct _list_st list_t;

#define _to_offset(type, mem) ((size_t)(&(((type*)(0))->mem)))
#define list_to_struct(ptr, type, mem) ((type*)((char*)(ptr) - _to_offset(type, mem)))

void list_init(list_t *ls);
void list_add_before(list_t *ls, list_t *elm);
void list_add_after(list_t *ls, list_t *elm);
void list_add(list_t *ls, list_t *elm);
void list_delete(list_t *elm);
list_t* list_next(list_t *ls);
list_t* list_prev(list_t *ls); 
int list_empty(list_t *ls); 
#endif
