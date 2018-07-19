#!/bin/sh

[ $# -lt 1 ] && exit 0
filename=$1

debug="CO_INFO"
ondbg="no"

[ $# -ge 2 ] && [ "$2" == "debug" ] && debug="CO_DEBUG -g" && ondbg="yes"

bin=`echo $filename | awk -F '.' '{print $1}'`

echo ""
echo ">> Test file: $filename"
echo ">> Debug    : $ondbg"
echo ">> Binary   : $bin"
echo ""
gcc -D "$debug" -D CO_SCHED_STOP -D CO_TICKET_SPINLOCK  -g  -w -pthread -o "$bin" $filename ../*.c -I../ -ldl -lssl -lcrypto -lpthread
