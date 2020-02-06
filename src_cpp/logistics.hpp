#ifndef __LOGISTICS_NVM__
#define __LOGISTICS_NVM__

#include "globals.hpp"
#include "malloc.hpp"

void init_hashmap();

// adding the new object to hash table
void add_new_object(__UINT64_TYPE__ key, __UINT16_TYPE__ type_of_allocation);  // calls create new obj based on obj.type_of_allocation

template <typename T>
int store_data(__UINT64_TYPE__ key, const T &data);

void* dram_pointer_return(__UINT64_TYPE__ key);

void* nvram_pointer_return(__UINT64_TYPE__ key);

#endif __LOGISTICS_NVM__ // !__LOGISTICS_NVM__ 