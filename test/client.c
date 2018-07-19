#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "co_define.h"

int mykey;

void client(void* ip, void *op) {
    //co_enable_hook();
    //
    int value = getcid();
    const char *mykey = "mykey";
    co_spec_set(mykey, &value, sizeof(int));
    int *v = NULL;
    DBG_LOG("set co value success");
    v = co_spec_get(mykey);
    INF_LOG("get value: %d\n", *v);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in sock_addr;
    socklen_t addr_len = sizeof(sock_addr);
    bzero(&sock_addr, addr_len);

    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(8848);
    sock_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    int ret = connect(sockfd, (struct sockaddr *)&sock_addr, addr_len);
    if(ret) {
        dperror(ret);
        return;
    }

    char* request = "GET / HTTP/1.1\r\nHost: www.tigerso.com\r\n\r\n";
    size_t len = strlen(request);
    while(len) {
        ret = write(sockfd, request, strlen(request));
        if(ret < 0) {
            dperror(ret);
            close(sockfd);
            return;
        }
        len -= ret;
    }
    INF_LOG("send request:\n%s\n", request);

    char* response = (char *) malloc(65536);
    bzero(response, 65536);
    ret = read(sockfd, response, 65536);
    if(ret < 0) {
        dperror(ret);
        return;
    }

    INF_LOG("recv response:\n%s\n", response);
    free(response);
    close(sockfd);
    return;
}



int main() {

    int i = 0;
    for(; i < 1000; i++) {
        co_create(client, NULL, NULL);
    }

    schedule();
    return 0;
}
