#include "specific.h"
#include <stdio.h>

void df(void *v) {
    printf("destory value : %d\n", *((int*)v));
    free(v);
}

int main() {
    specific_ctl_t* specs = specific_ctl_create();

    int i = 0;
    int* value;
    for(; i < 1024; i++) {
    value = (int*)malloc(sizeof(int));
    *value = i;
    specific_key_t key;
    specific_key_create(specs, &key, df);
    specific_set(specs, key, value);

    void *v = specific_get(specs, key);
    if(v)
        printf("key: %d, value: %d\n", key, *((int*)v));
    }
    specific_ctl_destory(specs);
    return 0;

}
