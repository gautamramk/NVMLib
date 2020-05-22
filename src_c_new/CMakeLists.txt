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
add_library( NVMlib STATIC
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
add_custom_command(TARGET NVMlib POST_BUILD
	COMMAND bash -c 'if [ ! -d intermediate ]; then mkdir intermediate\; fi'
	COMMAND bash -c 'mv ./*.i intermediate/'
	COMMAND bash -c 'rm -f ./*.s'
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/src_c_new/
	COMMENT "Arranging the intermediate files"
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
# LIBIBERTY
#-----------------------------------------
target_include_directories(NVMlib PUBLIC ${LibIberty_INCLUDE_DIRS})

#----------------------------------------#
# GCC-PLUGIN
#----------------------------------------#
target_include_directories(mem_track_plugin PUBLIC ${GccPlugin_INCLUDE_DIRS})