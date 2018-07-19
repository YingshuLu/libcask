#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "co_define.h"

void handler(void* ip, void *op) {
    co_enable_hook();
    int sockfd = (int)ip;
    char *buf = (char *)malloc(65536);
    int ret = read(sockfd, buf, 65536);
    if(ret < 0)  {
        dperror(ret);
        goto handler_error;
    }

    DBG_LOG("recv request:\n%s\n", buf);
    char *response ="HTTP/1.1 200 OK\r\nServer:libcask/1.0\r\n";
    ret = write(sockfd, response, strlen(response));
    if(ret < 0)  dperror(ret);
    else {
        DBG_LOG("send response:\n%s\n", response);
    }
    
handler_error:
    free(buf);
    close(sockfd);
    return;
}

void dispatch(void* ip, void *op) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in sock_addr;
    socklen_t addr_len = sizeof(sock_addr);
    bzero(&sock_addr, addr_len);

    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(8848);
    sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    int ret = bind(sockfd, (struct sockaddr *)&sock_addr, addr_len);
    if(ret) goto server_error;
    ret = listen(sockfd, 1024);
    if(ret) goto server_error;

    struct sockaddr_in client_addr;
    socklen_t caddr_len = sizeof(client_addr);
    task_t *t = NULL;
    while(1) {
        bzero(&client_addr, caddr_len);
        ret = accept(sockfd, (struct sockaddr *)&client_addr, &caddr_len);
        if(ret < 0) goto server_error;
        co_create(handler, (void *)ret, NULL);
    }
server_error:
    dperror(ret);
    close(sockfd);
    return;
}

int main() {
    co_create(dispatch, NULL, NULL);
    schedule();
    return 0;
}
