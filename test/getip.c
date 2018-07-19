#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "co_define.h"

void getip(void *ip, void *op) {
    //co_disable_hook();
    const char *host = (const char *)ip;
    INF_LOG("DNS query host: %s", host);
    struct hostent *net = gethostbyname(host);
    DBG_LOG("gethostbyname complete");
    if(net) {
        const char *h = net->h_addr_list[0];
        int i = 0;
        do {
            INF_LOG("host: %s, resolve at address: %s", host, inet_ntoa(*((struct in_addr*)(net->h_addr_list)[i])));
            i++;
            h = net->h_addr_list[i];
        } while(h);
    }
    else {
        INF_LOG("failed to resolve host: %s", host);
    }
}

const char *hosts[] = { 
    "www.baidu.com",
    "www.github.com",
    "www.sina.com",
    "www.google.com",
    "www.163.com",
    "yingshulu.github.io",
    "www.trendmicro.com",
    "www.zte.com",
    "www.hradsdewerwer.com",
    "test.yingshulu.net",
    "www.qq.com",
    "code.woboq.org",
    "asm.sourceforge.net"
};



void *routine(void * ip) {
    int cnt = sizeof(hosts) / sizeof(char *);
    int i;
    for(i = 0; i < cnt; i++) {
        co_create(getip, hosts[i%cnt], NULL);
        
    }

    schedule();
    return 0;
}


int main() {
    
    const int thread_num = 1;
    pthread_t tid[thread_num];

    /*
    for(int i = 0; i < thread_num; i++) {
        pthread_create(&tid[i], NULL, routine, NULL);
    }

    for(int i = 0; i < thread_num; i++) {
        pthread_join(tid[i], NULL);
    }
    */
    routine(NULL);
    
    return 0;
}
