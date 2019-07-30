#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "array.h"
#include "list.h"
#include "hash_map.h"

#define list_to_map_node(ls) list_to_struct(ls, map_node_t, link)
#define HASH_MAP_DEFALUT_CAPCITY 16

struct _map_node_st {
    hash_key_t key;
    char keyname[1024];
    void *value;
    list_t link;
};

struct _map_head_st {
    list_t head;
    size_t count; 
};

typedef struct _map_head_st map_head_t;
typedef struct _map_node_st map_node_t;

map_head_t *alloc_map_head() {
    map_head_t *mh = (map_head_t *) malloc(sizeof(map_head_t));
    list_init(&mh->head);
    mh->count = 0;
    return mh;
}

void free_map_head(map_head_t *mh) {
    if(!mh) return;
    list_delete(&mh->head);
    free(mh);
}

map_node_t *alloc_map_node() {
    map_node_t *mn = (map_node_t *) malloc(sizeof(map_node_t));
    mn->key = -1;
    mn->value = NULL;
    list_init(&mn->link);
    return mn;
}

void free_map_node(map_node_t *node) {
    if(!node) return;
    list_delete(&node->link);
    free(node);
}

hash_key_t string_hash(const char* keyname) {
    hash_key_t key = 0;
    int len = strlen(keyname);
    int i;
    for(i = 0; i < len; i++)
        key = key * 31 + keyname[i];
    return key;
}

int hash_map_init(hash_map_t *map) {
    MY_ASSERT(map, "map is NULL");
    map->capcity = HASH_MAP_DEFALUT_CAPCITY;
    map->load_factor = HASH_MAP_LOAD_FACTOR;
    array_init(&map->table);
    map->my_hash = string_hash;
    map->count = 0;

    int i = 0;
    for(; i < map->capcity; i++) {
        array_insert(&map->table, i, alloc_map_head());
    }
    return 0;
}

int hash_map_resize(hash_map_t *map);
int hash_map_get(hash_map_t *map, const char *keyname, void **value) {
    MY_ASSERT(map, "map is NULL");
    hash_key_t key = map->my_hash(keyname);
    int idx = key & (map->capcity - 1);
    int key_len = strlen(keyname);
    //DBG_LOG("key: %d, name: %s, index: %d", key, keyname, idx);

    map_head_t *h = (map_head_t *)array_get(&map->table, idx);
    list_t *ls = list_next(&h->head);
    map_node_t *node = NULL;
    while(ls != &h->head) {
        node = list_to_map_node(ls);
        ls = list_next(ls);
        if(key == node->key && strncmp(keyname, node->keyname, key_len) == 0){
            *value = node->value;
      //      DBG_LOG(" >>> find key <<<");
            return 0;
        }
    }
    return -1;
}

int hash_map_put(hash_map_t *map , const char *keyname, void *value, size_t len) {
    MY_ASSERT(map, "map is NULL");
    hash_key_t key = map->my_hash(keyname);
    int idx = key & (map->capcity -1);
    int key_len = strlen(keyname);

    //DBG_LOG("key: %d, name: %s, index: %d", key, keyname, idx);
    map_head_t *h = (map_head_t *)array_get(&map->table, idx);
    list_t *ls = list_next(&h->head);
    map_node_t *node = NULL;
    while(ls != &h->head) {
        node = list_to_map_node(ls);
        ls = list_next(ls);
      //  DBG_LOG("node keyname: %s", node->keyname);
        if(key == node->key && strncmp(keyname, node->keyname, key_len) == 0) {
            free(node->value);
            node->value = malloc(len);
            memcpy(node->value, value, len);
            return 0;
        }
    }

    if(map->count >= (map->capcity * map->load_factor)) {
        //DBG_LOG("up to load factor, resize hash map");
        hash_map_resize(map);
        idx = key & (map->capcity - 1);
        h = (map_head_t *)array_get(&map->table, idx);
    }

    node = alloc_map_node();
    node->key = key;
    //DBG_LOG("keyname: %s, len: %d", keyname, key_len);
    memcpy(node->keyname, keyname, key_len);
    node->keyname[key_len] = '\0';
    node->value = malloc(len);
    memcpy(node->value, value, len);
    list_add_before(&h->head, &node->link);
    h->count++;
    map->count++;
//    DBG_LOG("head: %d contains count: %d", idx, h->count);
    return 0;
}

int hash_map_del(hash_map_t *map, const char *keyname) {
    MY_ASSERT(map, "map is NULL");
    hash_key_t key = map->my_hash(keyname);
    int idx = key & (map->capcity -1);
    int key_len = strlen(keyname);

    map_head_t *h = (map_head_t *)array_get(&map->table, idx);
    list_t *ls = list_next(&h->head);
    map_node_t *node = NULL;
    while(ls != &h->head) {
        node = list_to_map_node(ls);
        ls = list_next(ls);
        if(key == node->key && strncmp(keyname, node->keyname, key_len) == 0) {
            free(node->value);
            free_map_node(node);
            h->count--;
            map->count--;
            return 0;
        }
    }
    return -1;
}

int hash_map_resize(hash_map_t *map) {
    MY_ASSERT(map, "map is NULL");
    int old_cap = map->capcity;
    int i = old_cap;
    map->capcity = old_cap << 1;

  //  DBG_LOG("count: %lu, old cap: %lu, new cap: %lu", map->count, old_cap, map->capcity);
    for(;i < map->capcity; i++) {
        array_insert(&map->table, i, alloc_map_head());   
    }

    map_head_t *head = NULL;
    map_node_t *node = NULL;
    list_t *ls;
    int idx;
    for(i= 0; i < old_cap; i++) {
         head = (map_head_t *)array_get(&map->table, i);
         ls = list_next(&head->head);
         while(ls != &head->head) {
            node = list_to_map_node(ls);
            ls = list_next(ls);
            idx = node->key & (map->capcity - 1);
            if(idx == i) continue;

            list_delete(&node->link);
            head->count--;
            list_add_before(&(((map_head_t *)array_get(&map->table, idx))->head), &node->link);
            ((map_head_t *)array_get(&map->table, idx))->count++;
         }
    }
    return 0;
}

size_t hash_map_size(hash_map_t *map) {
    MY_ASSERT(map, "map is NULL");
    return map->count;
}

int hash_map_empty(hash_map_t *map) {
    MY_ASSERT(map, "map is NULL");
    return hash_map_size(map) == 0;
}

void hash_map_clearall(hash_map_t *map, int all) {
    MY_ASSERT(map, "map is NULL");
    map_head_t *head = NULL;
    map_node_t *node = NULL;
    list_t *ls;
    int i;
    for(i = map->capcity - 1; i >= 0; i--) {
         head = (map_head_t *)array_get(&map->table, i);
         ls = list_next(&head->head);
         while(!list_empty(ls)) {
            node = list_to_map_node(ls);
            ls = list_next(ls);
            free(node->value);
            free_map_node(node);
            head->count--;
            map->count--;
         }
         if(all) { free_map_head((map_head_t *)array_del(&map->table, i));}
    }
    if(all) {
        map->capcity = 0;
        array_destory(&map->table);
    }

}

void hash_map_clear(hash_map_t *map) {
    hash_map_clearall(map, 0);
    /*
    MY_ASSERT(map, "map is NULL");
    map_head_t *head = NULL;
    map_node_t *node = NULL;
    list_t *ls;
    int i;
    for(i = map->capcity - 1; i >= 0; i--) {
         head = (map_head_t *)array_get(&map->table, i);
         ls = list_next(&head->head);
         while(!list_empty(ls)) {
            node = list_to_map_node(ls);
            ls = list_next(ls);
            free(node->value);
            free_map_node(node);
            head->count--;
            map->count--;
         }
         array_del(&map->table, i);
    }
    */
}

void hash_map_destory(hash_map_t *map) {
    hash_map_clearall(map, 1);
}
