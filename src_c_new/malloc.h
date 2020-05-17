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

// The struct that stores the memptr for the object.
typedef struct MEMoid_st {
    uint64_t pool_id;
    uint64_t offset;
} MEMoid;

// The key of the HashTable that contains <MEMoidKey, MEMoid>.
typedef uint64_t MEMoidKey;

// Just a dummy obj.
static const MEMoid OID_NULL = { 0, 0 };

// The user facing fnction to allocate memory.
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define memalloc(size) _memalloc(size, __FILENAME__, __func__, __LINE__)

// Returns the direct pointer to the mem-object
inline void* get_memobj_direct(MEMoid obj);

#define memfree(o) _memfree((o).oidkey, sizeof(__typeof__(*(o)._type)))

#endif // !__NVM_MALLOC__
