#include <libNVMlib.h>
#include <unistd.h>

LIB_TOID_DECLARE(int);

int main() {
    initialize();

    // LIB_TOID(int) obj = (LIB_TOID(int))memalloc(sizeof(int), NVRAM_HEAP);

    // // LIB_TX_BEGIN;
    // // *LIB_D_RW(obj) = 2;
    // // LIB_TX_END;

    // // while (true) {
    // //     LIB_TX_BEGIN;
    // //     printf("printing value %d.\n", *LIB_D_RO(obj));
    // //     LIB_TX_END;
       
    // //     LIB_TX_BEGIN;

    // //     LIB_TX_END;

    
    // //     sleep(1);

    // // }

    // // LIB_TOID(int) obj1 = (LIB_TOID(int))memalloc(2 * sizeof(int), NVRAM_HEAP);
    // // LIB_TOID(int) obj2 = (LIB_TOID(int))memalloc(3 * sizeof(int), NVRAM_HEAP);

    // // LIB_TOID(int) obj3;

    // //memfree(obj1);
    // *LIB_D_RW(obj) = 1;
    // while (true) {

    //     // obj = (LIB_TOID(int))memalloc(sizeof(int), NVRAM_HEAP);
    //     (*LIB_D_RW(obj))++;
    //     printf("\n=================\nvalue = %d\n==========================\n", *LIB_D_RO(obj));
    //     // memfree(obj3);
        
        
    //     sleep(2);
    // }
    // memfree(obj);
    // // printf("The stored value: %d\n", *LIB_D_RO(obj));
    // // sleep(5); // for cleanup



    /**
     * CODE THAT DEMONSTRATES CRASH CONSISTENCY
     */ 

    int size = 10;
    LIB_TOID(int) arr = (LIB_TOID(int))memalloc(size * sizeof(int), NVRAM_HEAP);
    LIB_TOID(int) flag = (LIB_TOID(int))memalloc(sizeof(int), NVRAM_HEAP);

    LIB_TX_BEGIN;
    if(*LIB_D_RO(flag) == 1) {
        LIB_TX_END;
        goto skip_init;
    }
    LIB_TX_END;

    for(int i = 0; i< size; i++) {
        LIB_TX_BEGIN;
        LIB_D_RW(arr)[i] = size-i;
        LIB_TX_END;
    }

    LIB_TX_BEGIN;
    *LIB_D_RW(flag) = 1;
    LIB_TX_END;

skip_init:
    printf("values in array: ");
    for(int i = 0; i < size; i++) {
        LIB_TX_BEGIN;
        printf(" %d,", LIB_D_RO(arr)[i]);
        LIB_TX_END;
    }
    printf("\n");

    LIB_TOID(int) idx = (LIB_TOID(int))memalloc(sizeof(int), NVRAM_HEAP);

    LIB_TX_BEGIN;
    int k =  *LIB_D_RO(idx) < 10 ? *LIB_D_RO(idx):0;
    LIB_TX_END;
    for ( ; k < size ; k++) {
        LIB_TX_BEGIN;
        LIB_D_RW(arr)[k] = -1;
        LIB_TX_END;

        LIB_TX_BEGIN;
        *LIB_D_RW(idx) = k;
        LIB_TX_END;

        sleep(1);
    }

    memfree(arr);
    memfree(flag);
    memfree(idx);
    sleep(5);
    return 0;
}
