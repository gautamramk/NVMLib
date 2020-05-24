#include "types.h"
#include "hashmap_tx.h"
#include <libpmemobj.h>
#include <errno.h>

// The pool that would contain this HashTable
PMEMobjpool *pop;


// The stored HashTable
TOID(struct hashmap_tx) hashmap;

void init_types_table() {
    char filename[50];
    strcpy(filename, program_invocation_short_name);
    strcat(filename, "_types_table");
    pop = pmemobj_open(filename, POBJ_LAYOUT_NAME(types_tab));

    if(pop == NULL) {
        pop = pmemobj_create(filename, POBJ_LAYOUT_NAME(types_tab), PMEMOBJ_MIN_POOL, 0666);
        printf("%d %s %p\n", errno, filename, pop);
        hashmap = POBJ_ROOT(pop, struct hashmap_tx);
        
        struct hashmap_args *args = (struct hashmap_args *) malloc(sizeof(struct hashmap_args));
        args->seed = 8274;  // Just a random number
        printf("hmap create result = %d\n",hm_tx_create(pop, &hashmap, (void *)args));
    } else {
        hashmap = POBJ_ROOT(pop, struct hashmap_tx);
        hm_tx_init(pop, hashmap);
    }
}

// void* _key_get_first(MEMoidKey key) {
//     MEMoid m = get_MEMoid(key);
//     if (m != MEMOID_NULL) {
//         return MEMOID_FIRST(m);
//     }
//     return NULL;
// }

// void* _key_get_last(MEMoidKey key) {
//     MEMoid m = get_MEMoid(key);
//     if (m != MEMOID_NULL) {
//         return MEMOID_FIRST(m) + m.size;
//     }
//     return NULL;
// }

MEMoid get_MEMoid(MEMoidKey key) {
    return hm_tx_get(pop, hashmap, (uint64_t) key);
}

void insert_object_to_hashmap(MEMoidKey key, MEMoid oid) {
    hm_tx_insert(pop, hashmap, (uint64_t)key, oid);
}

void remove_object_from_hashmap(MEMoidKey key) {
    hm_tx_remove(pop, hashmap, (uint64_t)key);
}

TOID(struct hashmap_tx)* get_types_map(){
    return &hashmap;
}