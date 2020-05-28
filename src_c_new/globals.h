#ifndef __NVM_GLOBALS__
#define __NVM_GLOBALS__

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <uv.h>
#include <errno.h>


/**
 * Types of allocation
 * 
 * When the object is supposed to be in `DRAM stack`
 * @see decide_allocation()
 */
#define DRAM_STACK 0
/**
 * Types of allocation
 * 
 * When the object is supposed to be in `NVRAM stack`
 * @see decide_allocation()
 */
#define NVRAM_STACK 1
/**
 * Types of allocation
 * 
 * When the object is supposed to be in `DRAM heap`
 * @see decide_allocation()
 */
#define DRAM_HEAP 2
/**
 * Types of allocation
 * 
 * When the object is supposed to be in `NVRAM heap`
 * @see decide_allocation()
 */
#define NVRAM_HEAP 3
/**
 * Types of allocation
 * 
 * When the object can be placed anywhere.
 * @note This is for *internal usage* only. It is not exposed outside.
 * @see decide_allocation() _memalloc()
 */
#define ANY_RAM 4

typedef __uint128_t uint128_t;
typedef __uint64_t uint64_t;
typedef __uint32_t uint32_t;
typedef __uint16_t uint16_t;
typedef __uint8_t uint8_t;

typedef unsigned long int uintptr_t;
typedef long int intptr_t;

/// The predefined pools
extern char* program_invocation_short_name;
#define TYPES_TABLE_POOL strcat(program_invocation_short_name, "types_table")
#define ALLOC_LIST_POOL strcat(program_invocation_short_name, "alloc_list")

// The predefined layouts
#define TYPES_TABLE_LAYOUT "types_table"
#define  ALLOC_LIST_LAYOUT "alloc_list" 

/// The main log file where all the program runtime related info is recorded.
#define MAIN_LOG_FILE_NAME "./nvm_main_log.log"
FILE *main_log_file_fd;
#define MAIN_LOG_FILE main_log_file_fd


/// The mutex used during manupulation of `types map`
extern uv_mutex_t object_maintainence_hashmap_mutex;  
/// The mutex used during `nvm_free` / access too 
extern uv_mutex_t object_maintainence_memory_mutex;
/// The mutex used during manupulation of `maintainance map`
extern uv_mutex_t object_maintainence_maintain_map_mutex;
/// The mutex used during manupulation of `maintainance map`
extern uv_mutex_t object_maintainence_addtion_mutex;
/// The mutex used during manupulation of `maintainance map`
extern uv_mutex_t object_maintainence_deletion_mutex;
/// The mutex used during manupulation of `read splay tree`
extern uv_mutex_t read_splay_tree_mutex;
/// The mutex used during manupulation of `read splay tree`
extern uv_mutex_t write_splay_tree_mutex;
#endif // !__NVM_GLOBALS__
