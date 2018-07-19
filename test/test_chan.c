#include "co_define.h"
#include "chan.h"
#include "unistd.h"

chan_t *my_chan;
chan_t *chan;

void go_product(void *p1, void *p2) {
    const char *msg1 = "Hi, I Am Product1";
    const char *msg2 = "Hi, I Am Product2";
    
    chan_send(my_chan, msg1);
    chan_send(my_chan, msg2);

    INF_LOG("send 2 msgs");
    chan_send(my_chan, msg1);
    INF_LOG("send 3 msgs, fin");
}

void go_consume(void *ip, void *op) {
    void *msg;
    INF_LOG("go_consume sleep");
    //sleep(3);
    chan_recv(my_chan, &msg);
    INF_LOG("recv msg: %p", msg);
    INF_LOG("recv 1 msg: %s", (const char *)msg);
    free(msg);
    chan_recv(my_chan, &msg);
    INF_LOG("recv msg: %p", msg);
    INF_LOG("recv 2 msg: %s", (const char *)msg);
    free(msg);
    chan_recv(my_chan, &msg);
    INF_LOG("recv msg: %p", msg);
    INF_LOG("recv 3 msg: %s", (const char *)msg);
    free(msg);
}

int main() {
    my_chan = (chan_t *)malloc(sizeof(chan_t));
    chan_open(my_chan, 2, 17);

    co_create(go_consume, NULL, NULL);
    co_create(go_product, NULL, NULL);
    
    schedule();

    chan_close(my_chan);
    free(my_chan);
    return 0;
}
