#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include "inner_fd.h"
#include "task.h"
#include "list.h"

#define INVALID_FD -1
#define MAX_FD_NUM 102400

//5 seconds
#define FD_TIMEOUT 3

static inner_fd* g_inner_fd_list[MAX_FD_NUM] = {0};

int is_fd_valid(int fd) {
    return (fd > INVALID_FD && fd < MAX_FD_NUM);
}

inner_fd* new_inner_fd(int fd) {
    //if(!is_fd_valid(fd)) return NULL;
    inner_fd* ifd = (inner_fd*) malloc(sizeof(inner_fd));
    ifd->fd = fd;
    ifd->flags = 0;
    ifd->user_flags = 0;
    ifd->task = NULL;
    ifd->timeout = FD_TIMEOUT;
    ifd->error = IENONE;
    list_init(&(ifd->link));
    g_inner_fd_list[fd] = ifd;
    return ifd;
}

void delete_inner_fd(int fd) {
    if(!is_fd_valid(fd)) return;
    inner_fd* ifd = g_inner_fd_list[fd];
    if(ifd) {
        list_delete(&(ifd->link));
        free(ifd);
        g_inner_fd_list[fd] = NULL;
    }
}

inner_fd* get_inner_fd(int fd) {
    if(!is_fd_valid(fd)) {
        errno = EBADF;
        return NULL;
    }
    inner_fd* ifd = g_inner_fd_list[fd];
    return ifd;
}

void set_inner_fd_timeout(int fd, time_t msecs) {
      if(is_fd_valid(fd)) {
        inner_fd *ifd = get_inner_fd(fd);
        if(!ifd) ifd->timeout = msecs;
      }
}

void close_all_inner_fd() {
    int i;
    inner_fd* ifd = NULL;
    for(i = 0; i < MAX_FD_NUM; i++) {
        ifd = g_inner_fd_list[i];
        if(ifd) close(ifd->fd);
    }
}
