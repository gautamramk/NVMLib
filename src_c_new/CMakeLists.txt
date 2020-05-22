##########################################
# Collecting all the source files
#
# Note: Every time a new file is added or removed from the
#       DIR, `cmake` must be run again, not just `make`.
##########################################
file( GLOB LIB_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.c )
file( GLOB LIB_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h )


##########################################
# The Library creation
##########################################
add_library( NVMlib STATIC
    ${LIB_SOURCES} 
    ${LIB_HEADERS} 
)

##########################################
# Setting C11 as the standard
##########################################
set_property(TARGET NVMlib PROPERTY C_STANDARD 11)


##########################################
# Making it save the .i files too and 
# to give all warnings
##########################################
target_compile_options(NVMlib PRIVATE -save-temps)
target_compile_options(NVMlib PRIVATE -Wall)
add_custom_command(TARGET NVMlib POST_BUILD
	COMMAND bash -c 'if [ ! -d intermediate ]; then mkdir intermediate\; fi'
	COMMAND bash -c 'mv ./*.i intermediate/'
	COMMAND bash -c 'rm -f ./*.s'
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/src_c_new/
	COMMENT "Arranging the intermediate files"
)

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

