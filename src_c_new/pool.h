#ifndef __NVM_POOL__
#define __NVM__POOL__

#include "globals.h"

#define POOL_ID_MALLOC_OBJ -1

#define POOL_SIZE (2 << 12) // Pool size is 4kB

struct pool_free_slot {
    uint64_t start_b;
    uint64_t end_b;
    struct pool_free_slot* prev;
    struct pool_free_slot*
}

uintptr_t get_pool_from_poolid(uint64_t pool_id);

int initialize_pool();

uint64_t get_current_poolid();

uint64_t get_current_free_offset(size_t size);

// Increment the free offset counter, so that we know where the free
// space is.
void update_offset(size_t size);

#endif // !__NVM_POOL__
