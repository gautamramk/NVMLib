#include <libNVMlib.h>
#include <unistd.h>

LIB_TOID_DECLARE(int);

int main() {
    initialize();

    LIB_TOID(int) obj = (LIB_TOID(int))memalloc(sizeof(int), NVRAM_HEAP);
    /*if (*LIB_D_RO(obj) == 2) {
        goto l2;
    } else {
        *LIB_D_RW(obj) = 2;
        while (true) {
            printf("I am still here.\n");
            sleep(2);
        }
    }*/
    *LIB_D_RW(obj) = 2;
    while (true) {
        printf("printing value %d.\n", *LIB_D_RO(obj));
        (*LIB_D_RW(obj))++;
        LIB_TOID(int) obj = (LIB_TOID(int))memalloc(sizeof(int), NVRAM_HEAP);
        sleep(1);
    }
l2:
    printf("The stored value: %d\n", *LIB_D_RO(obj));

    memfree(obj);
    sleep(5);
    return 0;
}
