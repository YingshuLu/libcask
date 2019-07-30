#ifndef LIBCASK_INNER_FD_H_
#define LIBCASK_INNER_FD_H_

#include "task.h"
#include "list.h"
#include "types.h"

#define IENONE    0
#define IETIMEOUT 1
#define IEREAD    2
#define IEWRITE   4
#define IERDHUP   8
#define IEERR    16
#define IEHUP    32

struct _inner_fd_st {
    int fd;
    int flags;
    int user_flags;
    task_t* task; //only set it when read / write fd
    list_t link; //time wheel
    time_t timeout;
    int error;
};

typedef struct _inner_fd_st inner_fd;
#define list_to_inner_fd(ls) list_to_struct(ls, inner_fd, link)

int is_fd_valid(int fd);
inner_fd* new_inner_fd(int fd);
inner_fd* get_inner_fd(int fd);
void set_inner_fd_timeout(int fd, time_t msecs);
void delete_inner_fd(int fd);
void close_all_inner_fd();

#endif
