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
        pop = pmemobj_create(filename, POBJ_LAYOUT_NAME(types_tab), 5*PMEMOBJ_MIN_POOL, 0666);
        //printf("%d %s %p\n", errno, filename, pop);
        hashmap = POBJ_ROOT(pop, struct hashmap_tx);
        
        struct hashmap_args *args = (struct hashmap_args *) malloc(sizeof(struct hashmap_args));
        args->seed = 8274;  // Just a random number
    #ifdef DEBUG
        printf("hmap create result = %d\n",hm_tx_create(pop, &hashmap, (void *)args));
    #else
        hm_tx_create(pop, &hashmap, (void *)args);
    #endif
    } else {
        //printf("%second d %s %p\n", errno, filename, pop);
        hashmap = POBJ_ROOT(pop, struct hashmap_tx);
    #ifdef DEBUG
        printf("map address %p\n", (hashmap).oid.off);
    #endif
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
    MEMoid m = hm_tx_get(pop, hashmap, (uint64_t) key);
    //printf("memoid poolid = %d, offset = %ld, size = %ld\n", m.pool_id, m.offset, m.size);
    return m;
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

void debug_hashmap(MEMoidKey key) {
    hm_tx_debug(pop, hashmap, stderr);
}