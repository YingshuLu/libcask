#define swap(a, b) do {\
    a = a ^ b;\
    b = a ^ b;\
    a = a ^ b;\
} while(0)


#include <stdio.h>

int main() {
    int a = 12;
    int b = 23;
    
    printf("a: %d, b: %d\n", a, b);
    swap(a, b);
    printf("a: %d, b: %d\n", a, b);
    return 0;

}
