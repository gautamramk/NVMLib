#ifndef __NVM_ALGO__
#define __NVM_ALGO__

#include "global.h"
#include "malloc.h"

void update_read_access(MEMoidKey key);

void update_write_access(MEMoidKey key);

void update_time_of_access(MEMoidKey key);

void update_access_number(MEMoidKey key);

int decide_allocation(size_t size);

#endif // !__NVM__ALGO__