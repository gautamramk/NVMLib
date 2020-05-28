#ifndef __NVM_MALLOC__
#define __NVM_MALLOC__

/*
* This file contains the definitions of all the functions related to 
* `Heap allocation` of variables.
*
* Idea :
*                    (if to be sent to NVRAM)
*                    ........................  libpmem.h
*                   /
* memalloc() ------
*                   \........................  malloc()
*                    (if to be sent to DRAM)
*                     
*/

#include "globals.h"
#include "pool.h"
#include <stdint.h>
#include <libiberty/splay-tree.h>
#include <stdarg.h>


splay_tree addr2MemOID_read;
splay_tree addr2MemOID_write;

/// The struct that stores the `memptr` for the object.
typedef struct MEMoid_st {
    uint64_t pool_id;
    uint64_t offset;
    size_t size;
    
    // All of these are in object_maintainance.h
    // uint32_t num_reads;
    // uint32_t num_writes;
    // uint64_t *access_bitmap;
} MEMoid;

// Allocates the requested space in NVRAM and returns the offset of
// the pointer to the allocated space.
// This is called internally by `get_current_free_offset()`
uint64_t allot_first_free_offset_pool(uint64_t pool_id, size_t size);

MEMoid allot_first_free_offset(size_t size);

/// The `key` of the `HashTable` that contains `<MEMoidKey, MEMoid>`.
/// This is the library level representation of `pointer` to the object.
typedef uint64_t MEMoidKey;

/// Just a dummy obj.
static const MEMoid MEMOID_NULL = { 0, 0, 0 };

// The user facing fnction to allocate memory.
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define NUMARGS(...)  (sizeof((int[]){0, ##__VA_ARGS__})/sizeof(int)-1)

// User doesn't worry about where the object is placed
//
// Note this supports both with and wothout `which_ram`
MEMoidKey _memalloc(size_t size, const char *file, const char *func, const int line, int num_args, ...);

/**
 * The user API for requesting allocation of object.
 * 
 * ### Example
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~.cpp
 * LIB_TOID(int) array = LIB_TOID(int)memalloc(10 * sizeof(int));  // allocated an integer array of size 10.
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 
 * @param size: the `size` of memory that needs to be allocated.
 * @param which_ram (optional): the RAM in which the user wants to place the object
 * @return `MEMoidKey` that the user needs to `typecast` into `LIB_TOID`.
 * 
 * @note The API interface and functionality is almost exactly similar to that of `malloc()`.
 * @see _memalloc() __memalloc()
 */
#define memalloc(size, ...) _memalloc(size, __FILENAME__, __func__, __LINE__, NUMARGS(__VA_ARGS__), ##__VA_ARGS__)
// User specifies where to put the object
// #define memmalloc(size, which_ram) _memalloc(size, which_ram, __FILENAME__, __func__, __LINE__)

// Returns the direct pointer to the mem-object
void* get_memobj_direct(MEMoid obj);

void* _key_get_last(MEMoidKey key);
void* _key_get_first(MEMoidKey key);

#define KEY_FIRST(key) (_key_get_first(key))
#define KEY_LAST(key) (_key_get_last(key))

// #define memfree(o) _memfree((o).oidkey, sizeof(__typeof__(*(o)._type)))
// #define memfree(o) _memfree((o).oidkey, get_MEMoid(o).size)
void _memfree(MEMoidKey oidkey);

/**
 * The user API to `free` the memory space allocated.
 * 
 * ### Example
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~.cpp
 * memfree(array);  // freed the array that had bee allocated
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 
 * @param LIB_TOID object: the object that was return by memalloc() during allocation.
 * 
 * @note The API interface and functionality is almost exactly similar to that of `free()`.
 * @see _memfree() nvm_free()
 */
#define memfree(o) _memfree((o).oidkey)

enum splay_comp {
    cmp_node,
    cmp_addr
};

/// The structure used in `spaly_tree` which used for getting `MEMoid` from the memory address.
typedef struct addr2memoid_key {
    enum splay_comp comp;
    union {
        void* addr;
        MEMoidKey key;
    }; 

} addr2memoid_key;

void init_splay();
#endif // !__NVM_MALLOC__
