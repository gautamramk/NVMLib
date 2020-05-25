#include "globals.h"
#include "pool.h"
#include "metadata.h"
#include <libpmemobj.h>
#include <libpmem.h>
#include "hashmap.h"
#include "types.h"
#include "free_slot_list.h"
#include "log.h"

//extern const char *program_invocation_short_name;

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

DEFINE_HASHMAP(pool_kv, compare_kv, get_hash_kv, free, realloc)

HASH_MAP(pool_kv) *pool_map;

DEFINE_HASHMAP(pool_free_slot_val, compare_free_slot, get_hash_free_slot, free, realloc)

HASH_MAP(pool_free_slot_val) *pool_free_slot_map;

int initialize_pool() {
    pool_map = HASH_MAP_CREATE(pool_kv)();
    pool_free_slot_map = HASH_MAP_CREATE(pool_free_slot_val)();
    num_pools = retrieve_num_pools();
    printf("initialize pool num_pools= %d\n", num_pools);
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
        free_slots->head = &(D_RO(POBJ_ROOT(free_slot_pool, struct pool_free_slots_root))->head);

        int ret = HASH_MAP_INSERT(pool_free_slot_val)(pool_free_slot_map, &free_slots, HMDR_FIND);
        printf("pool free slot insert return %d\n", ret);
        init_pool_kv->key = idx;
        init_pool_kv->pool_ptr = pool_ptr;
        HASH_MAP_INSERT(pool_kv)(pool_map, &init_pool_kv, HMDR_FIND);
    }
    return 0;
}

uintptr_t get_pool_from_poolid(uint64_t pool_id) {
    pool_kv temp;
    pool_kv* temp_ptr = &temp;
    temp.key = pool_id;
    temp.pool_ptr = (uintptr_t)NULL;
    if (HASH_MAP_FIND(pool_kv)(pool_map, &temp_ptr)) {
        return (uintptr_t)temp_ptr->pool_ptr;
    }
    return NULL;
}

uint64_t get_current_poolid() {
    return num_pools;
}

void create_new_pool(size_t size) {
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
    sprintf(pool_number_str, "%3d", num_pools);
    strcat(pool_file_name, pool_number_str);
    strcat(pool_free_slot_file_name, pool_number_str);
    // Need to corrected with proper pathss
    size_t allot_size = (size_t)ceil((double)size/POOL_SIZE)*POOL_SIZE;
    uintptr_t pmemaddr = (uintptr_t)pmem_map_file(pool_file_name, allot_size, PMEM_FILE_CREATE,
			                                      0666, NULL, NULL);
    PMEMobjpool* free_slot_pool = pmemobj_create(pool_free_slot_file_name, POBJ_LAYOUT_NAME(free_slot_layout),
			                                     PMEMOBJ_MIN_POOL, 0666);
    TOID(struct pool_free_slots_root) pool_root = POBJ_ROOT(free_slot_pool, struct pool_free_slots_root);
    TX_BEGIN (free_slot_pool) {
        TOID(struct pool_free_slot) new_slot = TX_NEW(struct pool_free_slot);
        POBJ_TAILQ_INSERT_HEAD(&(D_RW(pool_root)->head), new_slot, fnd);
    } TX_END

    new_entry->pool_ptr = pmemaddr;
    new_free_slots->pool = free_slot_pool;
    new_free_slots->head = &(D_RO(pool_root)->head);
    printf("new_entry %p new_free_slots %p\n", new_entry, new_free_slots);
    printf("new_entry->pool_ptr %ld pmemaddr %ld\n", new_entry->pool_ptr, pmemaddr);
    printf("new_free_slots->pool %ld free_slot_pool %ld\n", new_free_slots->pool, free_slot_pool);
    update_num_pools(num_pools);
    HASH_MAP_INSERT(pool_kv)(pool_map, &new_entry, HMDR_FIND);
    HASH_MAP_INSERT(pool_free_slot_val)(pool_free_slot_map, &new_free_slots, HMDR_FIND);
}

void nvm_free(uint64_t pool_id, uint64_t offset, size_t size) {
    LOG_INFO("NVM freeing value pool_id = %ld, offset = %ld size = %ld\n", pool_id, offset, size);
    pool_free_slot_val temp;
    pool_free_slot_val* temp_ptr = &temp;
    temp.key = pool_id;
    HASH_MAP_FIND(pool_free_slot_val)(pool_free_slot_map, &temp_ptr);
    pool_free_slot_head* f_head = temp_ptr->head;
    TOID(struct pool_free_slot) node;
    TOID(struct pool_free_slot) next_node;
    TOID(struct pool_free_slot) to_remove_node;
    TOID(struct pool_free_slot) to_insert_node;

    /**
     * Flag : 1 -- to remove node
     *        2 -- to insert node
    **/
    int flag = 0;

    node = POBJ_TAILQ_FIRST(f_head);
    if (offset < D_RO(node)->start_b) {
        // its in the very first location
        if (offset + size - 1 == D_RO(node)->start_b - 1) {
            TX_BEGIN(temp_ptr->pool) {
                D_RW(node)->start_b = offset;
                flag = -2;
            } TX_END
        } else {
            TX_BEGIN(temp_ptr->pool) {
                to_insert_node = TX_NEW(struct pool_free_slot);
                // POBJ_TAILQ_INSERT_AFTER(f_head, node, to_insert_node, fnd);

                D_RW(to_insert_node)->start_b = offset;
                D_RW(to_insert_node)->end_b = offset + size - 1;

                flag = -3;                
            } TX_END
        }
        goto end;
    }   

    POBJ_TAILQ_FOREACH (node, f_head, fnd) {
        if (offset == D_RO(node)->end_b + 1) {
            TX_BEGIN(temp_ptr->pool) {
                D_RW(node)->end_b = D_RO(node)->end_b + size;
                next_node = POBJ_TAILQ_NEXT(node, fnd);
                if (!TOID_IS_NULL(next_node)) {
                    if (offset + size - 1 == D_RO(next_node)->start_b - 1) {
                        D_RW(node)->end_b = D_RO(next_node)->end_b;
                        // POBJ_TAILQ_REMOVE_FREE(f_head, next_node, fnd);
                        
                        // can't modify the datastructure within a loop
                        to_remove_node = next_node;
                        flag = 1;
                    }
                }
            } TX_END
            break;
        } else if (offset > D_RO(node)->end_b + 1) {
            next_node = POBJ_TAILQ_NEXT(node, fnd);
            if (!TOID_IS_NULL(next_node)) {
                if (offset + size - 1 == D_RO(next_node)->start_b - 1) {
                    TX_BEGIN(temp_ptr->pool) {
                        D_RW(next_node)->start_b = offset;
                        flag = 2;
                    } TX_END
                    break;
                } else {
                    TX_BEGIN(temp_ptr->pool) {
                        to_insert_node = TX_NEW(struct pool_free_slot);
                        // POBJ_TAILQ_INSERT_AFTER(f_head, node, to_insert_node, fnd);

                        D_RW(to_insert_node)->start_b = offset;
                        D_RW(to_insert_node)->end_b = offset + size - 1;

                        flag = 3;                  
                    } TX_END

                    break;
                }
            } 
        }
    }

end:
    printf("nvm_free: f_head %p\n", f_head);
    printf("nvm free: flag %d\n", flag);
    printf("nvm_free: node->end_b %p\n", D_RO(node)->end_b);
    

    if (flag == 1) {
        printf("nvm_free: to_remove->end_b %p\n", D_RO(to_remove_node)->end_b);
        TX_BEGIN(temp_ptr->pool) {
            POBJ_TAILQ_REMOVE_FREE(f_head, to_remove_node, fnd);
        } TX_END
        return;
    }

    if(flag == 3) {
        printf("nvm_free: to_insert_node->end_b %p, to_insert_node->start_b %p\n", D_RO(to_insert_node)->end_b, D_RO(to_insert_node)->start_b);
        TX_BEGIN(temp_ptr->pool) {
            POBJ_TAILQ_INSERT_AFTER(f_head, node, to_insert_node, fnd);
        } TX_END
        return;
    }

    if (flag == -3) {
        printf("nvm_free: to_insert_node->end_b %p, to_insert_node->start_b %p\n", D_RO(to_insert_node)->end_b, D_RO(to_insert_node)->start_b);
        TX_BEGIN(temp_ptr->pool) {
            POBJ_TAILQ_INSERT_HEAD(f_head, to_insert_node, fnd);
        } TX_END
        return;
    }
}