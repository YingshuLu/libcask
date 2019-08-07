#define _GNU_SOURCE

#include <dlfcn.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <stdarg.h>
#include "event.h"
#include "inner_fd.h"
#include "co_inner_define.h"
#include "sys_helper.h"
#include "co_mutex.h"

typedef int socklen_t;
typedef int (*fcntl_pfn_t)(int fd, int cnd, ...);
typedef int (*socket_pfn_t)(int domain, int type, int protocol);
typedef int (*listen_pfn_t)(int sockfd, int backlog);
typedef int (*connect_pfn_t)(int sockfd, const struct sockaddr *address, socklen_t address_len);
typedef int (*accept_pfn_t)(int sockfd, struct sockaddr *address, socklen_t *address_len);
typedef int (*close_pfn_t)(int fd);

typedef int (*read_pfn_t)(int fd, void *buffer, size_t n);
typedef ssize_t (*recv_pfn_t)(int sockfd, void *buf, size_t len, int flags);
typedef ssize_t (*recvfrom_pfn_t)(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrelen);
typedef ssize_t (*recvmsg_pfn_t)(int sockfd, struct msghdr* msg, int flags);

typedef int (*write_pfn_t)(int fd, const void *buffer, size_t n);
typedef ssize_t (*send_pfn_t)(int sockfd, const void *buf, size_t len, int flags);
typedef ssize_t (*sendto_pfn_t)(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrelen);
typedef ssize_t (*sendmsg_pfn_t)(int sockfd, const struct msghdr *msg, int flags);
typedef unsigned int (*sleep_pfn_t)(unsigned int seconds);
typedef struct hostent *(*gethostbyname_pfn_t)(const char *name);
typedef int (*gethostbyname_r_pfn_t)(const char *name, struct hostent *host, char *buf, size_t buflen, struct hostent **result, int *h_errnop);
typedef int (*__poll_pfn_t)(struct pollfd *fds, unsigned long int nfds, int timeout);

static fcntl_pfn_t     hook_fcntl_pfn    = NULL;
static socket_pfn_t    hook_socket_pfn   = NULL; 
static listen_pfn_t    hook_listen_pfn   = NULL;
static connect_pfn_t   hook_connect_pfn  = NULL; 
static accept_pfn_t    hook_accept_pfn   = NULL; 
static read_pfn_t      hook_read_pfn     = NULL; 
static write_pfn_t     hook_write_pfn    = NULL; 
static close_pfn_t     hook_close_pfn    = NULL; 
static recv_pfn_t      hook_recv_pfn     = NULL;
static recvfrom_pfn_t  hook_recvfrom_pfn = NULL;
static recvmsg_pfn_t   hook_recvmsg_pfn  = NULL;
static send_pfn_t      hook_send_pfn     = NULL;
static sendto_pfn_t    hook_sendto_pfn   = NULL;
static sendmsg_pfn_t   hook_sendmsg_pfn  = NULL;
static sleep_pfn_t     hook_sleep_pfn    = NULL;
static gethostbyname_pfn_t      hook_gethostbyname_pfn   = NULL;
static gethostbyname_r_pfn_t    hook_gethostbyname_r_pfn = NULL;
static __poll_pfn_t             hook___poll_pfn = NULL;

#define HOOK_SYS_CALL(func) { if(!hook_##func##_pfn) hook_##func##_pfn = (func##_pfn_t)dlsym(RTLD_NEXT, #func); }
#define current_thread_epoll() (current_sched()->epoll)

int fcntl(int fd, int cmd, ...) {
    HOOK_SYS_CALL(fcntl);
    if( fd < 0 ){
        errno = EBADF;
        return -1;  
    }

    DBG_LOG("hook fcntl");

    va_list arg_list;
    va_start( arg_list,cmd );

    int ret = -1;
    inner_fd *ifd = get_inner_fd(fd);
    switch( cmd ) {
        case F_DUPFD_CLOEXEC:
        case F_DUPFD: {
            int param = va_arg(arg_list, int);
            ret = hook_fcntl_pfn(fd, cmd, param );

            if(ret > 0 && co_hooked() && ifd && (ifd->flags & O_NONBLOCK)) {
                new_inner_fd(ret);
            }
            break;
        }
        case F_GETFD: {
            ret = hook_fcntl_pfn(fd, cmd);
            break;
        }
        case F_SETFD: {
            int param = va_arg(arg_list, int);
            ret = hook_fcntl_pfn( fd, cmd, param );
            break;
        }
        case F_GETFL: {
            if(!co_hooked() || !ifd) ret = hook_fcntl_pfn(fd, cmd);
            else ret = ifd->user_flags;
            break;
        }
        case F_SETFL: {
            int flags = va_arg(arg_list, int);
            if(!co_hooked()) {
                ret = hook_fcntl_pfn(fd, cmd, flags);
                break;
            }

            if(!ifd && (flags & O_NONBLOCK)) {
                ifd = new_inner_fd(fd);
            }

            //ignore unset O_NONBLOCK
            if (!(flags & O_NONBLOCK)) {
                ifd->user_flags = flags;
                flags |= O_NONBLOCK;
                ret = hook_fcntl_pfn(fd, cmd, flags);
                break;
            }

            if(ifd && ifd->user_flags == flags) {
                ret = 0;
                break;
            }

            if(ifd && ifd->flags == flags) {
                ifd->user_flags = ifd->flags;
                ret = 0;
                break;
            }

            ret = hook_fcntl_pfn(fd, cmd, flags);
            if(0 == ret && ifd) { ifd->flags = ifd->user_flags = flags; }
            break;
        }
        case F_GETOWN: {
            ret = hook_fcntl_pfn(fd, cmd);
            break;
        }
        case F_SETOWN: {
            int param = va_arg(arg_list, int);
            ret = hook_fcntl_pfn(fd, cmd, param);
            break;
        }
        case F_GETLK: {
            struct flock *param = va_arg(arg_list, struct flock *);
            ret = hook_fcntl_pfn(fd, cmd, param);
            break;
        }
        case F_SETLK: {
            struct flock *param = va_arg(arg_list, struct flock *);
            ret = hook_fcntl_pfn(fd, cmd, param);
            break;
        }
        case F_SETLKW: {
            struct flock *param = va_arg(arg_list, struct flock *);
            ret = hook_fcntl_pfn(fd, cmd, param);
            break;
        }
    }
    va_end(arg_list);
    return ret;
}

static int in_set_nonblock(int sockfd) {
    co_disable_hook();
    int user_flags = fcntl(sockfd, F_GETFL);
    co_enable_hook();

    if(fcntl(sockfd, F_SETFL, user_flags | O_NONBLOCK) < 0) return -1;
    inner_fd *ifd = get_inner_fd(sockfd);
    ifd->user_flags = user_flags;
    return 0;
}

int socket(int domain, int type, int protocol) {
    HOOK_SYS_CALL(socket);
    int sockfd = hook_socket_pfn(domain, type, protocol);
    if(sockfd < 0 || !co_hooked()) return sockfd;
    DBG_LOG("socket hooked");

    /*
    co_disable_hook();
    int user_flags = fcntl(sockfd, F_GETFL);
    co_enable_hook();

    fcntl(sockfd, F_SETFL, user_flags | O_NONBLOCK);
    inner_fd *ifd = get_inner_fd(sockfd);
    ifd->user_flags = user_flags;
    */

    in_set_nonblock(sockfd);
    return sockfd;
}

int listen(int sockfd , int backlog) {
    HOOK_SYS_CALL(listen);
    inner_fd *ifd = get_inner_fd(sockfd);
    if(co_hooked() && ifd) ifd->timeout = -1;
    DBG_LOG("listen hooked");
    return  hook_listen_pfn(sockfd, backlog);
}

int connect(int sockfd, const struct sockaddr *address, socklen_t address_len) {
    HOOK_SYS_CALL(connect);
    if(!co_hooked()) return hook_connect_pfn(sockfd, address, address_len);
    inner_fd *isfd = get_inner_fd(sockfd);
    if(!isfd || !(O_NONBLOCK & isfd->flags)) return hook_connect_pfn(sockfd, address, address_len);

    DBG_LOG("connect hooked");
    int ret = hook_connect_pfn(sockfd, address, address_len);
    if(ret == -1 && (errno == EALREADY || errno == EINPROGRESS)) {
        int events = EPOLLOUT | EPOLLRDHUP | EPOLLERR;
        ret = event_poll(current_thread_epoll(), sockfd, events);
        if(ret != 0) return -1;
        if(isfd->error & IEWRITE) {
           errno = get_connect_error(sockfd);
           //success
           if(!errno) return 0; 
           dperror(-1);
           return -1;
        }
    }
    return ret;
}

int accept(int sockfd, struct sockaddr *address, socklen_t *address_len) {
    HOOK_SYS_CALL(accept);
    if(!co_hooked()) return hook_accept_pfn(sockfd, address, address_len);
    inner_fd *isfd = get_inner_fd(sockfd);
    if(!isfd || !(O_NONBLOCK & isfd->flags)) return hook_accept_pfn(sockfd, address, address_len);

    DBG_LOG("accept hooked");
    int ret = hook_accept_pfn(sockfd, address, address_len);
    if(ret < 0) {
        //error
        if(errno != EAGAIN && errno != EWOULDBLOCK) return ret;
        else {
            DBG_LOG("accept poll");
            int events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLET;
            ret = event_poll(current_thread_epoll(), sockfd, events);
            if(ret != 0) return ret;
            ret = hook_accept_pfn(sockfd, address, address_len);
            //error
            if(ret < 0) return ret;
        }
    }
    in_set_nonblock(ret);
    //fcntl(ret, F_SETFL, fcntl(ret, F_GETFL) | O_NONBLOCK);
    return ret;
}

int close(int fd) {
    HOOK_SYS_CALL(close);
    if(!co_hooked()) return hook_close_pfn(fd);
    inner_fd *ifd = get_inner_fd(fd);
    if(ifd) {
        delete_inner_fd(fd);
        DBG_LOG("close hooked [%d]", fd);
    }
    return hook_close_pfn(fd);
}

int read(int fd, void *buffer, size_t n) {
    HOOK_SYS_CALL(read);
    if(!co_hooked()) return hook_read_pfn(fd, buffer, n);
    inner_fd *ifd = get_inner_fd(fd);
    if(!ifd || !(O_NONBLOCK & ifd->flags)) return hook_read_pfn(fd, buffer, n);
    DBG_LOG("read hooked");
    int ret = hook_read_pfn(fd, buffer, n);
    if(ret == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        int events = EPOLLIN | EPOLLRDHUP | EPOLLERR;
        ret = event_poll(current_thread_epoll(), fd, events);
        if(ret != 0) return -1;
        ret = hook_read_pfn(fd, buffer, n);
        if(ret == 0 && (ifd->error & IERDHUP)) return -1;
    }
    return ret;
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
    HOOK_SYS_CALL(recv);
    if(!co_hooked()) return hook_recv_pfn(sockfd, buf, len, flags);
    inner_fd *ifd = get_inner_fd(sockfd);
    if(!ifd || !(O_NONBLOCK & ifd->flags)) return hook_recv_pfn(sockfd, buf, len, flags);
    DBG_LOG("recv hooked");

    int ret = hook_recv_pfn(sockfd, buf, len, flags);
    if(ret == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        int events = EPOLLIN | EPOLLRDHUP | EPOLLERR;
        ret = event_poll(current_thread_epoll(), sockfd, events);
        if (ret != 0) return -1;
        ret = hook_recv_pfn(sockfd, buf, len, flags);
        if(ret == 0 && (ifd->error & IERDHUP)) return -1;
    }
    return ret;
}

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen) {
    HOOK_SYS_CALL(recvfrom);
    if(!co_hooked()) return hook_recvfrom_pfn(sockfd, buf, len, flags, src_addr, addrlen);
    DBG_LOG("recvfrom hooked");
    inner_fd *ifd = get_inner_fd(sockfd);
    if(!ifd || !(O_NONBLOCK & ifd->flags)) return hook_recvfrom_pfn(sockfd, buf, len, flags, src_addr, addrlen);
    
    int ret = hook_recvfrom_pfn(sockfd, buf, len, flags, src_addr, addrlen);
    if(ret == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        int events = EPOLLIN | EPOLLRDHUP | EPOLLERR;
        ret = event_poll(current_thread_epoll(), sockfd, events);
        if (ret != 0) return -1;
        ret = hook_recvfrom_pfn(sockfd, buf, len, flags, src_addr, addrlen);
        if(ret == 0 && (ifd->error & IERDHUP)) return -1;
    }
    return ret;
}

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags) {
    HOOK_SYS_CALL(recvmsg);
    if(!co_hooked()) return hook_recvmsg_pfn(sockfd, msg, flags);
    DBG_LOG("hook recvmsg");
    inner_fd *ifd = get_inner_fd(sockfd);
    if(!ifd || !(O_NONBLOCK & ifd->flags)) return hook_recvmsg_pfn(sockfd, msg, flags);
    int ret = hook_recvmsg_pfn(sockfd, msg, flags);
    if(ret == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        int events = EPOLLIN | EPOLLRDHUP | EPOLLERR;
        ret = event_poll(current_thread_epoll(), sockfd, events);
        if (ret != 0) return -1;
        ret = hook_recvmsg_pfn(sockfd, msg, flags);
        if(ret == 0 && (ifd->error & IERDHUP)) return -1;
    }
    return ret;
}

int write(int fd, const void *buffer, size_t n) {
    HOOK_SYS_CALL(write);
    if(!co_hooked()) return hook_write_pfn(fd, buffer, n);
    inner_fd *ifd = get_inner_fd(fd);
    if(!ifd || !(O_NONBLOCK & ifd->flags)) return hook_write_pfn(fd, buffer, n);

    DBG_LOG("write hooked");
    int len, ret;
    int events = EPOLLOUT | EPOLLRDHUP | EPOLLERR | EPOLLET;
    len = 0;
    while(1) {
        while(len < n) {
            ret = hook_write_pfn(fd, buffer + len, n - len);
            if(ret < 0) {
                if(errno == EAGAIN || errno == EWOULDBLOCK) break;
                //error
                return ret;
            }
            len += ret;
        }
        if(!(len < n)) break;
        ret = event_poll(current_thread_epoll(), fd, events);
        //error
        if(ret != 0) return -1;
    }
    return len;
}

ssize_t send(int sockfd, const void *buf, size_t n, int flags) {
    HOOK_SYS_CALL(send);
    if(!co_hooked()) return hook_send_pfn(sockfd, buf, n, flags);
    inner_fd *ifd = get_inner_fd(sockfd);
    if(!ifd || !(O_NONBLOCK & ifd->flags)) return hook_send_pfn(sockfd, buf, n, flags);

    DBG_LOG("send hooked");
    int len, ret;
    int events = EPOLLOUT | EPOLLRDHUP | EPOLLERR | EPOLLET;
    len = 0;
    while(1) {
        while(len < n) {
            ret = hook_send_pfn(sockfd, buf + len, n - len, flags);
            if(ret < 0) {
                if(errno == EAGAIN || errno == EWOULDBLOCK) break;
                //error
                return ret;
            }
            len += ret;
        }
        if(!(len < n)) break;
        ret = event_poll(current_thread_epoll(), sockfd, events);
        //error
        if(ret != 0) return -1;
    }
    return len;
}

ssize_t sendto(int sockfd, const void *buf, size_t n, int flags, const struct sockaddr *dest_addr, socklen_t addrlen) {
    HOOK_SYS_CALL(sendto);
    if(!co_hooked()) return hook_sendto_pfn(sockfd, buf, n, flags, dest_addr, addrlen);
    inner_fd *ifd = get_inner_fd(sockfd);
    if(!ifd || !(O_NONBLOCK & ifd->flags)) return hook_sendto_pfn(sockfd, buf, n, flags, dest_addr, addrlen);

    DBG_LOG("sendto hooked");
    int len, ret;
    int events = EPOLLOUT | EPOLLRDHUP | EPOLLERR | EPOLLET;
    len = 0;
    while(1) {
        while(len < n) {
            ret =  hook_sendto_pfn(sockfd, buf + len, n - len, flags, dest_addr, addrlen);
            if(ret < 0) {
                if(errno == EAGAIN || errno == EWOULDBLOCK) break;
                //error
                return ret;
            }
            len += ret;
        }
        if(!(len < n)) break;
        ret = event_poll(current_thread_epoll(), sockfd, events);
        //error
        if(ret != 0) return -1;
    }
    return len;
}

ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags) {
    HOOK_SYS_CALL(sendmsg);
    if(!co_hooked()) return hook_sendmsg_pfn(sockfd, msg, flags);
    inner_fd *ifd = get_inner_fd(sockfd);
    if(!ifd || !(O_NONBLOCK & ifd->flags)) return hook_sendmsg_pfn(sockfd, msg, flags);

    DBG_LOG("hook sendmsg");
    int n = get_msg_len(msg);
    if(n < 0) return hook_sendmsg_pfn(sockfd, msg, flags);

    int len, ret;
    int events = EPOLLOUT | EPOLLRDHUP | EPOLLERR | EPOLLET;
    len = 0;
    while(1) {
        while(len < n) {
            ret = hook_sendmsg_pfn(sockfd, msg, flags);
            if(ret < 0) {
                if(errno == EAGAIN || errno == EWOULDBLOCK) break;
                //error
                return ret;
            }
            len += ret;
        }
        if(!(len < n)) break;
        ret = event_poll(current_thread_epoll(), sockfd, events);
        //error
        if (ret != 0) return -1;
    }
    return len;
}

unsigned int sleep(unsigned int seconds) {
    HOOK_SYS_CALL(sleep);
    if(!co_hooked()) return hook_sleep_pfn(seconds);
    DBG_LOG("sleep hook");
    return co_sleep(seconds * 1000);    
}

struct hostent *gethostbyname(const char *name) {
    HOOK_SYS_CALL(gethostbyname);
    if(!co_hooked()) return hook_gethostbyname_pfn(name);
    DBG_LOG("hook gethostbyname");
    return co_gethostbyname(name);
}

int gethostbyname_r(const char *name, struct hostent *host, char *buf, size_t buflen, struct hostent **result, int *h_errnop) {
    HOOK_SYS_CALL(gethostbyname_r);
    if(!co_hooked()) return hook_gethostbyname_r_pfn(name, host, buf, buflen, result, h_errnop);

    DBG_LOG("hook gethostbyname_r");
    extern __thread co_mutex_t g_dns_mutex;
    co_mutex_lock(&g_dns_mutex);
    int ret = hook_gethostbyname_r_pfn(name, host, buf, buflen, result, h_errnop);
    co_mutex_unlock(&g_dns_mutex);
    return ret;
}

int __poll(struct pollfd *fds, unsigned long int nfds, int timeout) {
    HOOK_SYS_CALL(__poll);
    if(!co_hooked()) return hook___poll_pfn(fds, nfds, timeout);
    DBG_LOG("hook __poll");
    return co_poll(fds, nfds, timeout);
}

void co_enable_hook() {
    co_self()->hook = 1;
}

void co_disable_hook() {
    co_self()->hook = 0;
}
