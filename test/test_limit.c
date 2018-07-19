#include <sys/time.h>
#include <sys/resource.h>
#include <stdio.h>


int main() {

    printf("ulimit: %d\n", RLIM_INFINITY);
    return 0;

}
