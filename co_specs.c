#include "co_specs.h"

co_specs_t *alloc_co_specs() {
    co_specs_t *specs = (co_specs_t *) malloc(sizeof(co_specs_t));
    hash_map_init(&specs->map);
    return specs;
}

int co_specs_set(co_specs_t *specs, const char *key, void *value, size_t len) {
    return hash_map_put(&specs->map, key, value, len);
}

int co_specs_get(co_specs_t *specs, const char *key, void **value) {
    return hash_map_get(&specs->map, key, value);
}
int co_specs_del(co_specs_t *specs, const char *key) {
    return hash_map_del(&specs->map, key);
}

void free_co_specs(co_specs_t *specs) {
    if(!specs) return;
    hash_map_destory(&specs->map);
    free(specs);
}

