#include "object_maintainance.h"
#include "types.h"
#include "malloc.h"
#include <libpmemobj.h>
#include "globals.h"
#include "pool.h"
#include "hashmap.h"
#include "mem_log.h"
#include "log.h"
#include <uv.h>
#include <pthread.h>

uv_mutex_t object_maintainence_hashmap_mutex;   // used during manupulation of `types map`
uv_mutex_t object_maintainence_memory_mutex;    // used during `nvm_free` / access too
uv_mutex_t object_maintainence_maintain_map_mutex;   // used during manupulation of `maintainance map`

/**
 * Hashmap for object maintaince
 * 
 * will be used by `check_if_required_to_move`
 * 
**/

uint64_t get_hash(object_maintainance *entry) {
	return entry->key;
}

int compare(object_maintainance *a, object_maintainance *b) {
    if(a->key == b->key) {
        return 0;
    }
    return 1;
}

DECLARE_HASHMAP(object_maintainance)
DEFINE_HASHMAP(object_maintainance, compare, get_hash, free, realloc)

HASH_MAP(object_maintainance) *object_maintainance_map;

void initialise_logistics() {

    // Initialising hashmap for logistics
    create_maintainance_map();

    // Initialising the mutexes
    uv_mutex_init(&object_maintainence_hashmap_mutex);
    uv_mutex_init(&object_maintainence_memory_mutex);

    // The logistics and deletion threads
    pthread_t logistics_thread, deletion_thread;

    uv_loop_t *logistics_loop = uv_loop_new();
    pthread_create(&logistics_thread, NULL, logistics_thread_function, logistics_loop);

    uv_loop_t *deletion_loop = uv_loop_new();
    pthread_create(&deletion_thread, NULL, deletion_thread_function, deletion_loop);
}

void *logistics_thread_function(void *data){
    uv_loop_t *thread_loop = (uv_loop_t *) data;
    LOG_INFO("Logistics thread will start event loop\n");

    uv_timer_t timer_logistics;
    uv_timer_init(thread_loop, &timer_logistics);
    uv_timer_start(&timer_logistics, (uv_timer_cb)on_logistics_timer, 0, MOVE_LOOP_SLEEP_TIME);

    //Start this loop
    uv_run(thread_loop, UV_RUN_DEFAULT);
    pthread_exit(NULL);
}

void *deletion_thread_function(void *data){
    uv_loop_t *thread_loop = (uv_loop_t *) data;
    LOG_INFO("Deletion thread will start event loop\n");

    uv_timer_t timer_logistics;
    uv_timer_init(thread_loop, &timer_logistics);
    uv_timer_start(&timer_logistics, (uv_timer_cb)on_deletion_timer, 0, MOVE_LOOP_SLEEP_TIME);

    //Start this loop
    uv_run(thread_loop, UV_RUN_DEFAULT);
    pthread_exit(NULL);
}

static inline void set_bits(uint64_t* bitmap, size_t offset, size_t size) {
    int byteno = offset/64;
    int shift = offset%64;
    uint64_t bit = 1<<shift;
    for (int i = 0; i < size; i++) {
        if (shift >= 64) {shift = 0; byteno++; bit = 1;}
        bitmap[byteno] |= bit;
        shift++;
        bit <<= 1;
    }
}

void on_logistics_timer(uv_timer_t *timer, int status) {
    uv_work_t* work_req;
    object_maintainance var;

    if(!object_maintainance_map->entries){
        // nothing yet in the logistics map
        return;
    }
    while(!TAILQ_EMPTY(&write_queue_head)) {
        address_log* w_add = TAILQ_FIRST(&write_queue_head);
        addr2memoid_key skey;
        skey.comp = cmp_addr;
        skey.addr = w_add->addr;
        splay_tree_node ret_node = splay_tree_lookup(addr2MemOID_write, (splay_tree_key)&skey);
        MEMoidKey mkey = ((addr2memoid_key*)ret_node->key)->key;
        object_maintainance* om = find_in_maintainance_map(mkey);
        om->num_writes++;
        om->last_accessed_at = w_add->access_time;
        set_bits(om->write_bitmap, w_add->addr - KEY_FIRST(mkey), w_add->size);
    }
    while(!TAILQ_EMPTY(&read_queue_head)) {
        address_log* r_add = TAILQ_FIRST(&read_queue_head);
        addr2memoid_key skey;
        skey.comp = cmp_addr;
        skey.addr = r_add->addr;
        splay_tree_node ret_node = splay_tree_lookup(addr2MemOID_read, (splay_tree_key)&skey);
        MEMoidKey mkey = ((addr2memoid_key*)ret_node->key)->key;
        object_maintainance* om = find_in_maintainance_map(mkey);
        om->num_reads++;
        om->last_accessed_at = r_add->access_time;
        set_bits(om->read_bitmap, r_add->addr - KEY_FIRST(mkey), r_add->size);
    }
    for(size_t i=0; i<object_maintainance_map->power_of_two; i++) {
        HASH_MAP_BUCKET(object_maintainance) *bucket = &object_maintainance_map->entries[i];
        for(int j = 0; j < bucket->size; j++) {
            // by value !!!
            var = bucket->entries[j];
            

            int ret = check_if_required_to_move(var.key, var.oid);
            switch (ret){
                case 1:
                    work_req = (uv_work_t*)malloc(sizeof(*work_req));
                    work_req->data = malloc(sizeof(object_maintainance));
                    *((object_maintainance *)(work_req->data)) = var;

                    uv_queue_work(timer->loop, work_req, move_to_nvram, on_after_work);
                    break;
                case 2:
                    work_req = (uv_work_t*)malloc(sizeof(*work_req));
                    work_req->data = malloc(sizeof(object_maintainance));
                    *((object_maintainance *)(work_req->data)) = var;

                    uv_queue_work(timer->loop, work_req, move_to_dram, on_after_work);
                    break;
                default:
                    break;
            }
        }
    }
}


void on_deletion_timer(uv_timer_t *timer, int status) {
    uv_work_t* work_req;
    object_maintainance var;

    if(!object_maintainance_map->entries){
        // nothing yet in the logistics map
        return;
    }
    
    for(size_t i=0; i<object_maintainance_map->power_of_two; i++) {
        HASH_MAP_BUCKET(object_maintainance) *bucket = &object_maintainance_map->entries[i];
        for(int j = 0; j < bucket->size; j++) {
            // by value !!!
            var = bucket->entries[j];

            int ret = check_if_required_to_delete(var);
            if(ret) {
                // Needs to be deleted
                work_req = (uv_work_t*)malloc(sizeof(*work_req));
                work_req->data = malloc(sizeof(object_maintainance));
                *((object_maintainance *)(work_req->data)) = var;

                uv_queue_work(timer->loop, work_req, delete_object, on_after_work);
            }
        }
    }
}

void delete_object(uv_work_t *req){
    MEMoidKey key = ((object_maintainance*)(req->data))->key;
    MEMoid oid = ((object_maintainance*)(req->data))->oid;
    size_t size = oid.size;
    bool is_dram = false;

    // Get the hashmap mutex first to ensure there are no leftover accesses
    uv_mutex_lock(&object_maintainence_hashmap_mutex);
    
    switch (oid.pool_id) {
        case POOL_ID_MALLOC_OBJ: 
            // dram object

            // freeing the contents in dram
            uv_mutex_lock(&object_maintainence_memory_mutex);
            free((void*)oid.offset);
            uv_mutex_unlock(&object_maintainence_memory_mutex);
            is_dram = true;
            break;

        default:
            // nvram object

            // freeing the contents in nvram
            uv_mutex_lock(&object_maintainence_memory_mutex);
            nvm_free(oid.pool_id, oid.offset, size);
            uv_mutex_unlock(&object_maintainence_memory_mutex);
            break;
    }

    // deleting the object from `types map`
    remove_object_from_hashmap(key);

    uv_mutex_unlock(&object_maintainence_hashmap_mutex);

    // Updating the `object_maintainance_map`
    uv_mutex_lock(&object_maintainence_maintain_map_mutex);
    delete_from_maintainance_map((object_maintainance*)(req->data));
    uv_mutex_unlock(&object_maintainence_maintain_map_mutex);

    LOG_INFO("delete_object: %ld from %s at [%ld ms]\n", key, is_dram?"DRAM":"NVRAM", (uv_hrtime() / 1000000) % 100000);
}


void move_to_dram(uv_work_t *req) {

    MEMoidKey key = ((object_maintainance*)(req->data))->key;
    MEMoid oid = ((object_maintainance*)(req->data))->oid;
    size_t size = oid.size;

    MEMoid new_obj;
    new_obj.offset = (uint64_t)(malloc(size));
    new_obj.pool_id = POOL_ID_MALLOC_OBJ;
    new_obj.size = size;
    
    // copying the object contents
    memcpy((void*)new_obj.offset, (void*)(get_pool_from_poolid(oid.pool_id) + oid.offset), size);

    // Get the hashmap mutex first to ensure there are no leftover accesses
    uv_mutex_lock(&object_maintainence_hashmap_mutex);

    // freeing the contents in nvram
    uv_mutex_lock(&object_maintainence_memory_mutex);
    nvm_free(oid.pool_id, oid.offset, size);
    uv_mutex_unlock(&object_maintainence_memory_mutex);

    // updating the `types_table`
    // NOTE: we have to mannually delete before inserting for the same key
    remove_object_from_hashmap(key);
    insert_object_to_hashmap(key, new_obj);

    uv_mutex_unlock(&object_maintainence_hashmap_mutex);

    // Updating the `object_maintainance_map`
    uv_mutex_lock(&object_maintainence_maintain_map_mutex);
    
    object_maintainance *new_maintainance_map_obj = (object_maintainance *)malloc(sizeof(object_maintainance));
    // by value !!!
    *new_maintainance_map_obj = *((object_maintainance*)(req->data));
    // changed fields
    new_maintainance_map_obj->oid = new_obj;
    new_maintainance_map_obj->shift_level = JUST_ENTERED;
    new_maintainance_map_obj->which_ram = DRAM;

    // not this replaces the previous
    insert_into_maintainance_map(new_maintainance_map_obj);

    uv_mutex_unlock(&object_maintainence_maintain_map_mutex);

    LOG_INFO("move_to_dram: %ld at [%ld ms]\n", key, (uv_hrtime() / 1000000) % 100000);
}


void move_to_nvram(uv_work_t *req) {

    MEMoidKey key = ((object_maintainance*)(req->data))->key;
    MEMoid oid = ((object_maintainance*)(req->data))->oid;
    size_t size = oid.size;

    MEMoid new_obj;
    new_obj.pool_id = get_current_poolid();
    new_obj.offset = get_first_free_offset(size);
    new_obj.size = size;

    memcpy((void*)(get_pool_from_poolid(new_obj.pool_id) + new_obj.offset), (void*)oid.offset, size);
    //TODO: need to call peme_persist ... later!!

     // Get the hashmap mutex first to ensure there are no leftover accesses
    uv_mutex_lock(&object_maintainence_hashmap_mutex);

    // freeing the contents in dram
    uv_mutex_lock(&object_maintainence_memory_mutex);
    free((void*)oid.offset);
    uv_mutex_unlock(&object_maintainence_memory_mutex);

    // updating the `types_table`
    // NOTE: we have to mannually delete before inserting for the same key
    remove_object_from_hashmap(key);
    insert_object_to_hashmap(key, new_obj);

    uv_mutex_unlock(&object_maintainence_hashmap_mutex);


    // Updating the `object_maintainance_map`
    uv_mutex_lock(&object_maintainence_maintain_map_mutex);
    
    object_maintainance *new_maintainance_map_obj = (object_maintainance *)malloc(sizeof(object_maintainance));
    // by value !!!
    *new_maintainance_map_obj = *((object_maintainance*)(req->data));
    // changed fields
    new_maintainance_map_obj->oid = new_obj;
    new_maintainance_map_obj->shift_level = JUST_ENTERED;
    new_maintainance_map_obj->which_ram = DRAM;

    // not this replaces the previous
    insert_into_maintainance_map(new_maintainance_map_obj);

    uv_mutex_unlock(&object_maintainence_maintain_map_mutex);

    LOG_INFO("move_to_nvram: %ld at [%ld ms]\n", key, (uv_hrtime() / 1000000) % 100000);
}


void on_after_work(uv_work_t* req, int status) {
    free(req);
}




// Checks if an object can be deleted or not
// @return : 1 - If can be deleted
//           0 - otherwise
int check_if_required_to_delete(object_maintainance entry) {
    
    return entry.which_ram == NO_RAM ? 1 : 0;
}

// Checks if an object can be moved or not
// @return : 1 - If needs to be moved from DRAM to NVRAM
//           2 - If needs to be moved from NVRAM to DRAM             
//           0 - otherwise
int check_if_required_to_move(MEMoidKey key, MEMoid oid) {
    // To be completed
    return 0;
}


void create_maintainance_map() {
    object_maintainance_map = HASH_MAP_CREATE(object_maintainance)();
}

object_maintainance* create_new_maintainance_map_entry(MEMoidKey key, MEMoid oid, where_t which_ram, bool can_be_moved) {
    object_maintainance* obj = (object_maintainance *)malloc(sizeof(object_maintainance) * 1);
    obj->key = key;
    obj->oid = oid;
    obj->num_reads = 0;
    obj->num_writes = 0;
    obj->last_accessed_at = time(NULL);
    obj->time_since_previous_access = 0;
    obj->previous_access_type = ACCESS_UNKNOWN;
    obj->write_bitmap = (uint64_t*)malloc((ceil((double)oid.size/64)));
    obj->read_bitmap = (uint64_t*)malloc((ceil((double)oid.size/64)));
    obj->which_ram = which_ram;
    obj->shift_level = JUST_ENTERED;
    obj->can_be_moved = can_be_moved;

    return obj;
}

void insert_into_maintainance_map(object_maintainance *obj) {
    HASH_MAP_INSERT(object_maintainance)(object_maintainance_map, &obj, HMDR_REPLACE); 
}

void delete_from_maintainance_map(object_maintainance *obj) {
    HASH_MAP_ERASE(object_maintainance)(object_maintainance_map, obj);
}

object_maintainance* find_in_maintainance_map(MEMoidKey key) {
    // A placeholder for the actual object in the map
    object_maintainance *found_obj = create_new_maintainance_map_entry(key, get_MEMoid(key), RAM_UNKNOWN, false);

    bool is_found = HASH_MAP_FIND(object_maintainance)(object_maintainance_map, &found_obj);

    if(is_found) {
        return found_obj;
    }
    return NULL;

}



// ==============================================================================================================================
//                                       D E P R I C A T E D                C O D E
// ==============================================================================================================================

// uv_loop_t* get_logistics_loop() {
//     return uv_is_active(deletion_loop) ? deletion_loop : NULL;
// }


// // never return unless faults
// void delete_objects() {
//     TOID(struct hashmap_tx) *hashmap = get_types_map();

//     TOID(struct buckets) buckets = D_RO(hashmap)->buckets;
// 	TOID(struct entry) var;

//     while(1){
//         for (size_t i = 0; i < D_RO(buckets)->nbuckets; ++i) {
//             if (TOID_IS_NULL(D_RO(buckets)->bucket[i]))
//                 continue;

//             for (var = D_RO(buckets)->bucket[i]; !TOID_IS_NULL(var);
//                     var = D_RO(var)->next) {
//                 ret = check_if_required_to_delete(D_RO(var)->key, D_RO(var)->value);
//                 if(ret) {
//                     // Needs to be deleted
//                     memfree(D_RO(var)->value);
//                 }
//             }
//         }

//         // resume operation every 5 min
//         sleep(DELETE_LOOP_SLEEP_TIME);
//     }
// }

// // never return unless faults
// void move_objects() {
//     TOID(struct hashmap_tx) *hashmap = get_types_map();

//     TOID(struct buckets) buckets = D_RO(hashmap)->buckets;
// 	TOID(struct entry) var;

//     while(1){
//         for (size_t i = 0; i < D_RO(buckets)->nbuckets; ++i) {
//             if (TOID_IS_NULL(D_RO(buckets)->bucket[i]))
//                 continue;

//             for (var = D_RO(buckets)->bucket[i]; !TOID_IS_NULL(var);
//                     var = D_RO(var)->next) {
//                 ret = check_if_required_to_move(D_RO(var)->key, D_RO(var)->value);
//                 switch (ret){
//                     case 1:
//                         move_to_nvram(D_RO(var)->key, D_RO(var)->value, sizeof(TOID_TYPEOF(var)));
//                         break;
//                     case 2:
//                         move_to_dram(D_RO(var)->key, D_RO(var)->value, sizeof(TOID_TYPEOF(var)));
//                         break;
//                     default:
//                         break;
//                 }
//             }
//         }
        
//         // resume operation every 5 min
//         sleep(MOVE_LOOP_SLEEP_TIME);
//     }
// }


// void move_to_dram(MEMoidKey key, MEMoid oid, size_t size) {
//     MEMoid new_obj;
//     new_obj.offset = (uint64_t)(malloc(size));
//     new_obj.pool_id = POOL_ID_MALLOC_OBJ;
    
//     // copying the object contents
//     memcpy(new_obj.offset, get_pool_from_poolid(oid.pool_id) + oid.offset, size);

//     // freeing the contents in nvram
//     nvm_free(oid.pool_id, oid.offset, size);

//     // updating the `types_table`
//     // NOTE: we have to mannually delete before inserting for the same key
//     remove_object_from_hashmap(key);
//     insert_object_to_hashmap(key, new_obj);

//     // need to update the levels list
// }

// void move_to_nvram(MEMoidKey key, MEMoid oid, size_t size) {
//     MEMoid new_obj;
//     new_obj.pool_id = get_current_poolid();
//     new_obj.offset = get_first_free_offset(size);

//     memcpy(get_pool_from_poolid(new_obj.pool_id) + new_obj.offset, oid.offset, size);
//     //TODO: need to call peme_persist ... later!!

//     // updating the `types_table`
//     // NOTE: we have to mannually delete before inserting for the same key
//     remove_object_from_hashmap(key);
//     insert_object_to_hashmap(key, new_obj);

//     // need to update the levels list
// }
