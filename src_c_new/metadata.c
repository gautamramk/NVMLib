#include "metadata.h"

extern char *program_invocation_short_name;
PMEMobjpool *init_pop;

void initialize_metadata() {
    char init_file_name[50];
    bool opened = true;
    strcpy(init_file_name, program_invocation_short_name);
    strcat(init_file_name, "_init");
    init_pop = pmemobj_open(init_file_name, POBJ_LAYOUT_NAME(init_struct));
    if (!init_pop) {
        opened = false;
        char layout_name[50];
        strcpy(layout_name, POBJ_LAYOUT_NAME(init_struct));
        // strcat(layout_name, "_init_layout");
        init_pop = pmemobj_create(init_file_name, layout_name, PMEMOBJ_MIN_POOL * 4, 0666);
    }

    metadata_root* root = D_RO(POBJ_ROOT(init_pop, struct metadata_root));
    if (!opened) {
        metadata data;
        data.inst_num = 0;
        data.num_pools = 0;
        TX_BEGIN(init_pop) {
            TX_SET_DIRECT(root, init_metadata, data);
        } TX_END
    }
#ifdef DEBUG
    printf("metadata.c num_pools = %d\n", root->init_metadata.num_pools);
#endif
}

void update_num_pools(int num_pools) {
#ifdef DEBUG
    printf("metadata: numpools = %d \n", num_pools);
#endif
    metadata_root* root = D_RW(POBJ_ROOT(init_pop, struct metadata_root));
    metadata data;
    data.inst_num = root->init_metadata.inst_num;
    data.num_pools = num_pools;
    TX_BEGIN(init_pop) {
        TX_SET_DIRECT(root, init_metadata, data);
    } TX_ONABORT {
		fprintf(stderr, "%s: Updation of num_pools aborted: %s\n", __func__,
			pmemobj_errormsg());
		abort();
    } TX_END
}

int retrieve_num_pools() {
    metadata_root* root = D_RO(POBJ_ROOT(init_pop, struct metadata_root));
    return root->init_metadata.num_pools;
}