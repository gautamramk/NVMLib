#include "globals.h"
#include "pool.h"
#include <libpmemobj.h>
#include <libpmem.h>

PMEMobjpool *free_slot_pop;
struct pool_free_slot_head *f_head;
uint32_t num_pools;

typedef struct pool_kv_st {
    uint16_t key;
    uintptr_t pool_ptr;
} pool_kv;

DECLARE_HASHMAP(pool_kv)
DEFINE_HASHMAP(pool_kv, compare, get_hash, free, realloc)

static inline compare(pool_kv* left, pool_kv* right) {
    if (left->pool_ptr == right->pool_ptr) return 0;
    return 1;
}

int get_hash(pool_kv *val){
    return val->key;
}

HASH_MAP(pool_kv) *pool_map;

int initialize_pool() {
    num_pools = 0;
    pool_map = HASH_MAP_CREATE(pool_kv)();

    /*
    free_slot_pop = pmemobj_open("free_slots", POBJ_LAYOUT_NAME(list));
    if (free_slot_pop == NULL) {
		perror("pmemobj_create");
		return 1;
	}
    TOID(struct pool_free_slots_root) root = POBJ_ROOT(free_slot_pop, struct pool_free_slots_root);
    f_head = &D_RW(root)->head;
    */
}

static inline uint64_t free_slot_size(TOID(struct pool_free_slot) slot) {
    return D_RO(slot)->end_b - D_RO(slot)->start_b + 1;
}

uintptr_t get_pool_from_poolid(uint64_t pool_id) {
    uintptr_t temp;
    if (HASH_MAP_FIND(uintptr_t)(pool_map, &temp)) {
        return temp;
    }
    return NULL;
}

uint64_t get_current_poolid() {
    return num_pools;
}

void create_new_pool() {
    num_pools++;
    pool_kv* new_entry = (pool_kv*)malloc(sizeof(pool_kv));
    new_entry->key = num_pools;
    uintptr_t pmemaddr = (uintptr_t)pmem_map_file(PATH, PMEM_LEN, PMEM_FILE_CREATE,
			                                      0666, &mapped_len, &is_pmem);
    new_entry->pool_ptr = pmemaddr;
    
}

uint64_t allot_first_free_offset(size_t size) {
    // LOG shit
    uint64_t ret = -1;
    TOID(struct pool_free_slot) node;
    POBJ_TAILQ_FOREACH (node, f_head, fnd) {
        if (free_slot_size(node) == size) {
            ret = D_RO(node)->start_b
            TX_BEGIN(free_slot_pop) {
                POBJ_TAILQ_REMOVE_FREE(f_head, node, fnd);
            } TX_END
            break;
        } else if (free_slot_size(node) > size) {
            ret = D_RO(node)->start_b
            uint64_t new_start = D_RO(node)->start_b + size;
            D_RW(node)->start_b = new_start;
            break;
        }
    }
    return ret;
}

void nvm_free(uint64_t pool_id, uint64_t offset, size_t size) {
    TOID(struct pool_free_slot) node;
    POBJ_TAILQ_FOREACH (node, f_head, fnd) {
        if (offset == D_RO(node)->end_b + 1) {
            D_RW(node)->end_b = D_RO(node)->end_b + size;
        } else if (offset > D_RO(node)->end_b + 1) {
            if (!TOID_IS_NULL(POBJ_TAILQ_NEXT(node, fnd))) {
                if (offset + size - 1 == D_RO(POBJ_TAILQ_NEXT(node, fnd))->start_b - 1) {
                    D_RW(POBJ_TAILQ_NEXT(node, fnd))->start_b = offset;
                }
            } else {
                TOID(struct pool_free_slot) new_node = TX_NEW(struct pool_free_slot);
                POBJ_TAILQ_INSERT_AFTER(node, new_node, fnd);
                D_RW(new_node)->start_b = offset;
                D_RW(new_node)->end_b = offset + size - 1;
            }
        }
    }
}