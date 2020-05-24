#ifndef __NVM_METADATA_H__
#define __NVM_METADATA_H__

#include "globals.h"
#include <libpmemobj.h>

POBJ_LAYOUT_BEGIN(init);
POBJ_LAYOUT_ROOT(init, struct metadata_root)
POBJ_LAYOUT_TOID(init, struct metadata);
POBJ_LAYOUT_END(init);

typedef struct metadata {
    int num_pools;
    uint64_t inst_num;
} metadata;

typedef struct metadata_root {
    metadata init_metadata;
} metadata_root;

void initialize_metadata();

void update_num_pools(int numpools);

int retrieve_num_pools();


#endif // !__NVM_METADATA_H__