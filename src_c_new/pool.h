#ifndef __NVM_POOL__
#define __NVM_POOL__

#include "globals.h"
#include "free_slot_list.h"
#include <libpmemobj.h>

#define POOL_ID_MALLOC_OBJ -1

#define POOL_SIZE (2 << 12) // Pool size is 4kB


POBJ_LAYOUT_BEGIN(free_slot_layout);
POBJ_LAYOUT_ROOT(free_slot_layout, struct pool_free_slots_root);
POBJ_LAYOUT_TOID(free_slot_layout, struct pool_free_slot);
POBJ_LAYOUT_END(free_slot_layout);


typedef struct pool_free_slot {
    uint64_t start_b;
    uint64_t end_b;
    POBJ_TAILQ_ENTRY(struct pool_free_slot) fnd;
} pool_free_slot;
POBJ_TAILQ_HEAD(pool_free_slot_head, struct pool_free_slot);
typedef struct pool_free_slot_head pool_free_slot_head;

struct pool_free_slots_root {
    struct pool_free_slot_head head;
};

// Initialise the metadata datastructures
int initialize_pool();

// Returns the memory mapped file pointer of the current pool
uintptr_t get_pool_from_poolid(uint64_t pool_id);

// Returns the current number of the pool
uint64_t get_current_poolid();

// Returns the offset of the pointer to the allocated space in NVRAM
uint64_t get_first_free_offset(size_t size);

// Allocates the requested space in NVRAM and returns the offset of
// the pointer to the allocated space.
// This is called internally by `get_current_free_offset()`
uint64_t allot_current_free_offset(size_t size);

// Frees the allocated spaces
void nvm_free(uint64_t pool_id, uint64_t offset, size_t size);

#endif // !__NVM_POOL__
