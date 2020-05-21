#include "malloc.h"
#include "pool.h"
#include "types.h"
#include "algo.h"
#include "math.h"
#include <stdint.h>
#include <libiberty/splay-tree.h>
#include "object_maintainance.h"

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
    new_obj.size = size;

    // new_obj.access_bitmap = (uint64_t*)malloc((ceil((double)size/64));
    return new_obj;
}

MEMoid __memalloc(size_t size, uint8_t which_ram) {
    MEMoid new_obj;

    if (which_ram - NVRAM_HEAP == 0) {
        // allocate in NVRAM
        new_obj.pool_id = get_current_poolid();
        new_obj.offset = get_first_free_offset(size);

    } else if (which_ram - DRAM_HEAP == 0) {
        // allocate in DRAM
        new_obj.offset = (uint64_t)(malloc(size));
        new_obj.pool_id = POOL_ID_MALLOC_OBJ;
    }
    new_obj.size = size;

    // new_obj.access_bitmap = (uint64_t*)malloc((ceil((double)size/64));
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
    MEMoid oid =  __memalloc(size);
    // Insert in the main types table
    insert_object_to_hashmap(key, oid);
    // Insert into the object maintainance table for logistics
    insert_into_maintainance_map(create_new_maintainance_map_entry(key, oid, oid.pool_id==POOL_ID_MALLOC_OBJ?DRAM:NVRAM, true));
    struct addr2memoid_key* new_key = (struct addr2memoid_key*)malloc(sizeof(addr2memoid_key));
    new_key->comp = cmp_node;
    new_key->key = key;
    splay_tree_insert(addr2MemOID, new_key, NULL);
    return key;
}

MEMoidKey _memalloc(size_t size, uint8_t which_ram, const char *file, const char *func, const int line) {
    // Can be made faster ... but thats an after thought as of now xD
    MEMoidKey key = (((string_hash(file) + string_hash(func)) % __UINT64_MAX__)
                    + line) % __UINT64_MAX__;
    MEMoid oid =  __memalloc(size);
    // Insert in the main types table
    insert_object_to_hashmap(key, oid);
    // Insert into the object maintainance table for logistics
    insert_into_maintainance_map(create_new_maintainance_map_entry(key, oid, oid.pool_id==POOL_ID_MALLOC_OBJ?DRAM:NVRAM, false));
    struct addr2memoid_key* new_key = (struct addr2memoid_key*)malloc(sizeof(addr2memoid_key));
    new_key->comp = cmp_node;
    new_key->key = key;
    splay_tree_insert(addr2MemOID, new_key, NULL);
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
    if(oid.offset == MEMOID_NULL.offset && oid.pool_id == MEMOID_NULL.pool_id) {
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


#define MEMOID_FIRST(m) (get_pool_from_poolid(m.pool_id) + m.offset)
#define MEMOID_LAST(m) (get_pool_from_poolid(m.pool_id) + m.offset + m.size)

void* _key_get_first(MEMoidKey key) {
    MEMoid m = get_MEMoid(key);
    if (m.offset == MEMOID_NULL.offset && m.pool_id == MEMOID_NULL.pool_id) {
        return MEMOID_FIRST(m);
    }
    return NULL;
}

void* _key_get_last(MEMoidKey key) {
    MEMoid m = get_MEMoid(key);
    if (m.offset == MEMOID_NULL.offset && m.pool_id == MEMOID_NULL.pool_id) {
        return MEMOID_FIRST(m) + m.size;
    }
    return NULL;
}

int addr2memoid_cmp(splay_tree_key key1, splay_tree_key key2) {
    if (((addr2memoid_key*)key2)->comp == cmp_node) {
        if (KEY_FIRST(((addr2memoid_key*)key1)->key) == KEY_FIRST(((addr2memoid_key*)key2)->key))
            return 0;
        else
            return KEY_FIRST(((addr2memoid_key*)key1)->key) > KEY_FIRST(((addr2memoid_key*)key1)->key)?1:-1;

    }else if (((addr2memoid_key*)key2)->comp == cmp_addr) {
        if (((addr2memoid_key*)key2)->addr >= KEY_FIRST(((addr2memoid_key*)key1)->key) &&
            ((addr2memoid_key*)key2)->addr < KEY_LAST(((addr2memoid_key*)key1)->key))
            return 0;
        else
            return KEY_FIRST(((addr2memoid_key*)key1)->key) > ((addr2memoid_key*)key1)->addr?1:-1;
    }
}

void addr2memoid_del(splay_tree_key key) {
    free((void*)key);
}

// The splay tree is used for reverse mapping
// from address to MemOiD
static void init_splay() {
    addr2MemOID = splay_tree_new(*addr2memoid_cmp, *addr2memoid_del, NULL);
}
