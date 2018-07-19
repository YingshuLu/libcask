#ifndef LIBCASK_SPECS_H_
#define LIBCASK_SPECS_H_

#include <stdlib.h>
#include "hash_map.h"

struct _co_specs_st{
    hash_map_t map;
};

typedef struct _co_specs_st co_specs_t;

co_specs_t *alloc_co_specs();
int co_specs_set(co_specs_t *specs, const char *key, void *value, size_t len);
int co_specs_get(co_specs_t *specs, const char *key, void **value);
int co_specs_del(co_specs_t *specs, const char *key);
void free_co_specs(co_specs_t *specs);

#endif
