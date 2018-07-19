#!/bin/sh
gcc -fPIC -D CO_DEBUG -D CO_SCHED_STOP -shared -o output/libcask.so *.c -I./ -ldl

