#include "malloc.h"
#include "pool.h"

MEMoid memalloc(size_t size) {
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

static inline void _memfree(MEMoid oid, size_t size) {
    switch(oid.pool_id) {
        case POOL_ID_MALLOC_OBJ:
            free(oid.offset);

        default:
            nvm_free(oid.pool_id, oid.offset, size);
    }
}
