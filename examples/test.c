#include <libNVMlib.h>


LIB_TOID_DECLARE(int);

int main() {
    printf("Hai\n\n\\n\n");
    initialize();

    LIB_TOID(int) obj = (LIB_TOID(int))memalloc(sizeof(int));
    *LIB_D_RW(obj) = 2;

    printf("The stored value: %d\n", *LIB_D_RO(obj));

    memfree(obj);

    return 0;
}
