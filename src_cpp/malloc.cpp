#include "malloc.hpp"
#include "algo.hpp"
#include "logistics.hpp"

template <typename T>
nvmalloc<T>::nvmalloc(__UINT16_TYPE__ _type_of_allocation, __SIZE_TYPE__ _size, __UINT64_TYPE__ _key){
    type_of_allocation = _type_of_allocation;
    size = _size;
    key = _key;
}

template <typename T>
nvmalloc<T>::nvmalloc(){
    // empty constructor
    // just in case :)
}

template <typename T>
__UINT16_TYPE__ nvmalloc<T>::get_type_of_allocation(){
    return this->type_of_allocation;
}

template <typename T>
__UINT64_TYPE__ nvmalloc<T>::get_size(){
    return this->size;
}

template <typename T>
__UINT64_TYPE__ nvmalloc<T>::get_key(){
    return this->key;
}

template <typename T>
void nvmalloc<T>::set_type_of_allocation(__UINT16_TYPE__ type_of_allocation_){
    this->type_of_allocation = type_of_allocation_;
}

template <typename T>
void nvmalloc<T>::set_size(__UINT64_TYPE__ size_){
    this->size = size_;
}

template <typename T>
void nvmalloc<T>::set_key(__UINT64_TYPE__ key_){
    this->key = key_;
}

// Funtion to access the data
template <typename T>
T nvmalloc<T>::access(){
    __UINT16_TYPE__ type = type_of_allocation;
    return type - NVRAM_HEAP ? access_from_dram(key) : access_from_nvmram(key);
}

// The top level function for allocation of space in heap.
nvmalloc<void> nvmmalloc(__SIZE_TYPE__ _size){
    nvmalloc<void> new_obj;
    new_obj.set_size(_size);
    new_obj.set_key(reinterpret_cast<uintptr_t>(&new_obj));
    return new_obj;
}

template <typename T>
nvmalloc<T> nvmmalloc(const T &data){
    return nvmmalloc(sizeof(data), data);
}


template <typename T>
nvmalloc<T> nvmmalloc(__SIZE_TYPE__ _size, const T &data){
    nvmalloc<T> new_obj;
    new_obj.set_size(_size);
    new_obj.set_key(reinterpret_cast<uintptr_t>(&new_obj));

    new_obj.set_type_of_allocation(decide_allocation());

    add_new_obj(new_obj.get_key());
    store_data(new_obj.get_key(), data);

    return new_obj;
}

// The object destroyer
template <typename T>
void destroy(nvmalloc<T>* obj){

}

template <typename T>
nvmalloc<T> nvmalloc<T>::operator + (const nvmalloc<T> &obj_1){
    return nvmalloc(access() + obj_1.access());
}


template <typename T>
nvmalloc<T> nvmalloc<T>::operator - (const nvmalloc<T> &obj_1){
    return nvmalloc(access() - obj_1.access());
}



template <typename T>
nvmalloc<T> nvmalloc<T>::operator * (const nvmalloc<T> &obj_1){
    return nvmalloc(access() * obj_1.access());
}



template <typename T>
nvmalloc<T> nvmalloc<T>::operator / (const nvmalloc<T> &obj_1){
    return nvmalloc(access() / obj_1.access());
}


template <typename T>
void nvmalloc<T>::operator = (const nvmalloc<T> &obj_1){
    type_of_allocation = obj_1.type_of_allocation;
    key = obj_1.key;
    size = obj_1.size;
}


template <typename T>
nvmalloc<T> nvmalloc<T>::operator [] (int idx){
    return nvmalloc(access()[idx]);
}



template <typename T>
static T access_from_dram(__UINT64_TYPE__ key) {
    return *((T*)dram_pointer_return(key)->data);
}

template <typename T>
static T access_from_nvram(__UINT64_TYPE__ key){

}