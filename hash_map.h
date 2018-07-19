#ifndef LIBCASK_HASH_MAP_H_
#define LIBCASK_HASH_MAP_H_

#include "array.h"

#define HASH_MAP_LOAD_FACTOR 0.75
typedef int hash_key_t;

struct _hash_map_st {
    size_t capcity;
    hash_key_t (*my_hash)(const char*);
    array_t table;
    size_t count;
    float load_factor;
};

typedef struct _hash_map_st hash_map_t;

int hash_map_init(hash_map_t *map);
int hash_map_get(hash_map_t *map, const char *keyname, void **value);
int hash_map_put(hash_map_t *map , const char *keyname, void *value, size_t len); 
int hash_map_del(hash_map_t *map, const char *keyname);
size_t hash_map_size(hash_map_t *map);
int hash_map_empty(hash_map_t *map);
void hash_map_clear(hash_map_t *map);
void hash_map_destory(hash_map_t *map);

#endif
