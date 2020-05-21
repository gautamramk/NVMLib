#include "metadata.h"

extern char *program_invocation_short_name;

void initialize_metadata() {
    char init_file_name[50];
    bool opened = true;
    strcpy(init_file_name, program_invocation_short_name);
    strcat(init_file_name, "_init");
    init_pop = pmemobj_open(init_file_name, POBJ_LAYOUT_NAME(init_struct));
    if (!init_pop) {
        opened = false;
        char layout_name[50];
        strcpy(layout_name, program_invocation_short_name);
        strcat(layout_name, "_init_layout");
        init_pop = pmemobj_create(init_file_name, layout_name, PMEMOBJ_MIN_POOL, 0666);
    }
    TOID(metadata_root) root = POBJ_ROOT(init_pop, metadata_root);
    if (!opened) {
        D_RW(D_RW(root)->init_metadata)->inst_num = 0;
        D_RW(D_RW(root)->init_metadata)->num_pools = 0;
    }
}

void update_num_pools(int num_pools) {
    TOID(metadata_root) root = POBJ_ROOT(init_pop, metadata_root);
    TX_BEGIN(init_pop) {
        D_RW(D_RW(root)->init_metadata)->num_pools = num_pools;
    } TX_END
}

int retrieve_num_pools() {
    TOID(metadata_root) root = POBJ_ROOT(init_pop, metadata_root);
    return D_RO(D_RO(root)->init_metadata)->num_pools;
}