#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

void print(const char *fmt, ...) {
   char buffer[1024] = {0};
   
   memcpy(buffer, fmt, strlen(fmt));

   va_list args;
   //va_start(args, fmt);
   vdprintf(2, buffer, args);
   //va_end(args);
}

int main() {
    int fd = 1;
    print("test: %d\n", 1);
    return 0;
}
