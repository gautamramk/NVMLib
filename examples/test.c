#include <libNVMlib.h>


LIB_TOID_DECLARE(int);

int main() {
    printf("Hai\n\n\\n\n");
    initialize();

    LIB_TOID(int) obj = (LIB_TOID(int))memalloc(sizeof(LIB_TOID(int)), DRAM_HEAP);

    return 0;
}
