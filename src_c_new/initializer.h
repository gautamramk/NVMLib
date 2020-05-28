#ifndef __NVM_INITIALIZER__
#define __NVM_INITIALIZER__

#include "globals.h"

/**
 * The function to initialise all the internal datastructures. 
 * This is the function, the user has to call before any of his/her code.
 * 
 * @attention This must be the first line in the `main` function of the user program.
 */
void initialize();


#endif // !__NVM_INITIALIZER__
