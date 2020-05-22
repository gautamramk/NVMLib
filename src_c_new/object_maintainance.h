#ifndef __NVM_OBJ_MAINTAIN__
#define __NVM_OBJ_MAINTAIN__

#include <uv.h>
#include "globals.h"
#include "malloc.h"
#include <time.h>

#define DELETE_LOOP_SLEEP_TIME 5    // 5 milli secs
#define MOVE_LOOP_SLEEP_TIME 5    // 5 milli secs

typedef enum access_type_en {
    ACCESS_UNKNOWN,
    READ,
    WRITE
} access_types_t;

typedef enum shitf_levels_en {
    JUST_ENTERED,   // After it has entered a RAM
    ONE,
    TWO,
    THREE,
    FOUR,
    MOVE_NOW    // Move in the next cycle
} shift_levels_t;

typedef enum where_en {
    RAM_UNKNOWN,    // A default
    DRAM,
    NVRAM,
    NO_RAM    // to be deleted
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

    uint64_t *write_bitmap;
    uint64_t *read_bitmap;

    uint64_t w_entropy;
    uint64_t r_entropy;
    void* last_write;
    void* last_read;
    size_t last_write_size;
    size_t last_read_size;
    size_t bytes_write;
    size_t bytes_read;

    where_t which_ram;

    shift_levels_t shift_level;

    bool can_be_moved;
} object_maintainance;

void initialise_logistics();

// uv_loop_t* get_logistics_loop();

// Returns the actaul object in the map as a pointer.
object_maintainance* find_in_maintainance_map(MEMoidKey key);

void insert_into_maintainance_map(object_maintainance *obj);

void delete_from_maintainance_map(object_maintainance *obj);

object_maintainance* create_new_maintainance_map_entry(MEMoidKey key, MEMoid oid, where_t which_ram, bool can_be_moved);

#endif // !__NVM_OBJ_MAINTAIN__
