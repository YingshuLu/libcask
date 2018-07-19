#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#define INF_LOG printf


int main() {
    //current_sched();
    int b = 1;
    long a = 12333345;

    long ptr = (long) &a;
    INF_LOG("int size: %lu, long size: %lu, ptr size: %lu, ptr: %p\n", sizeof(b), sizeof(a), sizeof(&a), &a);
    
    long t = *((long*) ptr);

    long long v = 134;
    INF_LOG("long t = %ld, long long size: %lu\n", t, sizeof(v));

    return -1;
}
