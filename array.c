#include <string.h>
#include "array.h"

#define MIN_ARRAY_CAPCITY 256

int array_init(array_t* array) {
    if(!array) return -1;
    array->vector = NULL;
    array->size = 0;
    array->capcity = 0;
    //array_destory(array);
    return 0;
}

void array_destory(array_t *array) {
    if(array->vector) {
        free((void *)array->vector);
        array->vector = NULL;
    }
    array->size = 0;
    array->capcity = 0;
}

int realloc_array(array_t *array) {
    if(!array) return -1;

    if(!array->vector) {
        array->size = 0;
        array->capcity = array->capcity? array->capcity : MIN_ARRAY_CAPCITY;
        array->vector = (void **)malloc(sizeof(void *) * array->capcity);
        return 0;
    }

    void **ta = array->vector;
    array->capcity <<= 1; 
    array->vector = (void **)malloc(sizeof(void *) * array->capcity);
    int i = 0;
    for(; i < array->size; i++)
        array->vector[i] = ta[i];
    //if(array->size) memcpy(array->vector, ta, array->size);
    free(ta);
    return 0; 
}

size_t array_size(array_t* array) {
    return array->size;
}

int array_insert(array_t *array, size_t idx, void *elm) {
    if(!array || idx > array->size) return -1;
    if(!array->vector || array->capcity <= array->size) {
        realloc_array(array);
    }

    size_t loc = idx;
    while(loc < array->size) {
        array->vector[loc + 1] = array->vector[loc];
        loc++;
    }

    array->vector[idx] = elm;
    (array->size)++;
    return 0;
}

int array_put(array_t *array, size_t idx, void *elm) {
    if(!array || !(idx < array_size(array))) return -1; 
    array->vector[idx] = elm;
    return 0;
}

void* array_get(array_t *array, size_t idx) {
    if(!array || idx >= array->size) return NULL;
    return array->vector[idx];
}

void* array_del(array_t *array, size_t idx) {
    if(!array || idx >= array->size) return NULL;
    void * elm = array->vector[idx];
    while(idx < array->size - 1) {
        array->vector[idx] = array->vector[++idx];
    }
    array->size--;
    return elm;
}

