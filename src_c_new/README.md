## Libraries used

* pmem.io/libpmemobj
* glibc/libc-internal.h
* pmem.io/libpmem
* libiberty/splay-tree.h

## Things to be completed

* object_maintainanc.c
    - Implement the `uv_loops` properly with mutexes
    - Complete `check_if_required_to_move` and `check_if_required_to_delete`
    - Correct other functions to use `MEMoid size` properly. Note `TOID_SIZEOF` is not what we need.
    - Use logging when you move stuff.

*  algo.c
    - Implement the fucntions. Use the `Queue` in `mem_log.c`.
    - Implement the `decide_allocation` function

* malloc.c and malloc.h (maybe everywhere)
    - Correct the usage of `MEMoid`. Make sure they use `size` in `oid`.
