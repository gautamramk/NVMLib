#include "types.h"

// The number of types that have used libnvm
static uint64_t type_num = 0;

uint64_t get_current_type_num() {
    return type_num;
}

uint64_t get_next_type_num() {
    type_num++;
    return type_num;
}