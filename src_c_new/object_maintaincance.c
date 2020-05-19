#include "object_maintainance.h"
#include "types.h"
#include "malloc.h"
#include <libpmemobj.h>
#include "globals.h"
#include "pool.h"
#include "hashmap.h"

uv_loop_t *deletion_loop;
uv_loop_t *moving_loop;


void initialise_logistics() {

    // initialising hashmap
    create_hashmap();

    // Loop for deletion
    deletion_loop = uv_loop_new();
    uv_work_t *work_del = (uv_work_t *)malloc(sizeof(uv_work_t));
    work_del->loop = deletion_loop;

    // add other shit... decide as you go
    uv_queue_work(deletion_loop, work_del, delete_objects, NULL); // add "after_work_cb" is required later

    // Start the logistics thread
    // Start in `UV_RUN_DEFAULT` mode
    uv_run(deletion_loop);

    // Loop for moving objects
    moving_loop = uv_loop_new();
    uv_work_t *work_move = (uv_work_t *)malloc(sizeof(uv_work_t));
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
        sleep(DELETE_LOOP_SLEEP_TIME);
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
        sleep(MOVE_LOOP_SLEEP_TIME);
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


/**
 * Hashmap for object maintaince
 * 
 * will be used by `check_if_required_to_move`
 * 
**/

int compare(object_maintainance *a, object_maintainance *b) {
    if(a->key == a->key) {
        return 0;
    }
    return 1;
}

uint64_t get_hash(object_maintainance *entry) {
	return entry->oid.pool_id + entry->oid.offset;
}


DECLARE_HASHMAP(object_maintainance)
DEFINE_HASHMAP(object_maintainance, compare, get_hash, free, realloc)

HASH_MAP(object_maintainance) *object_maintainance_map;

void create_hashmap() {
    object_maintainance_map = HASH_MAP_CREATE(object_maintainance)();
}

object_maintainance* create_new_maintainance_map_entry(MEMoidKey key, MEMoid oid, where_t which_ram) {
    object_maintainance* obj = (object_maintainance *)malloc(sizeof(object_maintainance) * 1);
    obj->key = key;
    obj->oid = oid;
    obj->num_reads = 0;
    obj->num_writes = 0;
    obj->last_accessed_at = time(NULL);
    obj->time_since_previous_access = 0;
    obj->previous_access_type = UNKNOWN;
    obj->access_bitmap = (uint64_t*)malloc((ceil((double)oid.size/64)));
    obj->which_ram = which_ram;
    obj->shift_level = JUST_ENTERED;

    return obj;
}

void insert_into_maintainance_map(object_maintainance *obj) {
    HASH_MAP_INSERT(object_maintainance)(object_maintainance_map, &obj, HMDR_REPLACE); 
}

void delete_from_maintainance_map(object_maintainance *obj) {
    HASH_MAP_ERASE(object_maintainance)(object_maintainance_map, obj);
}

object_maintainance* find_in_maintainance_map(MEMoidKey key, MEMoid oid) {
    // A placeholder for the actual object in the map
    object_maintainance *found_obj = create_new_maintainance_map_entry(key, oid, UNKNOWN);

    bool is_found = HASH_MAP_FIND(object_maintainance)(object_maintainance_map, &found_obj);

    if(is_found) {
        return found_obj;
    }
    return NULL;
}   