#include "hash_map.h"
#include "types.h"

int main() {
    hash_map_t map;
    hash_map_init(&map);
    char *key = "test key";
    char *key1 = "test mykey";
    int value = 12;
    int value1 = 13;

    hash_map_put(&map, key, &value,  sizeof(value));
    hash_map_put(&map, key1, &value1,  sizeof(value));
    int *v = NULL;
    hash_map_get(&map, key, &v);

    if(v) 
      INF_LOG("get value: %d", *v);
    
    int i = 0;

    const char *template = "<test_key_%d>";
    char buf[1024] = {0};
    for(; i < 102400; i++) {
        sprintf(buf, template, i);
        INF_LOG("start put key value");
        hash_map_put(&map, buf, &i, sizeof(i));
        INF_LOG("key: %s, put value: %d",buf, i);
    }

    sprintf(buf, template, 1024);
    hash_map_del(&map, buf);

    for(i = 0; i < 1025; i++) {
        sprintf(buf, template, i);
        INF_LOG("start find key value");
        int ret = hash_map_get(&map, buf, &v);
        //MY_ASSERT(ret == 0, "not find");
        if(v) { 
            INF_LOG("key: %s, get value: %d", buf, *v);
            //MY_ASSERT(*v != i, "wrong value in mao");
        }
    }

    INF_LOG("map count: %d", hash_map_size(&map));
    hash_map_clear(&map);
    INF_LOG("map count: %d", hash_map_size(&map));

    hash_map_destory(&map);

    INF_LOG("map capcity: %lu", map.capcity);

    return 0;

}
