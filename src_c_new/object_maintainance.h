#ifndef __NVM_OBJ_MAINTAIN__
#define __NVM_OBJ_MAINTAIN__

#include <uv.h>
#include "globals.h"
#include "malloc.h"
#include <time.h>

#define DELETE_LOOP_SLEEP_TIME 5    // 5 secs
#define MOVE_LOOP_SLEEP_TIME 5    // 5 secs

typedef enum access_type_en {
    UNKNOWN,
    READ,
    WRITE
} access_types_t;

typedef enum shitf_levels_en {
    JUST_ENTERED,
    ONE,
    TWO,
    THREE,
    FOUR,
    MOVE_NOW
} shift_levels_t;

typedef enum where_en {
    UNKNOWN,
    DRAM,
    NVRAM
} where_t;

// The structure used by the logistics threads
typedef struct object_maintainance_st {
    MEMoidKey key;
    MEMoid oid;

    uint32_t num_reads;
    uint32_t num_writes;
    time_t last_accessed_at;
    time_t time_since_previous_access;
    access_types_t previous_access_type;

    uint64_t *access_bitmap;

    where_t which_ram;

    shift_levels_t shift_level;
} object_maintainance;

void initialise_logistics();

uv_loop_t* get_logistics_loop();

// Returns the actaul object in the map as a pointer.
object_maintainance* find_in_maintainance_map(MEMoidKey key, MEMoid oid);

void insert_into_maintainance_map(object_maintainance *obj);

void delete_from_maintainance_map(object_maintainance *obj);

object_maintainance* create_new_maintainance_map_entry(MEMoidKey key, MEMoid oid, where_t which_ram);


#endif // !__NVM_OBJ_MAINTAIN__
