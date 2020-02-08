#ifndef __NVM_HASHMAP__
#define __NVM__HASHMAP__

/**
* This file provides a generic hashmap stored in DRAM.
*
* This hashmap will be a chaining hashmap, with
* load factor = 0.5
* no. of buckets = power of 2
* hash function for distribution = fibonacci hashing
* hash function for hashing the object = User provided
*/


#include <stdbool.h>
#include <stdlib.h>

typedef __uint128_t uint128_t;
typedef __uint64_t uint64_t;
typedef __uint32_t uint32_t;
typedef __uint16_t uint16_t;
typedef __uint8_t uint8_t;

#define LOAD_FACTOR 0.5

#define HASH_MAP(VALUE_TYPE) VALUE_TYPE##_hash_map
#define HASH_MAP_BUCKET(VALUE_TYPE) VALUE_TYPE##_hash_map_bucket

#define HASH_MAP_INSERT(VALUE_TYPE) insert_into_##VALUE_TYPE##_hash_map
#define HASH_MAP_FIND(VALUE_TYPE) find_in_##VALUE_TYPE##_hash_map
#define HASH_MAP_CREATE(VALUE_TYPE)  create_new_##VALUE_TYPE##_hash_map
#define HASH_MAP_ERASE(VALUE_TYPE) erase_from_##VALUE_TYPE##_hash_map
#define HASH_MAP_DESTROY(VALUE_TYPE) destroy_##VALUE_TYPE##_hash_map


inline size_t next_power_of_two(size_t i /*map->power_of_two + 1*/) {
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


#define _HASH_STRUCTURE(VALUE_TYPE)\
struct {\
    size_t size; /*current size*/\
    size_t power_of_two; /*capacity*/\
    VALUE_TYPE *entries;\
}

/**
* Declaration of HashMap of a given type
* @param VALUE_TYPE : The type of entries that would be stored. 
*                     It has to be the Typedef'd name, i.e 
*                     `struct abc` is not allowed.
*/
#define DECLARE_HASHMAP(VALUE_TYPE) \
\
/*\
* The structure of `root nodes` of each `bucket`\
*/\
typedef _HASH_STRUCTURE(VALUE_TYPE) HASH_MAP_BUCKET(VALUE_TYPE);\
\
/*\
* The structure of `root node` `hash map`\
*/\
typedef _HASH_STRUCTURE(HASH_MAP_BUCKET(VALUE_TYPE)) HASH_MAP(VALUE_TYPE);\
\
\
/*\
* Create new Hash map for the given type.\
* @retrun : the pointer to the created hash map\
*/\
HASH_MAP(VALUE_TYPE)* create_new_##VALUE_TYPE##_hash_map();\
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
void destroy_##VALUE_TYPE##_hash_map(HASH_MAP(VALUE_TYPE) *map);





/**
* Definition of the declared HashMap
* @param VALUE_TYPE : Type of stored entries. It has to be Typedef'd name.
* @param CMP : int (*cmp)(VALUE_TYPE *left, VALUE_TYPE *right).
*            Could easily be a macro. Must return 0 if and only if *left
*            equals *right.
* @param GET_HASH : inttype (*getHash)(VALUE_TYPE *entry). Could easily be
*                 a macro.
* @param FREE : free() to use
* @param REALLOC : realloc() to use.
*/
#define DEFINE_HASHMAP(VALUE_TYPE, CMP, GET_HASH, FREE, REALLOC)\
\
\
void destroy_##VALUE_TYPE##_hash_map(HASH_MAP(VALUE_TYPE) *map) {\
    if (map->entries) {\
        for (size_t i = 0; i < map->power_of_two; i++) {\
            if(map->entries[i].entries) {\
                FREE(map->entries[i].entries);\
            }\
        }\
    }\
    FREE(map->entries);\
    map->size = 0;\
    map->power_of_two = 0;\
    map->entries = NULL\
}\
\
\
HASH_MAP(VALUE_TYPE)*  create_new_##VALUE_TYPE##_hash_map() {\
    HASH_MAP(VALUE_TYPE) *new_map = (HASH_MAP(VALUE_TYPE) *)malloc(sizeof(HASH_MAP(VALUE_TYPE)));\
    new_map->size = 0;\
    new_map->power_of_two = 0;\
    new_map->entries = NULL;\
    return new_map;\
}\
\
\
bool find_in_##VALUE_TYPE##_hash_map(HASH_MAP(VALUE_TYPE) *map, VALUE_TYPE **result_of_search){\
    if(!map->entries) {\
        return false\
    }\
    HASH_MAP_BUCKET(VALUE_TYPE) *bucket = &map->entries[index_for_hash(\
                                        GET_HASH(*result_of_search), \
                                        64 - map->power_of_two)];\
    for (size_t i = 0; i < bucket->size; i++) {\
        if (!(CMP((&bucket->entries[i]), (*result_of_search)))) {\
            *result_of_search = &bucket->entries[i];\
            return true;\
        }\
    }\
    return false;\
}\
\
\
bool erase_from_##VALUE_TYPE##_hash_map(HASH_MAP(VALUE_TYPE) *map, VALUE_TYPE *deleted_entry){\
    HASH_MAP_BUCKET(VALUE_TYPE) *bucket = &map->entries[index_for_hash(\
                                        GET_HASH(deleted_entry),\
                                        64 - map->power_of_two)];\
    for (size_t i; i < bucket->size; i++) {\
        if (!(CMP(deleted_entry, &bucket->entry[i]))) {\
            *deletd_entry = bucket->entry[i];\
            memmove(&bucket->entry[i], &bucket->entry[i+1],\
                (bucket->size - i - 1) * sizeof(HASH_MAP_BUCKET(VALUE_TYPE)));\
            --bucket->size;\
            --map->size;\
            return true;\
        }\
    }\
    return false;\
}\
\
\



#endif // !__NVM_HASHMAP__
