#include "malloc.h"
#include "pool.h"
#include "types.h"
#include "algo.h"

MEMoid __memalloc(size_t size) {
    MEMoid new_obj;

    if (decide_allocation(size) - NVRAM_HEAP == 0) {
        // allocate in NVRAM
        new_obj.pool_id = get_current_poolid();
        new_obj.offset = get_first_free_offset(size);

    } else if (decide_allocation(size) - DRAM_HEAP == 0) {
        // allocate in DRAM
        new_obj.offset = (uint64_t)(malloc(size));
        new_obj.pool_id = POOL_ID_MALLOC_OBJ;
    }
    
    return new_obj;
}

uint64_t string_hash(const char *str) {
    uint64_t hash = 5381;
    int c;
    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

MEMoidKey _memalloc(size_t size, const char *file, const char *func, const int line) {
    // Can be made faster ... but thats an after thought as of now xD
    MEMoidKey key = (((string_hash(file) + string_hash(func)) % __UINT64_MAX__)
                    + line) % __UINT64_MAX__;
    insert_object_to_hashmap(key, __memalloc(size));
    return key;
}

inline void* get_memobj_direct(MEMoid oid) {
    if (oid.offset == NULL || oid.pool_id == 0){
        return NULL;
    }

    switch(oid.pool_id){
        case POOL_ID_MALLOC_OBJ:
            // It is a malloc object
            return (void *)((uintptr_t)oid.offset);

        default:
            // It is a nvm object
            return (void *)((uintptr_t)get_pool_from_poolid(oid.pool_id) + oid.offset);
    }
}

static inline void _memfree(MEMoidKey oidkey, size_t size) {
    MEMoid oid = get_MEMoid(oidkey);
    if(oid != OID_NULL) {
        switch(oid.pool_id) {
            case POOL_ID_MALLOC_OBJ:
                free(oid.offset);
                break;

            default:
                nvm_free(oid.pool_id, oid.offset, size);
        }
    }

    // remove the entry from the HashTable
    remove_object_from_hashmap(oidkey);
}
