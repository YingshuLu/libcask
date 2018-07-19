#include "heap.h"
#include <stdio.h>
#include "types.h"

int cmp(void *l, void *r) {
    int lv =(int )l;
    int rv = (int )r;
    return lv < rv;
}

int main() {

    heap_t hp;

    int array[] = {11, 3,55,12,43,2,7,89,23, 1, 100, 12, 123,54,2,12, 32,68,9,53,89,23,908,232,125,753,234,657,34,212,45,246,896,321,456,345,234,231,345, 12,12,123,12311,34552,122231,3799563,121673};


    heap_init(&hp, cmp);
    int i = 0;
    for(; i < sizeof(array) / sizeof(int); i++) {
        void *v = array[i];
        heap_push(&hp, v);
    }
    
    void *elm;
    while(!heap_empty(&hp)) {
        heap_pop(&hp, (&elm));
        INF_LOG("heap elm: %d\n", (int)elm);
    }
    return 0;
    
}
