#include "cid.h"
#include "stdio.h"

int main() {
    int i = 0;
    int cid = 0;
    for(; i <= 100; i++) {
        cid = alloc_cid();
        printf("push cid: %d\n", cid);
        free_cid(cid);
    }
    for(i = 0; i <= 100; i++) {
        cid = alloc_cid();
        printf("cid: %d\n", cid);
    }

    printf("thread id: %d\n", tid());

    return 0;
}
