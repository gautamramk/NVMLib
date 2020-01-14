#ifndef __NVM_MALLOC__
#define __NVM_MALLOC__

/*
* This file contains the definitions of all the functions related to 
* `Heap allocation` of variables.
*
* Idea :
*                    (if to be sent to NVRAM)
*                    ........................  libpmem.io
*                   /
* nvmmalloc() ------
*                   \........................  malloc()
*                    (if to be sent to DRAM)
*                     
*/

#include "globals.h"

typedef struct nvmalloc_rt {
    u_int16_t type_of_allcoation;
    size_t size;
    void *access;
} nvmalloc;

// The top level function for allocation of space in heap.
nvmalloc nvmmalloc(size_t size);

nvmalloc nvmmalloc(size_t size, __auto_type data);




#endif // !__NVM_MALLOC__
