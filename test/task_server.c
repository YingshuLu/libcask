#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "co_define.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define MAXBUF 1024

int main(int argc, char **argv)
{
    int sockfd, new_fd;
    socklen_t len;
    struct sockaddr_in my_addr, their_addr;
    unsigned int myport, lisnum;
    char buf[MAXBUF + 1];
    SSL_CTX *ctx;

    myport = 443;
    lisnum = 1024;

    /* SSL 库初始化*/
    SSL_library_init();

    /* 载入所有SSL 算法*/
    OpenSSL_add_all_algorithms();

    /* 载入所有SSL 错误消息*/
    SSL_load_error_strings();

    /* 以SSL V2 和V3 标准兼容方式产生一个SSL_CTX ，即SSL Content Text */
    //ctx = SSL_CTX_new(SSLv23_server_method());
    ctx = SSL_CTX_new(TLSv1_2_server_method());
    printf("new ctx\n");

    /* 也可以用SSLv2_server_method() 或SSLv3_server_method() 单独表示V2 或V3
     * 标准*/
    if (ctx == NULL) {
        ERR_print_errors_fp(stdout);
        exit(1);
    }

    const char* cert = "cask.cert";
    const char* key = "cask.pkey";
    /* 载入用户的数字证书， 此证书用来发送给客户端。证书里包含有公钥*/
    if (SSL_CTX_use_certificate_file(ctx, cert, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stdout);
        exit(1);
    }

    /* 载入用户私钥*/
    if (SSL_CTX_use_PrivateKey_file(ctx, key, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stdout);
        exit(1);
    }

    /* 检查用户私钥是否正确*/
    if (!SSL_CTX_check_private_key(ctx)) {
        ERR_print_errors_fp(stdout);
        exit(1);
    }

    /* 开启一个socket 监听*/
    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    } else
        printf("socket created\n");

    bzero(&my_addr, sizeof(my_addr));
    my_addr.sin_family = PF_INET;
    my_addr.sin_port = htons(myport);

    if (argv[3])
        my_addr.sin_addr.s_addr = inet_addr(argv[3]);
    else
        my_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr))
            == -1) {
        perror("bind");
        exit(1);
    } else
        printf("binded\n");

    if (listen(sockfd, lisnum) == -1) {
        perror("listen");
        exit(1);
    } else
        printf("begin listen\n");

    while (1) {
        SSL *ssl;
        len = sizeof(struct sockaddr);

        /* 等待客户端连上来*/
        if ((new_fd =
                    accept(sockfd, (struct sockaddr *) &their_addr,
                           &len)) == -1) {
            perror("accept");
            exit(errno);
        } else
            printf("server: got connection from %s, port %d, socket %d\n",
                   inet_ntoa(their_addr.sin_addr),
                   ntohs(their_addr.sin_port), new_fd);

        /* 基于ctx 产生一个新的SSL */
        ssl = SSL_new(ctx);

        /* 将连接用户的socket 加入到SSL */
        SSL_set_fd(ssl, new_fd);

        /* 建立SSL 连接*/
        if (SSL_accept(ssl) == -1) {
            perror("accept");
            close(new_fd);
            break;
        }

        /* 开始处理每个新连接上的数据收发*/
        bzero(buf, MAXBUF + 1);
        strcpy(buf, "server->client");

        /* 发消息给客户端*/
        len = SSL_write(ssl, buf, strlen(buf));
        if (len <= 0) {
            printf
            ("消息'%s'发送失败！错误代码是%d，错误信息是'%s'\n",
             buf, errno, strerror(errno));
            goto finish;
        } else
            printf("消息'%s'发送成功，共发送了%d 个字节！\n",
                   buf, len);
        bzero(buf, MAXBUF + 1);

        /* 接收客户端的消息*/
        len = SSL_read(ssl, buf, MAXBUF);
        if (len > 0)
            printf("接收消息成功:'%s'，共%d 个字节的数据\n",
                   buf, len);
        else
            printf
            ("消息接收失败！错误代码是%d，错误信息是'%s'\n",
             errno, strerror(errno));
        /* 处理每个新连接上的数据收发结束*/

finish:
        /* 关闭SSL 连接*/
        SSL_shutdown(ssl);

        /* 释放SSL */
        SSL_free(ssl);

        /* 关闭socket */
        close(new_fd);
    }

    /* 关闭监听的socket */
    close(sockfd);

    /* 释放CTX */
    SSL_CTX_free(ctx);

    return 0;
}

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
    //stop_event_loop(current_thread_epoll());
    
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
