@mainpage
# NVMLib

`NVMLib` is library to optimally use a Hybrid RAM setup. It is designed to resemble `malloc` in its features and interfaces. It is designed to have little to no input from
the programmer regard the decision of where a perticular object needs to be placed.

## What the library is trying to achieve

1. **Crash consistency**: Basic crash consistency is implemented. Context-awareness is the next thing to be added
2. **Movement of objects from DRAM to NVRAM and vice versa** inorder to improve the performance and/or the energy consumption: The ability to move has been completed. The algorithm to decide movement is yet to be figured out.

## Unique things in the library

1. Crash consistency out of the box
2. Ability to move the object from NVRAM to DRAM and vice-versa at runtime
3. Provides the exact same interface as malloc and is just as feature-rich
4. Provides the user the ability to define where a particular object needs to go (DRAM or NVRAM) **if he chooses to** (it's not necessary)
5. The library overhead is just 40MB irrespective of the size of the program/its allocations
6. The performance is comparable to `malloc` from the 2nd run 
7. The library demands no intervention or hints from the programmer. Any hints provided are considered with priority though.
8. We use hashtables (with Fibonacci and cuckoo hashing) and splay trees as internal data-structures to make the queries` O(1)`
9. We offload all the object maintenance tasks like moving objects, deletion, other maintenance-related calculations onto either the `logistics` thread or the `deletion` thread and hence do not add latency to the user program (running on the main thread). At the same time we ensure data integrity and also the returned "access-pointer's" validity.



## Compiling 

* Clone the repo
```shell
$ git clone https://github.com/gautamramk/NVMLib
```

* Make a new build directory
```shell
$ cd NVMLib
$ mkdir build
```

* `cd` into the build directory and execute `make`
```shell
$ cd build
$ export CC=<path to C compiler>
$ export CXX=<path to C++ compiler>
$ cmake ..
$ make
```
Here `export CC=<path to C compiler>` `export CXX=<path to C++ compiler>` are optional, but recomended because `cmake` defaults to `cc` as the compiler.

Here we do `cmake` before `make` inorder to rebuild the `Makefiles`. This should be used when a additional files are added to the source code.

* For verbose build use `make VERBOSE=1`

After `make` the static library file (`libNVMlib.a`) and the gcc-plugin (`libmem_track_plugin.so`) will be in `build/` and the intermediate files (`.i`) will be in `build/intermediate/`.

## Dependecy graph

![alt text](graphs/depen.png)