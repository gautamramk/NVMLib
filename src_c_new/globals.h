#ifndef __NVM_GLOBALS__
#define __NVM_GLOBALS__

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>


// Types of allocation
#define DRAM_STACK 0
#define NVRAM_STACK 1
#define DRAM_HEAP 2
#define NVRAM_HEAP 3


typedef __uint128_t uint128_t;
typedef __uint64_t uint64_t;
typedef __uint32_t uint32_t;
typedef __uint16_t uint16_t;
typedef __uint8_t uint8_t;

typedef unsigned long int uintptr_t;
typedef long int intptr_t;

#endif // !__NVM_GLOBALS__
