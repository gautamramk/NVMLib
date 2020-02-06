#include <unordered_map>
#include <libpmemobj/p.hpp>
#include <libpmemobj/persistent_ptr.hpp>
#include <libpmemobj++/persistent_ptr_base.hpp>
#include <libpmemobj.h>
#include <memory>
#include "logistics.hpp"


static std::unordered_map <__UINT64_TYPE__, pmem::obj::persistent_ptr_base> alloc_table_nvram;
static std::unordered_map <__UINT64_TYPE__, std::unique_ptr> alloc_table_dram;

void init_hashmap(){
    alloc_table_nvram = new std::unordered_map<__UINT64_TYPE__, pmem::obj::persistent_ptr_base>();
    alloc_table_dram = new std::unordered_map<__UINT64_TYPE__, std::unique_ptr>();
}

template <typename T>
void add_new_object(__UINT64_TYPE__ key, __UINT16_TYPE__ type_of_allocation){
    const nvmalloc<T> nvm_obj = *((nvmalloc<T>*)key);

    alloc_table[key] = type_of_allocation - NVRAM_HEAP ? create_dram_obj(nvm_obj.get_size()) : create_nvram_obj(nvm_obj.get_size());
}

template <typename T>
void add_new_object(__UINT64_TYPE__ key, __UINT16_TYPE__ type_of_allocation, const T &data) {

    alloc_table[key] = type_of_allocation - NVRAM_HEAP ? create_dram_obj(data) : create_nvram_obj(data);

    store_data(key, data);
}

// how to get the type
template <typename T>
static std::unique_ptr create_dram_obj(const T &data) {
    std::unique_ptr<T> new_ptr((T*)malloc(data));

    return new_ptr;
}


template <typename T>
static pmem::obj::persistent_ptr_base create_nvram_obj(const T &_data) {

    if (std::is_class(T)) {
        struct root {
          pmem::obj::persistent_ptr<T> data;
        };

        pmem::obj::persistent_ptr<root> rootp = pmemobj_root(pop, sizeof(root));
    } else {
        struct actual_datatype {
          pmem::obj::p<T> var; 
        };

        struct root {
          pmem::obj::persistent_ptr<T> data;
        };

        pmem::obj::persistent_ptr<root> rootp = pmemobj_root(pop, sizeof(root));
    }
    

    return rootp;
}

template <typename T>
int store_data(__UINT64_TYPE__ key, const T &data){
    if (alloc_table.find(key) == alloc_table.end())
        return -1;
    else {
        nvmalloc<T>* ptr = (nvmalloc<T>*)key;
        *(ptr->data) = data;
    }
    return 0;
}

void* dram_pointer_return(__UINT64_TYPE__ key){

}

void* nvram_pointer_return(__UINT64_TYPE__ key){

}