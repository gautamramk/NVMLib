#include "pool.h"

struct pool_free_slot* pool_free_slot_head;

int initialize_pool() {
    pool_free_slot_head = (struct pool_free_slot*)malloc(sizeof(struct pool_free_slot));
    pool_free_slot_head->start_b = 0;
    pool_free_slot_head->end_b = POOL_SIZE - 1;
    pool_free_slot_head->next = NULL;
    pool_free_slot_head->prev = NULL;
}

inline uint64_t free_slot_size(struct pool_free_slot * slot) {
    return slot->end_b - slot->start_b + 1;
}

uint64_t get_current_free_offset(size_t size) {
    struct pool_free_slot it = pool_free_slot_head;
    uint64_t ret = -1;
    while (it) {
        if (free_slot_size(it) == size) {
            if (it->prev)
                it->prev->next = it->next;
            if (it->next)
                it->next->prev = it->prev;
            ret = it->start_b;
            free(it);
        } else if (free_slot_size(it) > size) {
            ret = it->start_b;
            it->start_b += size;
        }
        it = it->next;
    }
}