#include <strings.h>
#include <stdlib.h>
#include "bit_map.h"

#define BIT_MAP_BASE 8
#define BIT_MAP_DEFAULT_MAX_NUM ((BIT_MAP_BASE << 3) - 1)

static size_t get_bit_map_size(const unsigned int max_num) {
    size_t mn = max_num < BIT_MAP_DEFAULT_MAX_NUM? BIT_MAP_DEFAULT_MAX_NUM : max_num;
    size_t count = 0;
    size_t base = BIT_MAP_BASE;
    while((base <<= 1) < max_num) {
        count++;
    }
    return count;
}

#define get_bit_map_index(num) (num / BIT_MAP_BASE)
#define get_bit_map_loc(num) (char)(num & (BIT_MAP_BASE - 1))
#define set_bit(c, n) do { (c) |= (1 << n); } while(0)
#define unset_bit(c, n) do { (c) &= ~(1 << n); } while(0)
#define get_bit(c, n) ((c) & (1 << n))

void bit_map_init(bit_map_t* map, const unsigned int max_num) {
    if (map == NULL) return;
    map->size = get_bit_map_size(max_num);
    map->map = (char*) calloc(map->size, 1);
}

int bit_map_set(bit_map_t* map, const unsigned int num) {
    if (map == NULL || num >= BIT_MAP_BASE * map->size) return -1;
    size_t idx = get_bit_map_index(num);
    char loc = get_bit_map_loc(num);
    set_bit(map->map[idx], loc);
    return 0;
}

int bit_map_unset(bit_map_t* map, const unsigned int num) {
    if (map == NULL || num >= BIT_MAP_BASE * map->size) return -1;
    size_t idx = get_bit_map_index(num);
    char loc = get_bit_map_loc(num);
    unset_bit(map->map[idx], loc);
    return 0;
}

int bit_map_get(bit_map_t* map, const unsigned int num) {
    if (map == NULL || num >= BIT_MAP_BASE * map->size) return -1;
    size_t idx = get_bit_map_index(num);
    char loc = get_bit_map_loc(num);
    return get_bit(map->map[idx], loc);
}

unsigned int bit_map_get_max_num(bit_map_t* map) {
    if (map == NULL) return 0;
    return (map->size * BIT_MAP_BASE) - 1;
}

void bit_map_clear(bit_map_t* map) {
    if (map == NULL) return;
    bzero(map->map, map->size);
}

void bit_map_destory(bit_map_t* map) {
    if (map == NULL) return;
    free(map->map);
    map->map = NULL;
    map->size = 0;
}
