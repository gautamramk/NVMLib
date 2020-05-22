#include "malloc.h"
#include "types.h"
#include "algo.h"
#include "math.h"
#include <stdint.h>
#include <libiberty/splay-tree.h>
#include "object_maintainance.h"

// MEMoid __memalloc(size_t size) {
//     MEMoid new_obj;

//     if (decide_allocation(size) - NVRAM_HEAP == 0) {
//         // allocate in NVRAM
//         new_obj.pool_id = get_current_poolid();
//         new_obj.offset = get_first_free_offset(size);

//     } else if (decide_allocation(size) - DRAM_HEAP == 0) {
//         // allocate in DRAM
//         new_obj.offset = (uint64_t)(malloc(size));
//         new_obj.pool_id = POOL_ID_MALLOC_OBJ;
//     }
//     new_obj.size = size;

//     // new_obj.access_bitmap = (uint64_t*)malloc((ceil((double)size/64));
//     return new_obj;
// }
splay_tree addr2MemOID_read;
splay_tree addr2MemOID_write;

uint64_t free_slot_size(TOID(struct pool_free_slot) slot) {
    return D_RO(slot)->end_b - D_RO(slot)->start_b + 1;
}

uint64_t allot_first_free_offset_pool(uint64_t pool_id, size_t size) {
    // LOG shit
    pool_free_slot_val temp;
    pool_free_slot_val* temp_ptr = &temp;
    temp.key = pool_id;
    HASH_MAP_FIND(pool_free_slot_val)(pool_free_slot_map, &temp_ptr);
    pool_free_slot_head *f_head = &(temp_ptr->head);
    uint64_t ret = -1;
    TOID(struct pool_free_slot) node;
    POBJ_TAILQ_FOREACH (node, f_head, fnd) {
        if (free_slot_size(node) == size) {
            ret = D_RO(node)->start_b;
            TX_BEGIN(temp_ptr->pool) {
                POBJ_TAILQ_REMOVE_FREE(f_head, node, fnd);
            } TX_END
            break;
        } else if (free_slot_size(node) > size) {
            ret = D_RO(node)->start_b;
            uint64_t new_start = D_RO(node)->start_b + size;
            TX_BEGIN(temp_ptr->pool) {
                D_RW(node)->start_b = new_start;
            } TX_END
            break;
        }
    }
    return ret;
}

MEMoid allot_first_free_offset(size_t size) {
    for (int i = 0; i < num_pools; i++) {
        uint64_t ret = allot_first_free_offset_pool(i, size);
        if (ret >= 0) {
            MEMoid m;
            m.pool_id = i;
            m.offset = ret;
            m.size = size;
            return m;
        }
    }
    create_new_pool();
    uint64_t ret = allot_first_free_offset_pool(num_pools, size);
    MEMoid m;
    m.pool_id = num_pools;
    m.offset = ret;
    return m;
}

MEMoid __memalloc(size_t size, int which_ram) {
    MEMoid new_obj;

    switch(which_ram) {
        case ANY_RAM:
            if (decide_allocation(size) - NVRAM_HEAP == 0) {
                // allocate in NVRAM
                //new_obj.pool_id = get_current_poolid();
                //new_obj.offset = get_first_free_offset(size);
                new_obj = allot_first_free_offset(size);

            } else if (decide_allocation(size) - DRAM_HEAP == 0) {
                // allocate in DRAM
                new_obj.offset = (uint64_t)(malloc(size));
                new_obj.pool_id = POOL_ID_MALLOC_OBJ;
            }
            break;

        case NVRAM_HEAP:
            // allocate in NVRAM
            //new_obj.pool_id = get_current_poolid();
            //new_obj.offset = get_first_free_offset(size);
            new_obj = allot_first_free_offset(size);
            break;

        case DRAM_HEAP:
            // allocate in DRAM
            new_obj.offset = (uint64_t)(malloc(size));
            new_obj.pool_id = POOL_ID_MALLOC_OBJ;
            break;

        default:
            break;
    }
    new_obj.size = size;

    // new_obj.access_bitmap = (uint64_t*)malloc((ceil((double)size/64));
    return new_obj;
}

uint64_t string_hash(const char *str) {
    uint64_t hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

MEMoidKey _memalloc(size_t size, const char *file, const char *func, const int line, int num_args, ...) {
    // Can be made faster ... but thats an after thought as of now xD
    MEMoidKey key = (((string_hash(file) + string_hash(func)) % __UINT64_MAX__)
                    + line) % __UINT64_MAX__;

    int which_ram = ANY_RAM;
    va_list arg_list;
    va_start(arg_list, num_args);
    while(num_args--) {
        which_ram = va_arg(arg_list, int);
    }

    MEMoid oid =  __memalloc(size, which_ram);
    // Insert in the main types table
    insert_object_to_hashmap(key, oid);
    // Insert into the object maintainance table for logistics
    insert_into_maintainance_map(create_new_maintainance_map_entry(key, oid, oid.pool_id==POOL_ID_MALLOC_OBJ?DRAM:NVRAM, true));
    struct addr2memoid_key* new_key = (struct addr2memoid_key*)malloc(sizeof(addr2memoid_key));
    new_key->comp = cmp_node;
    new_key->key = key;
    splay_tree_insert(addr2MemOID_read, (uintptr_t)new_key, NULL);
    splay_tree_insert(addr2MemOID_write, (uintptr_t)new_key, NULL);
    return key;
}

// MEMoidKey _memalloc(size_t size, uint8_t which_ram, const char *file, const char *func, const int line) {
//     // Can be made faster ... but thats an after thought as of now xD
//     MEMoidKey key = (((string_hash(file) + string_hash(func)) % __UINT64_MAX__)
//                     + line) % __UINT64_MAX__;
//     MEMoid oid =  __memalloc(size);
//     // Insert in the main types table
//     insert_object_to_hashmap(key, oid);
//     // Insert into the object maintainance table for logistics
//     insert_into_maintainance_map(create_new_maintainance_map_entry(key, oid, oid.pool_id==POOL_ID_MALLOC_OBJ?DRAM:NVRAM, false));
//     struct addr2memoid_key* new_key = (struct addr2memoid_key*)malloc(sizeof(addr2memoid_key));
//     new_key->comp = cmp_node;
//     new_key->key = key;
//     splay_tree_insert(addr2MemOID, new_key, NULL);
//     return key;
// }

void* get_memobj_direct(MEMoid oid) {
    if (oid.offset == 0 || oid.pool_id == 0){
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

// static inline void _memfree(MEMoidKey oidkey, size_t size) {
//     MEMoid oid = get_MEMoid(oidkey);
//     if(oid.offset == MEMOID_NULL.offset && oid.pool_id == MEMOID_NULL.pool_id) {
//         switch(oid.pool_id) {
//             case POOL_ID_MALLOC_OBJ:
//                 free(oid.offset);
//                 break;

//             default:
//                 nvm_free(oid.pool_id, oid.offset, size);
//         }
//     }

//     // remove the entry from the HashTable
//     remove_object_from_hashmap(oidkey);
// }

static inline void _memfree(MEMoidKey oidkey) {
    // Just hand over the task to `Deletion thread`

    object_maintainance *found_obj = find_in_maintainance_map(oidkey);
    if (!found_obj)
        return;

    found_obj->which_ram = NO_RAM; // Delete the object
}



#define MEMOID_FIRST(m) (get_pool_from_poolid(m.pool_id) + m.offset) //need to use this in object_maintenence.c also, move to malloc.h?
#define MEMOID_LAST(m) (get_pool_from_poolid(m.pool_id) + m.offset + m.size)

void* _key_get_first(MEMoidKey key) {
    MEMoid m = get_MEMoid(key);
    if (m.offset == MEMOID_NULL.offset && m.pool_id == MEMOID_NULL.pool_id) {
        return (void *)MEMOID_FIRST(m);
    }
    return NULL;
}

void* _key_get_last(MEMoidKey key) {
    MEMoid m = get_MEMoid(key);
    if (m.offset == MEMOID_NULL.offset && m.pool_id == MEMOID_NULL.pool_id) {
        return (void *)MEMOID_FIRST(m) + m.size;
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
    return 0;
}

void addr2memoid_del(splay_tree_key key) {
    free((void*)key);
}

// The splay tree is used for reverse mapping
// from address to MemOiD
void init_splay() {
    addr2MemOID_read = splay_tree_new(*addr2memoid_cmp, *addr2memoid_del, NULL);
    addr2MemOID_write = splay_tree_new(*addr2memoid_cmp, *addr2memoid_del, NULL);
}
