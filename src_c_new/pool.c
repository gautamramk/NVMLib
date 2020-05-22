#include "globals.h"
#include "pool.h"
#include "metadata.h"
#include <libpmemobj.h>
#include <libpmem.h>
#include "hashmap.h"
#include "types.h"
#include "free_slot_list.h"

//extern const char *program_invocation_short_name;
uint32_t num_pools;

typedef struct pool_kv_st {
    uint16_t key;
    uintptr_t pool_ptr;
} pool_kv;

typedef struct pool_free_slot_val_st {
    uint16_t key;
    PMEMobjpool* pool;
    pool_free_slot_head head;
} pool_free_slot_val;

static int compare_kv(pool_kv* left, pool_kv* right) {
    if (left->key == right->key) return 0;
    return 1;
}

static int compare_free_slot(pool_free_slot_val* left, pool_free_slot_val* right) {
    if (left->key == right->key) return 0;
    return 1;
}

int get_hash_kv(pool_kv *val){
    return val->key;
}

int get_hash_free_slot(pool_free_slot_val *val){
    return val->key;
}

DECLARE_HASHMAP(pool_kv)
DEFINE_HASHMAP(pool_kv, compare_kv, get_hash_kv, free, realloc)

HASH_MAP(pool_kv) *pool_map;

DECLARE_HASHMAP(pool_free_slot_val)
DEFINE_HASHMAP(pool_free_slot_val, compare_free_slot, get_hash_free_slot, free, realloc)

HASH_MAP(pool_free_slot_val) *pool_free_slot_map;

int initialize_pool() {
    pool_map = HASH_MAP_CREATE(pool_kv)();
    num_pools = retrieve_num_pools();
    for (int idx = 1; idx <= num_pools; idx++) {
        char pool_file_name[50];
        char pool_free_slot_file_name[50];
        char pool_number_str[4];
        strcpy(pool_file_name, program_invocation_short_name);
        strcpy(pool_free_slot_file_name, program_invocation_short_name);
        strcat(pool_file_name, "_poolfile_");
        strcat(pool_free_slot_file_name, "_free_slot_");
        sprintf(pool_number_str, "%3d", idx);
        strcat(pool_file_name, pool_number_str);
        strcat(pool_free_slot_file_name, pool_number_str);
        uintptr_t pool_ptr = (uintptr_t)pmem_map_file(pool_file_name, 0, 0,
			                                          0666, NULL, NULL);
        pool_kv* init_pool_kv = malloc(sizeof(pool_kv));
        pool_free_slot_val* free_slots = malloc(sizeof(pool_free_slot_val));

        PMEMobjpool* free_slot_pool = pmemobj_open(pool_free_slot_file_name, POBJ_LAYOUT_NAME(free_slot_layout));

        free_slots->key = idx;
        free_slots->pool = free_slot_pool;
        free_slots->head = D_RO(POBJ_ROOT(free_slot_pool, struct pool_free_slots_root))->head;

        HASH_MAP_INSERT(pool_free_slot_val)(pool_free_slot_map, &free_slots, HMDR_FIND);

        init_pool_kv->key = idx;
        init_pool_kv->pool_ptr = pool_ptr;
        HASH_MAP_INSERT(pool_kv)(pool_map, &init_pool_kv, HMDR_FIND);
    }
    return 0;
}

static inline uint64_t free_slot_size(TOID(struct pool_free_slot) slot) {
    return D_RO(slot)->end_b - D_RO(slot)->start_b + 1;
}

uintptr_t get_pool_from_poolid(uint64_t pool_id) {
    pool_kv* temp;
    if (HASH_MAP_FIND(pool_kv)(pool_map, &temp)) {
        return (uintptr_t)temp;
    }
    return NULL;
}

uint64_t get_current_poolid() {
    return num_pools;
}

void create_new_pool() {
    num_pools++;
    pool_kv* new_entry = (pool_kv*)malloc(sizeof(pool_kv));
    pool_free_slot_val* new_free_slots = malloc(sizeof(pool_free_slot_val));

    new_entry->key = num_pools;
    new_free_slots->key = num_pools;

    char pool_file_name[50];
    char pool_free_slot_file_name[50];
    char pool_number_str[4];
    strcpy(pool_file_name, program_invocation_short_name);
    strcpy(pool_free_slot_file_name, program_invocation_short_name);
    strcat(pool_file_name, "_poolfile_");
    strcat(pool_free_slot_file_name, "_free_slot_");
    sprintf(pool_number_str, "%3d", num_pools-1);
    strcat(pool_file_name, pool_number_str);
    strcat(pool_free_slot_file_name, pool_number_str);
    // Need to corrected with proper pathss
    uintptr_t pmemaddr = (uintptr_t)pmem_map_file(pool_file_name, POOL_SIZE, PMEM_FILE_CREATE,
			                                      0666, NULL, NULL);
    PMEMobjpool* free_slot_pool = pmemobj_create(pool_free_slot_file_name, POBJ_LAYOUT_NAME(free_slot_layout),
			                                     PMEMOBJ_MIN_POOL, 0666);
    new_entry->pool_ptr = pmemaddr;
    new_free_slots->pool = free_slot_pool;

    update_num_pools(num_pools);
    HASH_MAP_INSERT(pool_kv)(pool_map, &new_entry, HMDR_FIND);
    HASH_MAP_INSERT(pool_free_slot_val)(pool_free_slot_map, &new_free_slots, HMDR_FIND);
}

uint64_t allot_first_free_offset(uint64_t pool_id, size_t size) {
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

void nvm_free(uint64_t pool_id, uint64_t offset, size_t size) {
    pool_free_slot_val temp;
    pool_free_slot_val* temp_ptr = &temp;
    temp.key = pool_id;
    HASH_MAP_FIND(pool_free_slot_val)(pool_free_slot_map, &temp_ptr);
    pool_free_slot_head *f_head = &(temp_ptr->head);
    TOID(struct pool_free_slot) node;
    TOID(struct pool_free_slot) next_node;
    POBJ_TAILQ_FOREACH (node, f_head, fnd) {
        if (offset == D_RO(node)->end_b + 1) {
            TX_BEGIN(temp_ptr->pool) {
                D_RW(node)->end_b = D_RO(node)->end_b + size;
                next_node = POBJ_TAILQ_NEXT(node, fnd);
                if (!TOID_IS_NULL(next_node)) {
                    if (offset + size - 1 == D_RO(next_node)->start_b - 1) {
                        D_RW(node)->end_b = D_RO(next_node)->end_b;
                        POBJ_TAILQ_REMOVE_FREE(f_head, next_node, fnd);
                    }
                }
            } TX_END
        } else if (offset > D_RO(node)->end_b + 1) {
            next_node = POBJ_TAILQ_NEXT(node, fnd);
            if (!TOID_IS_NULL(next_node)) {
                if (offset + size - 1 == D_RO(POBJ_TAILQ_NEXT(node, fnd))->start_b - 1) {
                    TX_BEGIN(temp_ptr->pool) {
                        D_RW(next_node)->start_b = offset;
                    } TX_END
                }
            } else {
                TX_BEGIN(temp_ptr->pool) {
                    TOID(struct pool_free_slot) new_node = TX_NEW(struct pool_free_slot);
                    POBJ_TAILQ_INSERT_AFTER(f_head, node, new_node, fnd);
                    D_RW(new_node)->start_b = offset;
                    D_RW(new_node)->end_b = offset + size - 1;
                } TX_END
            }
        }
    }
}