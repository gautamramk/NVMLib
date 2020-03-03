#include "pool.h"
#include <libpmemobj.h>

//struct pool_free_slot* pool_free_slot_head;
PMEMobjpool *pop;
struct pool_free_slot_head *f_head;


int initialize_pool() {
    pop = pmemobj_open("free_slots", POBJ_LAYOUT_NAME(list));
    if (pop == NULL) {
		perror("pmemobj_create");
		return 1;
	}
    TOID(struct pool_free_slots_root) root = POBJ_ROOT(pop, struct pool_free_slots_root);
    f_head = &D_RW(root)->head;
    /*
    pool_free_slot_head = (struct pool_free_slot*)malloc(sizeof(struct pool_free_slot));
    pool_free_slot_head->start_b = 0;
    pool_free_slot_head->end_b = POOL_SIZE - 1;
    pool_free_slot_head->next = NULL;
    pool_free_slot_head->prev = NULL;
    pool_alloc_head = NULL;
    */
}

static inline uint64_t free_slot_size(TOID(struct pool_free_slot) slot) {
    return D_RO(slot)->end_b - D_RO(slot)->start_b + 1;
}


uint64_t allot_first_free_offset(size_t size) {
    // LOG shit
    uint64_t ret = -1;
    TOID(struct pool_free_slot) node;
    POBJ_TAILQ_FOREACH (node, f_head, fnd) {
        if (free_slot_size(node) == size) {
            ret = D_RO(node)->start_b
            TX_BEGIN(pop) {
                POBJ_TAILQ_REMOVE_FREE(f_head, node, fnd);
            } TX_END
            break;
        } else if (free_slot_size(node) > size) {
            ret = D_RO(node)->start_b
            uint64_t new_start = D_RO(node)->start_b + size;
            D_RW(node)->start_b = new_start;
            break;
        }
    }
    return ret;
}

void nvm_free(uint64_t pool_id, uint64_t offset, size_t size) {
    TOID(struct pool_free_slot) node;
    POBJ_TAILQ_FOREACH (node, f_head, fnd) {
        if (offset == D_RO(node)->end_b + 1) {
            D_RW(node)->end_b = D_RO(node)->end_b + size;
        } else if (offset > D_RO(node)->end_b + 1) {
            if (!TOID_IS_NULL(POBJ_TAILQ_NEXT(node, fnd))) {
                if (offset + size - 1 == D_RO(POBJ_TAILQ_NEXT(node, fnd))->start_b - 1) {
                    D_RW(POBJ_TAILQ_NEXT(node, fnd))->start_b = offset;
                }
            } else {
                TOID(struct pool_free_slot) new_node = TX_NEW(struct pool_free_slot);
                POBJ_TAILQ_INSERT_AFTER(node, new_node, fnd);
                D_RW(new_node)->start_b = offset;
                D_RW(new_node)->end_b = offset + size - 1;
            }
        }
    }
}

/*
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
*/