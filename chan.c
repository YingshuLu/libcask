#include "chan.h"

#define validate_chan(chan) do { if(!(chan)) { PANIC("chan is NULL"); } } while(0)

void chan_open(chan_t *chan, size_t nelems, size_t elem_size) {
    MY_ASSERT(chan, "chan should not be NULL");
    queue_init(&chan->msgs);
    co_mutex_init(&chan->mutex);
    co_cond_init(&chan->not_full);
    co_cond_init(&chan->not_empty);
    chan->nelems = nelems;
    chan->elem_size = elem_size;
}

int chan_msgs_empty(chan_t *chan)  {
    return queue_empty(&chan->msgs);
}

int chan_msgs_full(chan_t *chan) {
    return (queue_size(&chan->msgs) == chan->nelems) || (chan->nelems == 0 && queue_size(&chan->msgs) == 1);
}

int chan_push_msg(chan_t *chan, void *msg) {
    void *nmsg = msg;
    if(chan->elem_size && msg) {
        nmsg = malloc(chan->elem_size);
        memcpy(nmsg, msg, chan->elem_size);
    }
    queue_push(&chan->msgs, nmsg);
    return 0;
}

void *chan_pop_msg(chan_t *chan) {
    return  queue_pop(&chan->msgs);
}

int chan_buffered(chan_t *chan) {
    MY_ASSERT(chan, "chan should not be NULL");
    return chan->nelems;
}

int chan_nobuffer_send(chan_t *chan, void *msg) {
    int sent = 0;
    co_mutex_lock(&chan->mutex);
    do {
        if(chan_msgs_empty(chan)) {
            chan_push_msg(chan, msg);
            co_cond_signal(&chan->not_empty);
            sent = 1;
        }
        while(!chan_msgs_empty(chan)) {
            co_cond_wait(&chan->not_full, &chan->mutex);
            if(sent) break;
        }
    } while(!sent);
    co_mutex_unlock(&chan->mutex);
    return 0;
}

int chan_nobuffer_recv(chan_t *chan, void **msg) {
    co_mutex_lock(&chan->mutex);
    while(chan_msgs_empty(chan)) {
        co_cond_wait(&chan->not_empty, &chan->mutex);
    }

    *msg = chan_pop_msg(chan);
    //co_cond_broadcast(&chan->not_full);
    //wake up sender and succssor
    co_cond_signal(&chan->not_full);
    co_cond_signal(&chan->not_full);
    co_mutex_unlock(&chan->mutex);
    return 0;
}

int chan_buffer_send(chan_t *chan, void* msg) {
    co_mutex_lock(&chan->mutex);
    while(chan_msgs_full(chan)) {
        co_cond_wait(&chan->not_full, &chan->mutex);
    }
    chan_push_msg(chan, msg);
    co_cond_signal(&chan->not_empty);
    co_mutex_unlock(&chan->mutex);
    return 0;
}

int chan_buffer_recv(chan_t *chan, void **msg) {
    co_mutex_lock(&chan->mutex);
    while(chan_msgs_empty(chan)) {
        co_cond_wait(&chan->not_empty, &chan->mutex);
    }
    *msg = chan_pop_msg(chan);
    co_cond_signal(&chan->not_full);
    co_mutex_unlock(&chan->mutex);
    return 0;
}

int chan_send(chan_t *chan, void *msg) {
    MY_ASSERT(chan, "chan should not be NULL");
    return chan_buffered(chan)? chan_buffer_send(chan, msg) : chan_nobuffer_send(chan, msg);
}

int chan_recv(chan_t *chan, void **msg) {
    MY_ASSERT(chan, "chan should not be NULL");
    return chan_buffered(chan)? chan_buffer_recv(chan, msg) : chan_nobuffer_recv(chan, msg);
}

int chan_size(chan_t *chan) {
    return queue_size(&chan->msgs);
}

void chan_close(chan_t *chan) {
    queue_clear(&chan->msgs);
}
