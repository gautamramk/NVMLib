#include <libNVMlib.h>
#include <unistd.h>
#include <time.h>

LIB_TOID_DECLARE(int);

int main() {
    initialize();
    srand(time(NULL));
    clock_t end, start;
    start = clock();
    int size = 1000;
    LIB_TOID(int) arr = (LIB_TOID(int))memalloc(size * sizeof(int), NVRAM_HEAP);
    LIB_TOID(int) flag = (LIB_TOID(int))memalloc(sizeof(int), NVRAM_HEAP);
    LIB_TOID(int) idx = (LIB_TOID(int))memalloc(sizeof(int), NVRAM_HEAP);
    LIB_TOID(int) idy = (LIB_TOID(int))memalloc(sizeof(int), NVRAM_HEAP);

    int i =  *LIB_D_RO(idx) < size ? *LIB_D_RO(idx):0;
    if(*LIB_D_RO(flag) == 1) {
        goto skip_init;
    }

    for(int i = 0; i< size; i++) {
        LIB_D_RW(arr)[i] = rand()%(size * 2);
    }

    *LIB_D_RW(flag) = 1;

skip_init:
    //printf("Starting skip init.\n");

   // int i = *LIB_D_RO(idx);

    for (; i < size - 1; i++) {
        int j =  *LIB_D_RO(idy) < size - i - 1? *LIB_D_RO(idy):0;
        //int j = *LIB_D_RO(idy);
        for (; j < size - i - 1; j++) {
            //int temp1 = LIB_D_RO(arr)[j];
            //int temp2 = LIB_D_RO(arr)[j+1];
            
            //if (temp1 > temp2) {
            if (LIB_D_RO(arr)[j] > LIB_D_RO(arr)[j+1]) {
                int temp = LIB_D_RO(arr)[j+1];
                LIB_D_RW(arr)[j + 1] = LIB_D_RO(arr)[j];
                LIB_D_RW(arr)[j] = temp;
            }
            *LIB_D_RW(idy) = j;
        }
        *LIB_D_RW(idx) = i;
        // printf("Array at iteration %d is: ", i);
        // for (int k = 0; k < size; k++) {
        //     printf("%d, ", LIB_D_RO(arr)[k]);
        // }
        // printf("\n");
        // sleep(1);
    }
    end = clock();
    printf("Elapsed time = %f\n", (double)(end - start)/CLOCKS_PER_SEC);
    // memfree(arr);
    // memfree(flag);
    // memfree(idx);
    // memfree(idy);
    // sleep(5);    
}