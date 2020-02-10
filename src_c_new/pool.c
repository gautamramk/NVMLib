#include "pool.h"

struct pool_free_slot* pool_free_slot_head;
struct pool_alloc* pool_alloc_head;

int initialize_pool() {
    pool_free_slot_head = (struct pool_free_slot*)malloc(sizeof(struct pool_free_slot));
    pool_free_slot_head->start_b = 0;
    pool_free_slot_head->end_b = POOL_SIZE - 1;
    pool_free_slot_head->next = NULL;
    pool_free_slot_head->prev = NULL;
    pool_alloc_head = NULL;
}

static inline uint64_t free_slot_size(struct pool_free_slot * slot) {
    return slot->end_b - slot->start_b + 1;
}


uint64_t get_current_free_offset(size_t size) {
    // LOG shit

    return allot_current_free_offset(size);
}

uint64_t allot_first_free_offset(size_t size) {
    struct pool_free_slot* it = pool_free_slot_head;
    uint64_t ret = -1;
    while (it) {
        if (free_slot_size(it) == size) {
            if (it->prev)
                it->prev->next = it->next;
            if (it->next)
                it->next->prev = it->prev;
            ret = it->start_b;
            break;
        } else if (free_slot_size(it) > size) {
            ret = it->start_b;
            it->start_b += size;
            break;
        }
        it = it->next;
    }

    return ret;
}


void nvm_free(uint64_t pool_id, uint64_t offset, size_t size) {
    struct pool_free_slot* it = pool_free_slot_head;
    while (it) {
        if (offset == it->end_b + 1) {
            it->end_b += size - 1;
        } else if (offset > it->end_b + 1) {
            struct pool_free_slot* new_slot = (struct pool_free_slot*)malloc(sizeof(struct pool_free_slot));
            new_slot->prev = it;
            new_slot->next = it->next;
            it->next = new_slot;
            new_slot->start_b = offset;
            new_slot->end_b = offset + size - 1;

            if (new_slot->next && new_slot->end_b == new_slot->next->start_b - 1) {
                new_slot->end_b = new_slot->next->end_b;
                struct pool_free_slot* temp = new_slot->next;
                new_slot->next = new_slot->next->next;
                free(temp);
            }
            it = it->next;
        }
    }
}
