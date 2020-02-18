#include "malloc.h"
#include "pool.h"
#include "types.h"

MEMoid _memalloc(size_t size) {
    MEMoid new_obj;

    if (decide_allocation(size) - NVRAM_HEAP == 0) {
        // allocate in NVRAM
        new_obj.pool_id = get_current_poolid();
        new_obj.offset = get_current_free_offset(size);

    } else if (decide_allocation() - DRAM_HEAP == 0) {
        // allocate in DRAM
        new_obj.offset = (uint64_t)(malloc(size));
        new_obj.pool_id = POOL_ID_MALLOC_OBJ;
    }
    
    return new_obj;
}

MEMoidKey memalloc(size_t size) {
    // get the key
    MEMoidKey key;
    insert_object_to_hashmap(key, _memalloc(size));
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
    switch(oid.pool_id) {
        case POOL_ID_MALLOC_OBJ:
            free(oid.offset);
            break;

        default:
            nvm_free(oid.pool_id, oid.offset, size);
    }

    // remove the entry from the HashTable
    remove_object_from_hashmap(oidkey);
}
