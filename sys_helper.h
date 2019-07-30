#ifndef LIBCASK_SYS_HELPER_H_
#define LIBCASK_SYS_HELPER_H_

struct hostent;
struct msghdr;
struct sockaddr;
struct pollfd;

int get_msg_len(const struct msghdr *msg);
int get_connect_error(int fd);
int co_sleep(unsigned long mseconds);
struct hostent *co_gethostbyname(const char *name);
int co_poll(struct pollfd *fds, unsigned long int nfds, int timeout);

#endif
