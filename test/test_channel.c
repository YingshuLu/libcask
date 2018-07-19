#include "co_define.h"
#include "chan.h"
#include "co_bar.h"

typedef chan_t channel_t;

#define channel_open(chan, n, m) chan_open(chan, n, m)
#define channel_send(chan, msg)  chan_send(chan, msg)
#define channel_recv(chan, msg)  chan_recv(chan, msg)

channel_t *chan;
co_bar_t bar;

void go1(void *p1, void *p2) {
    co_bar_wait(&bar);
    INF_LOG("go1 start\n");
    const char *msg = "hi, I am go1";
    INF_LOG("go1 send msg\n");
    int ret = channel_send(chan, msg);
    INF_LOG("go1 send msg fin\n");
    INF_LOG("go1 recv msg\n");
    void *test;
    INF_LOG("go1 recv msg\n");
    channel_recv(chan, &test);
    msg = (const char *)test;
    INF_LOG("go1 recv msg: %s\n", msg);
}

void go2(void *p1, void *p2) {
    co_bar_wait(&bar);
    INF_LOG("go2 start\n");
    void *msg = NULL;
    const char *msgstr;
    INF_LOG("go2 recv msg\n");
    channel_recv(chan, &msg);
    msgstr = (const char *)msg;
    INF_LOG("go2 recv msg: %s\n", msgstr);
    msgstr = "hi, I am go2";
    INF_LOG("go2 send msg: %s\n", msgstr);
    channel_send(chan, msgstr);
    INF_LOG("go2 send msg fin\n");
}

void go3(void *p1, void *p2) {
    co_bar_wait(&bar);
    INF_LOG("go3 start\n");
    void *msg = NULL;
    const char *msgstr;
    INF_LOG("go3 recv msg\n");
    channel_recv(chan, &msg);
    msgstr = (const char *)msg;
    INF_LOG("go3 recv msg: %s\n", msgstr);
    msgstr = "hi, I am go3";
    INF_LOG("go3 send msg\n", msgstr);
    channel_send(chan, msgstr);
    INF_LOG("go3 send msg fin\n");
}

void *go_thread1(void *param) {
    co_create(go1, NULL, NULL);
    schedule();
}

void *go_thread2(void *param) {
    co_create(go2, NULL, NULL);
    schedule();
}

void *go_thread3(void *param) {
    co_create(go3, NULL, NULL);
    schedule();
}

int main() {

    chan = (channel_t *)malloc(sizeof(channel_t));
    channel_open(chan, 0, 0);
    co_bar_init(&bar, 3);

    co_create(go2, NULL, NULL);
    co_create(go1, NULL, NULL);
    co_create(go3, NULL, NULL);

    pthread_t pid[3];
    pthread_create(pid, NULL, go_thread2);
    pthread_create(pid +1, NULL, go_thread1);
    pthread_create(pid +2, NULL, go_thread3);


    for(int i = 0; i < 3; i++) {
        pthread_join(pid[i], NULL);
        INF_LOG("main thread waited  thread %d", pid[i]);
    }

    return 0;
}
