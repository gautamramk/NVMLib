#include "metadata.h"
#include <libpmemobj.h>
#include <string.h>

void initialize_metadata() {
    char[50] init_file_name;
    bool opened = true;
    strcpy(init_file_name, __progname);
    strcat(init_file_name, "_init");
    init_pop = pmemobj_open(init_file_name, POBJ_LAYOUT_NAME(init_struct));
    if (!init_pop) {
        opened = false;
        init_pop = pmemobj_create(init_file_name, LAYOUT_NAME, PMEMOBJ_MIN_POOL, 0666);
    }
    TOID(init_root) root = POBJ_ROOT(init_pop, init_root);
    if (!opened) {
        D_RW(root)->num_pools = 0;
    }
}

void update_num_pools(int num_pools) {
    TOID(init_root) root = POBJ_ROOT(init_pop, init_root);
    TX_BEGIN(init_pop) {
        D_RW(init_root)->num_pools = num_pools;
    } TX_END
}

int retrieve_num_pools() {
    TOID(init_root) root = POBJ_ROOT(init_pop, init_root);
    return D_RO(init_root)->num_pools;
}