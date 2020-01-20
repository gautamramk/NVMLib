#ifndef __NVM_MALLOC__
#define __NVM_MALLOC__

#include "malloc.hpp"

using namespace std;

template <typename T>
nvmalloc<T>::nvmalloc(__UINT16_TYPE__ _type_of_allocation, __SIZE_TYPE__ _size, __UINT64_TYPE__ _key){
    type_of_allocation = _type_of_allocation;
    size = _size;
    key = _key;
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
    return this->key
}

template <typename T>
void nvmalloc<T>::set_type_of_allcoation(__UINT16_TYPE__ type_of_allocation_){
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

template <typename T>
nvmalloc nvmmalloc(__SIZE_TYPE__ _size){
    
}

template <typename T>
nvmalloc nvmmalloc(__SIZE_TYPE__ _size, T data){

}




#endif // !__NVM_MALLOC__