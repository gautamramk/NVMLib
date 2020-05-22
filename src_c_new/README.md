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
    - There seem to be a lot of compiler errors. Correct it.    --- DONE
    - Complete the functions `get_first_free_offset` and `allot_current_free_offset`
    - Write the creation of new pools function.
    - Complete the logic to create new `pool` if the current one is insufficient.

* See if `pmem_persist` needs to be called. 

* Make the `initializer.c` to function.     --- SEEMS DONE

* Write the CMake files.    --- DONE

* Make sure where ever there is `pointer` or `struct` as return type or argument type, they are passed correctly, i.e check `by-value` and `by-memory`. 

* Make sure to move all the `storage variables` into `.c` files or atleast make sure there are no collisions, i.e same global variable should not be initialised at two `.c` files.
