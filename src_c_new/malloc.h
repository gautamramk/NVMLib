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

typedef struct MEMoid_st {
    uint64_t pool_id;
    uint64_t offset;
} MEMoid;

// Just a dummy obj.
static const MEMoid OID_NULL = { 0, 0 };

// The user facing fnction to allocate memory.
MEMoid memalloc(size_t size);

// Returns the direct pointer to the mem-object
inline void* get_memobj_direct(MEMoid obj);

#endif // !__NVM_MALLOC__
