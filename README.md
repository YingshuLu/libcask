# libcask
## A implement for C coroutine

### Description
Libcask can help simplify the complex in async non-block network programs.
It can easily create millions tasks concurrently, coder can simply use sync
block style to build the application, libcask will help convert this code
implicitly into Reactor mode of network program.

For legacy 3rd-party libraries, libcask can hook them into async flows without
change codes or re-build libraries.

### Basic Features:
* Round robin scheduling coroutines (not support work-steal)
* Hook socket IO syscalls and related glibc APIs
* Coroutine-level sync primitives among multiple-threads: mutex, spinlock, barrier, condition variant, Semaphore
* Async task for unhookable blocked syscalls

### How to build

Platform:
* Linux only (*ubuntu 16.04 TLS recommended*)  
* kernel >= 4.8.0.59  
* gcc version >= 6.2.0   

Build library:  
$ cd libcask  
$ mkdir output  
$ bash build.sh  

then libcask.so would be present in libcask/output

Build tests:  
$ cd libcask/test    
$ bash build_test.sh server [debug]

### Appendix
[libco](https://github.com/Tencent/libco) & [libgo](https://github.com/yyzybb537/libgo) & [libtask](https://swtch.com/libtask/) inspired this C style coroutine library.
