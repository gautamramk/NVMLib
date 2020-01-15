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

#define access(type_t, ptr) ({ \
    (type_t *)access_var(ptr); \
})

typedef struct nvmalloc_rt {
    u_int16_t type_of_allocation;
    size_t size;
    u_int64_t key;
} nvmalloc;

// The top level function for allocation of space in heap.
nvmalloc* nvmmalloc(size_t size);

nvmalloc* nvmmalloc(size_t size, __auto_type data);

// Top level function for accessing the data
void *access_var(nvmalloc *ptr);

// Top level function for writing into the variable
void write_var(void *data, nvmalloc *ptr);

#endif // !__NVM_MALLOC__
