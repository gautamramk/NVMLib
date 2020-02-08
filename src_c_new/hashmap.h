#ifndef __NVM_HASHMAP__
#define __NVM__HASHMAP__

/*
* This file provides a generic hashmap stored in DRAM.
*
* This hashmap will be a chaining hashmap, with
* load factor = 0.5
* no. of buckets = power of 2
* hash function for distribution = fibonacci hashing
* hash function for hasing the object = User provided
*/


#include <stdbool.h>
#include <stdlib.h>

typedef __uint128_t uint128_t;
typedef __uint64_t uint64_t;
typedef __uint32_t uint32_t;
typedef __uint16_t uint16_t;
typedef __uint8_t uint8_t;

// makinf them dummy
#define struct
#define enum
#define union


#define LOAD_FACTOR 0.5

#define HASH_MAP(VALUE_TYPE) VALUE_TYPE##_hash_map


inline size_t next_power_of_two(size_t i) {
    --i;
    i |= i >> 1;
    i |= i >> 2;
    i |= i >> 4;
    i |= i >> 8;
    i |= i >> 16;
    i |= i >> 32;
    ++i;
    return i;
}

// Fibonacci hashing
// 11400714819323198485ull = 2^64 / golden_ratio
size_t index_for_hash(size_t hash, size_t shift /*64 - map->power_of_two*/) {
    return (11400714819323198485ull * hash) >> shift;
}


#define DECLARE_HASHMAP(VALUE_TYPE) \
typdef struct _hash_map_##VALUE_TYPE##_structure_st {\
    size_t size;\
    size_t power_of_two;\
    VALUE_TYPE *entries;\
} HASH_MAP(VALUE_TYPE);\
\
\
/*\
* Create new Hash map for the given type.\
* @retrun : the pointer to the created hash map\
*/\
VALUE_TYPE##hash_map* create_new_##VALUE_TYPE##_hash_map();\
\
/*\
* Insert new value in to the hash map.\
* @param map : The map into which insertion is to be made\
* @param entry [In / Out] : The entry to be inserted. If duplicate,\
                            return pointer in here.\
*/\
void insert_into_##VALUE_TYPE##_hash_map(HASH_MAP(VALUE_TYPE) *map, VALUE_TYPE **entry);\
\
/*\
* Looks up an entry in the map\
* @param map : The map in which look up is to be made\
* @param entry [In / Out] : The entry to be searched. If found,\
                            return pointer in here.\
* @return : false, if the entry was not found.\
*/\
bool find_in_##VALUE_TYPE##_hash_map(HASH_MAP(VALUE_TYPE) *map, VALUE_TYPE **result_of_search);\
\
/*\
* Removes entry from the map\
* @param map : The map in which look up is to be made\
* @param entry [In / Out] : The entry to be removed. If found, delete it and \
                            return pointer to the deleted item here.\
* @return : false, if the entry didn't exist.\
*/\
bool erase_from_##VALUE_TYPE##_hash_map(HASH_MAP(VALUE_TYPE) *map, VALUE_TYPE *deleted_entry);\
\
/*\
* Destroys the table, i.e size of table = 0 and capacity = 0 after the call\
*/\
void destroy_##VALUE_TYPE##_hash_map(HASH_MAP(VALUE_TYPE) *map);\


#endif // !__NVM_HASHMAP__
