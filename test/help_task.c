#include "task.h"

int ap[2];

#define SCHED_ACTIVE_FDS ap
#define SCHED_ACTIVE_FD SCHED_ACTIVE_FDS[1]
#define wakeup_sched() write(SCHED_ACTIVE_FD, "1", 1);
#define wakeup_sched_for_destory() write(SCHED_ACTIVE_FD, "11", 2)

static void *active(void *ip, void *op) {
    if(pipe(ap) == -1) { PANINC("failed open pipe"); }
    int flags = 0;
    for(int i = 0; i < 2; i++) {
        flags = fcntl(SCHED_ACTIVE_FDS[i], F_GETFL);
        fcntl(SCHED_ACTIVE_FDS[i], F_SETFL, flags | O_NONBLOCK);
    }

    char buffer[2];
    int ret = 1;
    while(ret == 1) {
        ret = read(SCHED_ACTIVE_FDS[0], buffer, 2); 
    }

    for(int i = 0; i < 2; i++) {
        close(SCHED_ACTIVE_FDS[i]);
    }
}

int init_active_task() {
    co_create(active, NULL, NULL);
    return 0;
}



