##########################################
# Collecting all the source files
#
# Note: Every time a new file is added or removed from the
#       DIR, `cmake` must be run again, not just `make`.
##########################################
file( GLOB LIB_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.c )
file( GLOB LIB_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h )
file( GLOB PLUGIN_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cc )

##########################################
# The Library creation
##########################################

#----------------------------------------#
# LIBNVMLIB (static)
#----------------------------------------#
add_library( NVMlib SHARED
    ${LIB_SOURCES} 
    ${LIB_HEADERS} 
)

#----------------------------------------#
# PLUGIN (shared)
#----------------------------------------#
add_library( mem_track_plugin SHARED
	${PLUGIN_SOURCES}
)

##########################################
# Setting C11 and C++17 as the standard
##########################################
set_property(TARGET NVMlib PROPERTY C_STANDARD 11)
set_property(TARGET mem_track_plugin PROPERTY CXX_STANDARD 17)


##########################################
# Making it save the .i files too and 
# to give all warnings
##########################################

#----------------------------------------#
# LIBNVMLIB
#----------------------------------------#
target_compile_options(NVMlib PRIVATE -save-temps)
target_compile_options(NVMlib PRIVATE -Wall)
target_compile_options(NVMlib PRIVATE -g3)
target_compile_options(NVMlib PRIVATE -static-libasan)
target_compile_options(NVMlib PRIVATE -fsanitize=address)
add_custom_command(TARGET NVMlib POST_BUILD
	COMMAND bash -c 'if [ ! -d intermediate ]; then mkdir intermediate\; fi'
	COMMAND bash -c 'mv ./*.i intermediate/'
	COMMAND bash -c 'rm -f ./*.s'
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}intermediate/*.i
	COMMENT "Arranging the intermediate files"
)
add_custom_command(TARGET NVMlib POST_BUILD
	COMMAND bash -c 'if [ ! -d include ]; then mkdir include\; fi'
	COMMAND bash -c 'cd ${CMAKE_CURRENT_SOURCE_DIR}\; for name in *.h\; do newname=\"$$\(echo \"$$name\"\)\"\; cp \"$$name\" \"${CMAKE_CURRENT_BINARY_DIR}/include/$$newname\"\; done'
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}include/*.h
	COMMENT "Making the library header files"
)

#----------------------------------------#
# PLUGIN
#----------------------------------------#
target_compile_options(mem_track_plugin PRIVATE -fPIC)
target_compile_options(mem_track_plugin PRIVATE -fno-rtti)


##########################################
# Linking libraries
##########################################

#-----------------------------------------
# PTHREAD
#-----------------------------------------
target_link_libraries(NVMlib PUBLIC Threads::Threads)

#-----------------------------------------
# LIBPMEM
#-----------------------------------------
target_link_libraries(NVMlib PUBLIC ${libpmem_LIBRARY})

#-----------------------------------------
# LIBPMEMOBJ
#-----------------------------------------
target_link_libraries(NVMlib PUBLIC ${libpmemobj_LIBRARY})

#-----------------------------------------
# LIBUV
#-----------------------------------------
target_link_libraries(NVMlib PUBLIC ${libuv_LIBRARY})

#-----------------------------------------
# LIBDL
#-----------------------------------------
target_link_libraries(NVMlib PUBLIC ${libdl_LIBRARY})

#-----------------------------------------
# LIBC
#-----------------------------------------
target_link_libraries(NVMlib PUBLIC ${libc_LIBRARY})

#-----------------------------------------
# LIBMATH
#-----------------------------------------
target_link_libraries(NVMlib PUBLIC ${libmath_LIBRARY})

#-----------------------------------------
# LIBIBERTY
#-----------------------------------------
target_include_directories(NVMlib PUBLIC ${LibIberty_INCLUDE_DIRS})
target_link_libraries(NVMlib PUBLIC ${LibIberty_LIBRARIES})

#----------------------------------------#
# GCC-PLUGIN
#----------------------------------------#
target_include_directories(mem_track_plugin PUBLIC ${GccPlugin_INCLUDE_DIRS})
