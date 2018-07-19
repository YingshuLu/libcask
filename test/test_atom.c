#include "atomic.h"
#include "types.h"

int main() {
    
    atomic_t a;

    atomic_init(&a);
    INF_LOG("atomic int value: %d", atomic_get(&a));
    atomic_inc(&a);
    atomic_add(&a, 21);
    INF_LOG("atomic int value: %d", atomic_get(&a));
    atomic_sub(&a, 14);
    atomic_sub_and_test(&a, 12);
    atomic_inc_and_test(&a);
    atomic_set(&a, -2);
    if(atomic_inc_and_negative(&a))
      INF_LOG("atomic inc test negative : true");

    INF_LOG("atomic int value: %d", atomic_get(&a));
    
    return atomic_get(&a);
}
