#ifndef __NVM_GLOBALS__
#define __NVM_GLOBALS__

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <uv.h>
#include <errno.h>


// Types of allocation
#define DRAM_STACK 0
#define NVRAM_STACK 1
#define DRAM_HEAP 2
#define NVRAM_HEAP 3
#define ANY_RAM 4

typedef __uint128_t uint128_t;
typedef __uint64_t uint64_t;
typedef __uint32_t uint32_t;
typedef __uint16_t uint16_t;
typedef __uint8_t uint8_t;

typedef unsigned long int uintptr_t;
typedef long int intptr_t;

// The predefined pools
extern char* program_invocation_short_name;
#define TYPES_TABLE_POOL strcat(program_invocation_short_name, "types_table")
#define ALLOC_LIST_POOL strcat(program_invocation_short_name, "alloc_list")

// The predefined layouts
#define TYPES_TABLE_LAYOUT "types_table"
#define  ALLOC_LIST_LAYOUT "alloc_list" 

// The logging files
#define MAIN_LOG_FILE_NAME "./nvm_main_log.log"
extern FILE *main_log_file_fd;
#define MAIN_LOG_FILE main_log_file_fd

// The mutex required when accessing the Types map
extern uv_mutex_t object_maintainence_hashmap_mutex;   // used during manupulation of `types map`
extern uv_mutex_t object_maintainence_memory_mutex;    // used during `nvm_free` / access too

#endif // !__NVM_GLOBALS__
