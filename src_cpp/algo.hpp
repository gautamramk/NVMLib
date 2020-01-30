#ifndef __ALGO__
#define __ALGO__

#include "globals.hpp"
#include "malloc.hpp"

int decide_allocation();

template <typename T>
T& retrieve_value(nvmalloc <T>);
#endif // !__ALGO__