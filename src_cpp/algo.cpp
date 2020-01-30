#include <algo.hpp>



int decide_allocation () {

}

template <typename T>
T& retrieve_value(nvmalloc <T> obj) {
    switch (obj.type_of_allocation()) {
        case NVRAM_HEAP:
            break;
        case NVRAM_STACK:
            break;
        case DRAM_HEAP:
            break;
        case DRAM_STACK:
            break;
    }
}