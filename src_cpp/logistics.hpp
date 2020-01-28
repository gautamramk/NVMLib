#ifndef __LOGISTICS_NVM__
#define __LOGISTICS_NVM__


// adding the new object to hash table
void add_new_object(__UINT64_TYPE__ key);  // calls create new obj based on obj.type_of_allocation

template <typename T>
void store_data(__UINT64_TYPE__ key, const T &data);

void* dram_pointer_return(__UINT64_TYPE__ key);

PMEMoid nvram_pointer_return(__UINT64_TYPE__ key);

#endif __LOGISTICS_NVM__ // !__LOGISTICS_NVM__ 