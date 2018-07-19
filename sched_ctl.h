#ifndef LIBCASK_SCHED_CTL_H_
#define LIBCASK_SCHED_CTL_H_

#include "queue.h"

//thread once 
struct _sched_ctl_st {
    queue_t scheds;
};

typedef struct _sched_ctl_st sched_ctl_t;

void sched_ctl_init(sched_ctl_t *ss) {
    list_init(&ss->scheds);
}

int sched_ctl_add(sched_ctl_t *ss, sched_t *s) {
    queue_push(ss, s);
    return 0;
}

long time_num(struct timeval *tv) {
    return tv->tv_sec * 1000000 + tv->tv_usec;
}

void monitor_sched(sched_ctl_t *s) {
    while(1) {
        sched_t *s = (sched_t*) queue_pop(s);
        if (!(s->in_sched) && !(s->force_sched)) {
            struct timeval tv;
            gettimeofday(&tv, NULL);
            long ntv = time_num(tv);
            long otv = time_num(s->tv);

            //3s
            if(ntv - otv > 3 * 1000000) {
                if(otv == time_num(s->tv)) {
                    s->force_sched = 1;
                }
            }
        }
        
        sleep(1);
    }
}


#endif
