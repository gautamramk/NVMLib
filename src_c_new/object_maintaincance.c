#include "object_maintainance.h"
#include "types.h"
#include "malloc.h"
#include <libpmemobj.h>
#include "globals.h"
#include "pool.h"

uv_loop_t *deletion_loop;
uv_loop_t *moving_loop;

void initialise_logistics() {
    // Loop for deletion
    deletion_loop = uv_loop_new();
    uv_work_t *work_del = (uv_work_t *)malloc(size(uv_work_t));
    work_del->loop = deletion_loop;

    // add other shit... decide as you go
    uv_queue_work(deletion_loop, work_del, delete_objects, NULL); // add "after_work_cb" is required later

    // Start the logistics thread
    // Start in `UV_RUN_DEFAULT` mode
    uv_run(deletion_loop);

    // Loop for moving objects
    moving_loop = uv_loop_new();
    uv_work_t *work_move = (uv_work_t *)malloc(size(uv_work_t));
    work_move->loop = moving_loop;

    // add other shit... decide as you go
    uv_queue_work(moving_loop, work_move, move_objects, NULL); // add "after_work_cb" is required later

    // Start the logistics thread
    // Start in `UV_RUN_DEFAULT` mode
    uv_run(moving_loop);
}

uv_loop_t* get_logistics_loop() {
    return uv_is_active(deletion_loop) ? deletion_loop : NULL;
}

// never return unless faults
void delete_objects() {
    TOID(struct hashmap_tx) *hashmap = get_types_map();

    TOID(struct buckets) buckets = D_RO(hashmap)->buckets;
	TOID(struct entry) var;

    while(1){
        for (size_t i = 0; i < D_RO(buckets)->nbuckets; ++i) {
            if (TOID_IS_NULL(D_RO(buckets)->bucket[i]))
                continue;

            for (var = D_RO(buckets)->bucket[i]; !TOID_IS_NULL(var);
                    var = D_RO(var)->next) {
                ret = check_if_required_to_delete(D_RO(var)->key, D_RO(var)->value);
                if(ret) {
                    // Needs to be deleted
                    memfree(D_RO(var)->value);
                }
            }
        }

        // resume operation every 5 min
        sleep(5);
    }
}

// never return unless faults
void move_objects() {
    TOID(struct hashmap_tx) *hashmap = get_types_map();

    TOID(struct buckets) buckets = D_RO(hashmap)->buckets;
	TOID(struct entry) var;

    while(1){
        for (size_t i = 0; i < D_RO(buckets)->nbuckets; ++i) {
            if (TOID_IS_NULL(D_RO(buckets)->bucket[i]))
                continue;

            for (var = D_RO(buckets)->bucket[i]; !TOID_IS_NULL(var);
                    var = D_RO(var)->next) {
                ret = check_if_required_to_move(D_RO(var)->key, D_RO(var)->value);
                switch (ret){
                    case 1:
                        move_to_nvram(D_RO(var)->key, D_RO(var)->value, sizeof(TOID_TYPEOF(var)));
                        break;
                    case 2:
                        move_to_dram(D_RO(var)->key, D_RO(var)->value, sizeof(TOID_TYPEOF(var)));
                        break;
                    default:
                        break;
                }
            }
        }
        
        // resume operation every 5 min
        sleep(5);
    }
}


// Checks if an object can be deleted or not
// @return : 1 - If can be deleted
//           0 - otherwise
int check_if_required_to_delete(MEMoidKey key, MEMoid oid) {
    // To be completed
    return 0
}

// Checks if an object can be moved or not
// @return : 1 - If needs to be moved from DRAM to NVRAM
//           2 - If needs to be moved from NVRAM to DRAM             
//           0 - otherwise
int check_if_required_to_move(MEMoidKey key, MEMoid oid) {
    // To be completed
    return 0
}

void move_to_dram(MEMoidKey key, MEMoid oid, size_t size) {
    MEMoid new_obj;
    new_obj.offset = (uint64_t)(malloc(size));
    new_obj.pool_id = POOL_ID_MALLOC_OBJ;
    
    // copying the object contents
    memcpy(new_obj.offset, get_pool_from_poolid(oid.pool_id) + oid.offset, size);

    // freeing the contents in nvram
    nvm_free(oid.pool_id, oid.offset, size);

    // updating the `types_table`
    // NOTE: we have to mannually delete before inserting for the same key
    remove_object_from_hashmap(key);
    insert_object_to_hashmap(key, new_obj);

    // need to update the levels list
}

void move_to_nvram(MEMoidKey key, MEMoid oid, size_t size) {
    MEMoid new_obj;
    new_obj.pool_id = get_current_poolid();
    new_obj.offset = get_first_free_offset(size);

    memcpy(get_pool_from_poolid(new_obj.pool_id) + new_obj.offset, oid.offset, size);
    //TODO: need to call peme_persist ... later!!

    // updating the `types_table`
    // NOTE: we have to mannually delete before inserting for the same key
    remove_object_from_hashmap(key);
    insert_object_to_hashmap(key, new_obj);

    // need to update the levels list
}
