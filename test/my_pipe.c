#define _GNU_SOURCE
#include <fcntl.h>
#include <unistd.h>
#include "co_define.h"

int fds[2];

void task(void *ip, void *op) {
    pipe2(fds, O_NONBLOCK);
    close(fds[1]);
    close(fds[0]);
}

int main() {

    co_create(task, NULL, NULL);
    schedule();
    return 0;
    
}
