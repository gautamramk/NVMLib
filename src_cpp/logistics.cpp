#include <unordered_map>
#include <libpmemobj.h>

template <typename T>
static std::unordered_set <__UINT64_TYPE__> alloc_table;

void init_hashmap(){
    alloc_table = new std::unordered_set<struct nvmalloc<T>*>();
}

void add_new_object(__UINT64_TYPE__ key){
    alloc_table.insert(key);
}

template <typename T>
int store_data(__UINT64_TYPE__ key, const T &data){
    if (alloc_table.find(key) == alloc_table.end())
        return -1;
    else {
        nvmalloc<T>* ptr = reinterpret_cast<nvmalloc<T>*>(key);
        *(ptr->data) = data;
    }
    return 0;
}

void* dram_pointer_return(__UINT64_TYPE__ key){

}

void* nvram_pointer_return(__UINT64_TYPE__ key){

}