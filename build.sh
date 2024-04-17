#!/bin/sh

rm -rf output
mkdir output

gcc -D CO_DEBUG  -fPIC -std=gnu11 -shared -o output/libcask.so *.c -I./ -ldl

