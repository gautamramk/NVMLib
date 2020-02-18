#ifndef __NVM_HASHMAP_TX_H
#define __NVM_HASHMAP_TX_H

#include <stddef.h>
#include <stdint.h>
#include <libpmemobj.h>
#include "malloc.h"


// #include <hashmap.h>

struct hashmap_args {
	uint32_t seed;
};

enum hashmap_cmd {
	HASHMAP_CMD_REBUILD,
	HASHMAP_CMD_DEBUG,
};

#ifndef HASHMAP_TX_TYPE_OFFSET
#define HASHMAP_TX_TYPE_OFFSET 1004
#endif

struct hashmap_tx;
TOID_DECLARE(struct hashmap_tx, HASHMAP_TX_TYPE_OFFSET + 0);

int hm_tx_check(PMEMobjpool *pop, TOID(struct hashmap_tx) hashmap);
int hm_tx_create(PMEMobjpool *pop, TOID(struct hashmap_tx) *map, void *arg);
int hm_tx_init(PMEMobjpool *pop, TOID(struct hashmap_tx) hashmap);
int hm_tx_insert(PMEMobjpool *pop, TOID(struct hashmap_tx) hashmap,
		uint64_t key, MEMoid value);
MEMoid hm_tx_remove(PMEMobjpool *pop, TOID(struct hashmap_tx) hashmap,
		uint64_t key);
MEMoid hm_tx_get(PMEMobjpool *pop, TOID(struct hashmap_tx) hashmap,
		uint64_t key);
int hm_tx_lookup(PMEMobjpool *pop, TOID(struct hashmap_tx) hashmap,
		uint64_t key);
int hm_tx_foreach(PMEMobjpool *pop, TOID(struct hashmap_tx) hashmap,
	int (*cb)(uint64_t key, MEMoid value, void *arg), void *arg);
size_t hm_tx_count(PMEMobjpool *pop, TOID(struct hashmap_tx) hashmap);
int hm_tx_cmd(PMEMobjpool *pop, TOID(struct hashmap_tx) hashmap,
		unsigned cmd, uint64_t arg);

#endif // !__NVM_HASHMAP_TX_H 
