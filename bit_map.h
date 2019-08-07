#ifndef LIBCASK_BIT_MAP_H_
#define LIBCASK_BIT_MAP_H_

#include <stdlib.h>

struct __co_bit_map {
    char* map;
    size_t size;
};

typedef struct __co_bit_map bit_map_t;

void bit_map_init(bit_map_t* map, const unsigned int max_num);
int bit_map_set(bit_map_t* map, const unsigned int num);
int bit_map_unset(bit_map_t* map, const unsigned int num);
int bit_map_get(bit_map_t* map, const unsigned int num);
unsigned int bit_map_max_num(bit_map_t* map);
void bit_map_clear(bit_map_t* map);
void bit_map_destory(bit_map_t* map);

#endif

