#include "globals.h"
#include <libpmemobj.h>
#include <string.h>
#include <libc-internal.h>

extern const char *__progname;

PMEMobjpool *init_pop;

typedef struct init_root {
    int num_pools;
    uint64_t inst_num;
} init_root;

// Initializes all the metadata and data pools and loads corresponding
// information into hashmaps.

void initialize() {
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


    // Opening the logging files
    open_logging_files();
}

// Open the logging files
void open_logging_files() {
    main_log_file_fd = fopen(MAIN_LOG_FILE_NAME, "w");
}