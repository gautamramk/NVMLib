#ifndef __NVM_MALLOC__
#define __NVM_MALLOC__

/*
* This file contains the definitions of all the functions related to 
* `Heap allocation` of variables.
*
* Idea :
*                    (if to be sent to NVRAM)
*                    ........................  libpmem.io
*                   /
* nvmmalloc() ------
*                   \........................  malloc()
*                    (if to be sent to DRAM)
*                     
*/

#include "globals.hpp"

template <typename T>
class nvmalloc {

    private:
    __UINT16_TYPE__ type_of_allocation;
    __SIZE_TYPE__ size;
    __UINT64_TYPE__ key;

    nvmalloc(__UINT16_TYPE__ _type_of_allocation, __SIZE_TYPE__ _size, __UINT64_TYPE__ _key);

    ~nvmalloc();

    public:

    // Getters and Setters
    __UINT16_TYPE__ get_type_of_allocation();

    __UINT64_TYPE__ get_size();

    __UINT64_TYPE__ get_key();

    void set_type_of_allcoation(__UINT16_TYPE__ type_of_allocation_);

    void set_size(__UINT64_TYPE__ size_);

    void set_key(__UINT64_TYPE__ key_);

    // The top level function for allocation of space in heap.
    friend nvmalloc nvmmalloc(__SIZE_TYPE__ _size);

    friend nvmalloc nvmmalloc(__SIZE_TYPE__ _size, T data);

    // The object destroyer
    friend void destroy(nvmalloc* obj);

};

#endif // !__NVM_MALLOC__