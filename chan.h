#ifndef LIBCASK_CHAN_H_
#define LIBCASK_CHAN_H_

#include "queue.h"
#include "co_mutex.h"
#include "co_cond.h"

struct _chan_st {
    co_mutex_t mutex;
    co_cond_t not_full;
    co_cond_t not_empty;
    queue_t msgs;
    unsigned int nelems;
    unsigned int elem_size;
};

typedef struct _chan_st chan_t;

void chan_open(chan_t *chan, size_t nelems, size_t elem_size);
int chan_send(chan_t *chan, void *msg);
int chan_recv(chan_t *chan, void **msg);
int chan_size(chan_t *chan);
void chan_close(chan_t *chan);
int chan_buffered();

#endif
