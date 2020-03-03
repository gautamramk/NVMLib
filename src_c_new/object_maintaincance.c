#include "object_maintainance.h"
#include "types.h"
#include "malloc.h"
#include <libpmemobj.h>

uv_loop_t *loop;

void initialise_logistics() {
    loop = uv_loop_new();

    uv_work_t *work = (uv_work_t *)malloc(size(uv_work_t));
    work->loop = loop;

    // add other shit... decide as you go
    uv_queue_work(loop, work, delete_objects, NULL); // add "after_work_cb" is required later

    // Start the logistics thread
    // Start in `UV_RUN_DEFAULT` mode
    uv_run(loop);
}

uv_loop_t* get_logistics_loop() {
    return uv_is_active(loop) ? loop : NULL;
}

// never return unless faults
void delete_objects() {
    TOID(struct hashmap_tx) *hashmap = get_types_map();

    TOID(struct buckets) buckets = D_RO(hashmap)->buckets;
	TOID(struct entry) var;

	for (size_t i = 0; i < D_RO(buckets)->nbuckets; ++i) {
		if (TOID_IS_NULL(D_RO(buckets)->bucket[i]))
			continue;

		for (var = D_RO(buckets)->bucket[i]; !TOID_IS_NULL(var);
				var = D_RO(var)->next) {
			ret = cb(D_RO(var)->key, D_RO(var)->value);
            if(ret) {
                // Needs to be deleted
                memfree(D_RO(var)->value);
            }
		}
	}

    // resume operation every 5 min
    sleep(5);
}


// Checks if an object can be deleted or not
// @return : 1 - If can be deleted
//           0 - otherwise
int check_if_required_to_delete(MEMoidKey key, MEMoid oid) {
    // To be completed
    return 0
}