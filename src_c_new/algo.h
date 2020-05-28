#ifndef __NVM_ALGO__
#define __NVM_ALGO__

#include "globals.h"
#include "malloc.h"

// void update_read_access(MEMoidKey key);

// void update_write_access(MEMoidKey key);

// void update_time_of_access(MEMoidKey key);

// void update_access_number(MEMoidKey key);

/**
 * Function to decide the initial allocation of object.
 * 
 * It decides where to place the object when the programmer doesn't 
 * provide it himself. 
 * @param size: The `size` of the object being allocated.
 * @return: Either NVRAM_HEAP, DRAM_HEAP, NVRAM_STACK, DRAM_STACK
 */
int decide_allocation(size_t size) {
    return DRAM_HEAP;
}

#endif // !__NVM__ALGO__