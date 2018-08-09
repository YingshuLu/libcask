#ifndef LIBCASK_STACK_H_
#define LIBCASK_STACK_H_

#include "deque.h"

typedef deque_t stack_t;

#define stack_init(s)    deque_init((s))
#define stack_destory(s) deque_destory((s))
#define stack_empty(s)   deque_empty((s))
#define stack_size(s)    deque_size((s))
#define stack_push(s, e) deque_push_back((s), (e))
#define stack_pop(s)     deque_pop_back((s))

#endif
