#ifndef __NVM_METADATA_H__
#define __NVM_METADATA_H__

#include "globals.h"
#include <libpmemobj.h>


typedef struct metadata_st {
    int num_pools;
    uint64_t inst_num;
} metadata;

typedef struct metadata_root_str {
    TOID(metadata) init_metadata;
} metadata_root;

POBJ_LAYOUT_BEGIN(init);
POBJ_LAYOUT_ROOT(init, metadata_root);
POBJ_LAYOUT_TOID(init, metadata);
POBJ_LAYOUT_END(init);

PMEMobjpool *init_pop;

void initialize_metadata();

void update_num_pools(int numpools);

int retrieve_num_pools();


#endif // !__NVM_METADATA_H__