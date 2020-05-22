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


splay_tree addr2MemOID;

// The struct that stores the memptr for the object.
typedef struct MEMoid_st {
    uint64_t pool_id;
    uint64_t offset;
    size_t size;
    
    // All of these are in object_maintainance.h
    // uint32_t num_reads;
    // uint32_t num_writes;
    // uint64_t *access_bitmap;
} MEMoid;

#define KEY_FIRST(key) (_key_get_first(key))
#define KEY_LAST(key) (_key_get_last(key))

// The key of the HashTable that contains <MEMoidKey, MEMoid>.
typedef uint64_t MEMoidKey;

// Just a dummy obj.
static const MEMoid MEMOID_NULL = { 0, 0, 0 };

// The user facing fnction to allocate memory.
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define NUMARGS(...)  (sizeof((int[]){__VA_ARGS__})/sizeof(int))

// User doesn't worry about where the object is placed
//
// Note this supports both with and wothout `which_ram`
#define memalloc(size, ...) _memalloc(size, __FILENAME__, __func__, __LINE__, NUMARGS(__VA_ARGS__), __VA_ARGS__)
// User specifies where to put the object
// #define memmalloc(size, which_ram) _memalloc(size, which_ram, __FILENAME__, __func__, __LINE__)

// Returns the direct pointer to the mem-object
void* get_memobj_direct(MEMoid obj);

// #define memfree(o) _memfree((o).oidkey, sizeof(__typeof__(*(o)._type)))
// #define memfree(o) _memfree((o).oidkey, get_MEMoid(o).size)
#define memfree(o) _memfree((o).oidkey)

enum splay_comp {
    cmp_node,
    cmp_addr
};
typedef struct addr2memoid_key {
    enum splay_comp comp;
    union {
        void* addr;
        MEMoidKey key;
    }; 

} addr2memoid_key;

#endif // !__NVM_MALLOC__
