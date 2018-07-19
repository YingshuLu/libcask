#ifndef LIBCASK_CID_H_
#define LIBCASK_CID_H_

#include <stddef.h>
typedef unsigned long cid_t;

int cid_allocator_init(size_t max);
void cid_allocator_destory();
cid_t alloc_cid();
void free_cid(cid_t cid);

//thread id
int tid();

#endif
