#ifndef __NVM_HASHMAP__
#define __NVM_HASHMAP__

/**
* This file provides a generic hashmap stored in DRAM.
*
* This hashmap will be a chaining hashmap, with
* load factor = 0.8
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

#define LOAD_FACTOR 0.8

#define HASH_MAP(VALUE_TYPE) VALUE_TYPE##_hash_map
#define HASH_MAP_BUCKET(VALUE_TYPE) VALUE_TYPE##_hash_map_bucket

#define HASH_MAP_INSERT(VALUE_TYPE) insert_into_##VALUE_TYPE##_hash_map
#define HASH_MAP_FIND(VALUE_TYPE) find_in_##VALUE_TYPE##_hash_map
#define HASH_MAP_CREATE(VALUE_TYPE)  create_new_##VALUE_TYPE##_hash_map
#define HASH_MAP_ERASE(VALUE_TYPE) erase_from_##VALUE_TYPE##_hash_map
#define HASH_MAP_DESTROY(VALUE_TYPE) destroy_##VALUE_TYPE##_hash_map

typedef enum {
    HMDR_FAIL = 0,
    HMDR_FIND,
    HMDR_REPLACE,
    HMDR_SWAP,
} HashMapDuplicateResolution; /*HMDR*/

typedef enum {
    HMR_FAILED = 0,
    HMR_FOUND,
    HMR_REPLACED,
    HMR_SWAPPED,
    HMR_INSERTED,
} HashMapResult; /*HMR*/

static inline size_t next_power_of_two(size_t i /*map->power_of_two + 1*/) {
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
static inline size_t index_for_hash(size_t hash, size_t shift, size_t pow_of_two) {
    return ((11400714819323198485ul * hash) >> shift ) % pow_of_two;
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
HashMapResult insert_into_##VALUE_TYPE##_hash_map(HASH_MAP(VALUE_TYPE) *map, VALUE_TYPE **entry, HashMapDuplicateResolution dr);\
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
	FREE(map);\
}\
\
\
HASH_MAP(VALUE_TYPE)*  create_new_##VALUE_TYPE##_hash_map() {\
    HASH_MAP(VALUE_TYPE) *new_map = (HASH_MAP(VALUE_TYPE) *)malloc(sizeof(HASH_MAP(VALUE_TYPE)));\
    new_map->size = 0;\
    new_map->power_of_two = 2;\
	HASH_MAP_BUCKET(VALUE_TYPE) *new_entries = REALLOC(NULL, 2 * sizeof(HASH_MAP_BUCKET(VALUE_TYPE)));\
	memset(new_entries, 0, 2 * sizeof(HASH_MAP_BUCKET(VALUE_TYPE)));\
    new_map->entries = new_entries;\
    return new_map;\
}\
\
\
bool find_in_##VALUE_TYPE##_hash_map(HASH_MAP(VALUE_TYPE) *map, VALUE_TYPE **result_of_search){\
    if(!map->entries) {\
        return false;\
    }\
    HASH_MAP_BUCKET(VALUE_TYPE) *bucket = &map->entries[index_for_hash(\
                                        GET_HASH(*result_of_search), \
                                        64 - __builtin_popcount(map->power_of_two-1), map->power_of_two)];\
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
                                        64 - __builtin_popcount(map->power_of_two-1), map->power_of_two)];\
    for (size_t i = 0; i < bucket->size; i++) {\
        if (CMP(deleted_entry, &bucket->entries[i]) == 0) {\
            *deleted_entry = bucket->entries[i];\
            memmove(&bucket->entries[i], &bucket->entries[i+1],\
                (bucket->size - i - 1) * sizeof(HASH_MAP_BUCKET(VALUE_TYPE)));\
            --bucket->size;\
            if (!bucket->size){\
                /* A bucket is now free */\
                --map->size;\
            }\
            return true;\
        }\
    }\
    return false;\
}\
\
\
VALUE_TYPE* actual_insert_##VALUE_TYPE##_hash_map(HASH_MAP(VALUE_TYPE) *map, VALUE_TYPE *entry){\
    HASH_MAP_BUCKET(VALUE_TYPE) *bucket = &map->entries[index_for_hash(\
                                        GET_HASH(entry), \
                                        64 - __builtin_popcount(map->power_of_two-1), map->power_of_two)];\
    /* check if available to add */\
    size_t old_size = bucket->size;\
    if (bucket->power_of_two < old_size + 1) {\
        size_t new_capacity = next_power_of_two(bucket->power_of_two + 1);\
        bucket->entries = REALLOC(bucket->entries,new_capacity * sizeof(VALUE_TYPE));\
        bucket->power_of_two = new_capacity;\
    }\
    VALUE_TYPE *result = &bucket->entries[old_size];\
    *result = *entry;\
    if (!old_size) {\
        map->size++;\
    }\
    bucket->size++;\
    return result;\
}\
\
\
void hash_map_##VALUE_TYPE##_ensure_size(HASH_MAP(VALUE_TYPE) *map, size_t capacity_required) {\
    size_t capacity = (size_t)((double)map->power_of_two * LOAD_FACTOR);\
    if (capacity_required <= capacity) {\
        return;\
    }\
    size_t old_size = map->size;\
	size_t old_power_of_two = map->power_of_two;\
    HASH_MAP_BUCKET(VALUE_TYPE) *old_entries = map->entries;\
    size_t new_size =  next_power_of_two(map->power_of_two+1);\
    HASH_MAP_BUCKET(VALUE_TYPE) *new_entries = REALLOC(NULL, \
                                    new_size * sizeof(HASH_MAP_BUCKET(VALUE_TYPE)));\
    if(!new_entries) {\
        return;\
    }\
    memset(new_entries, 0, new_size * sizeof(HASH_MAP_BUCKET(VALUE_TYPE)));\
    map->entries = new_entries;\
    map->power_of_two = new_size;\
    map->size = 0; /* needs to be reevaluated */\
    \
    if(old_size) {\
        for (size_t i = 0; i < old_power_of_two; i++) {\
            HASH_MAP_BUCKET(VALUE_TYPE) *bucket = &old_entries[i];\
            for (size_t j = 0; j < bucket->size; j++) {\
                actual_insert_##VALUE_TYPE##_hash_map(map, &bucket->entries[j]);\
            }\
            FREE(bucket->entries);\
        }\
    }\
	FREE(old_entries);\
}\
\
\
HashMapResult insert_into_##VALUE_TYPE##_hash_map(HASH_MAP(VALUE_TYPE) *map, VALUE_TYPE **entry,HashMapDuplicateResolution dr){\
    HashMapResult result = HMR_FAILED;\
    VALUE_TYPE *current = *entry, tmp;\
    if (!HASH_MAP_FIND(VALUE_TYPE)(map, &current)) {\
        current = *entry;\
        result = HMR_INSERTED;\
    } else {\
        switch(dr) {\
            case HMDR_FAIL:\
                *entry = current;\
                return HMR_FAILED;\
            case HMDR_FIND:\
                *entry = current;\
                return HMR_FOUND;\
            case HMDR_REPLACE:\
                *current = **entry;\
                *entry = current;\
                return HMR_REPLACED;\
            case HMDR_SWAP:\
                tmp = *current;\
                *current = **entry;\
                **entry = tmp;\
                *entry = current;\
                return HMR_SWAPPED;\
            default:\
                return HMR_FAILED;\
        }\
    }\
	if (!map->entries[index_for_hash(\
                GET_HASH(current), \
                64 - __builtin_popcount(map->power_of_two-1), \
                map->power_of_two)].size) {\
        /* New bucket is going to be filled */\
        hash_map_##VALUE_TYPE##_ensure_size(map, map->size + 1);\
    }\
    VALUE_TYPE *inserted_entry = actual_insert_##VALUE_TYPE##_hash_map(map, current);\
    if (result == HMR_INSERTED) {\
        *entry = inserted_entry;\
    }\
    return result;\
}

/**
* For Debug purposes
* @param VALUE_TYPE : Type of stored entries. It has to be Typedef'd name.
* @param PRINT_OBJ_FUNC : void (*print_obj)(VALUE_TYPE entry).
*            A function to print the contents of the stored object. 
*            Could be a marco too.
*/
#define HASH_MAP_PRINT(VALUE_TYPE) print_##VALUE_TYPE##_hash_map

#define HASH_MAP_PRINT_FUNC(VALUE_TYPE, PRINT_OBJ_FUNC) \
void print_##VALUE_TYPE##_hash_map(HASH_MAP(VALUE_TYPE) *map) {\
	if(!map->entries){\
		return;\
	}\
	for(size_t i = 0; i < map->power_of_two ; i++) {\
		printf("%ld ---> ", i);\
		HASH_MAP_BUCKET(VALUE_TYPE) *bucket = &map->entries[i];\
		printf("size = %ld || ", bucket->size);\
		for(int j = 0; j < bucket->size; j++) {\
			PRINT_OBJ_FUNC(&bucket->entries[j]);\
			printf(" | ");\
		}\
		printf("\n");\
	}\
	printf("\n");\
}

#endif // !__NVM_HASHMAP__
