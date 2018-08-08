#ifndef LIBCASK_QUEUE_H_
#define LIBCASK_QUEUE_H_

#include "deque.h"

typedef deque_t queue_t;

#define queue_init(q)    deque_init((q))
#define queue_destory(q) deque_destory((q))
#define queue_empty(q)   deque_empty((q))
#define queue_size(q)    deque_size((q))
#define queue_push(q, e) deque_push_back((q), e)
#define queue_pop(q)     deque_pop_front((q))
#define queue_clear(q)   deque_clear((q))

#endif
