## Libraries used

* pmem.io/libpmemobj
* glibc/libc-internal.h
* pmem.io/libpmem
* libiberty/splay-tree.h

## Things to be completed

* object_maintainanc.c
    - Implement the `uv_loops` properly with mutexes        --- DONE
    - Complete `check_if_required_to_move`
    - Complete `check_if_required_to_delete`    --- DONE
    - Correct other functions to use `MEMoid size` properly. Note `TOID_SIZEOF` is not what we need.    --- DONE
    - Use logging when you move stuff.      --- DONE

*  algo.c
    - Implement the fucntions. Use the `Queue` in `mem_log.c`.
    - Implement the `decide_allocation` function

* malloc.c and malloc.h (maybe everywhere)
    - Correct the usage of `MEMoid`. Make sure they use `size` in `oid`.    --- DONE

* Make sure the MEMoid compares are correct (structs can't be compared directly in C)   --- DONE

* types.c
    - Make sure `init_types_table` works

* pool.c
    - There seem to be a lot of compiler errors. Correct it.

* See if `pmem_persist` needs to be called. 

* Make the `initializer.c` to function.     --- SEEMS DONE

* Write the CMake files.    --- DONE