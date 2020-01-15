#include "malloc.h"

void *access_var_func (nvmalloc *ptr) {
    if (ptr == NULL) {
        return NULL;
    }
    if (ptr->type_of_allocation == NVRAM_HEAP) {
        
    }
    else if (ptr->type_of_allocation == DRAM_HEAP) {
    }
}
