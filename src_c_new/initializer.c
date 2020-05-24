#include "globals.h"
#include <libpmemobj.h>
#include <string.h>
#include "metadata.h"
#include "pool.h"
#include "malloc.h"
#include "types.h"
#include "object_maintainance.h"

extern char *program_invocation_short_name;

// Open the logging files
void open_logging_files() {
    main_log_file_fd = fopen(MAIN_LOG_FILE_NAME, "w");
}

// Initializes all the metadata and data pools and loads corresponding
// information into hashmaps.

void initialize() {
    printf("1\n");
    init_splay();
    printf("2\n");
    init_types_table();
    // Initialize metadata and open metadata pools
    printf("3\n");

    initialize_metadata();
    printf("4\n");

    // Open data pools
    initialize_pool();
    // Opening the logging files
    printf("5\n");

    open_logging_files();
    initialise_logistics();
}
