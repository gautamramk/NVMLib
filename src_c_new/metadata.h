#ifndef __NVM_METADATA_H__
#define __NVM_METADATA_H__

#include "globals.h"

typedef struct init_root {
    int num_pools;
    uint64_t inst_num;
} init_root;

PMEMobjpool *init_pop;

void initialize_metadata();

void update_num_pools(int numpools);

int retrieve_num_pools();


#endif __NVM_METADATA_H__