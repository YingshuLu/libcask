#include "array.h"
#include "heap.h"
#include "types.h"

#define parent(idx) (idx <= 0? 0 : (idx - 1)>>1)   
#define left_child(idx) ((idx<<1)+1)
#define right_child(idx) ((idx<<1)+2)
#define heap_get(hp, idx) array_get(&(hp->array), idx)
#define heap_put(hp, idx, elm) array_put(&(hp->array), idx, elm)

int swap(heap_t *h, int l, int r) {
    void *tmp = heap_get(h, l);
    heap_put(h, l, heap_get(h, r));
    heap_put(h, r, tmp);
    return 0;
} 

int heapify(heap_t *hp, int root) {
    int size  = heap_size(hp);
    if(!(root < size)) return 0;

    int left  = left_child(root) < size? left_child(root) : -1;
    int right = right_child(root) < size? right_child(root) : -1;
    int max = root;
    
    if(left > 0 && hp->cmp(heap_get(hp, left), heap_get(hp, max))) max = left;
    if(right > 0 && hp->cmp(heap_get(hp, right), heap_get(hp, max))) max = right;
    if(max != root) swap(hp, max, root);

    return max;
}

int heap_fixup(heap_t *hp) {
    int start = parent(heap_size(hp) - 1);
    for(; start >= 0; start--) {
        heapify(hp, start);
    }
    return 0;
}

int heap_fixdown(heap_t *hp) {
    int stop = parent(heap_size(hp) - 1);
    int root = 0;
    int max;
    while(root <= stop) {
        max = heapify(hp, root);
        if(max == root) break;
        root = max; 
    }
    return 0;
}

int heap_push(heap_t *hp, void *elm) {
    array_insert(&(hp->array), heap_size(hp), elm);
    return heap_fixup(hp);
}

int heap_pop(heap_t *hp, void **elm) {
    if(!heap_size(hp)) return -1;
    *elm = heap_get(hp, 0);
    heap_put(hp, 0, array_del(&(hp->array), heap_size(hp)-1));
   // heap_fixup(hp);
    heap_fixdown(hp);
    return 0;
}
