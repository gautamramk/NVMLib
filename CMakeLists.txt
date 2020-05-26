cmake_minimum_required(VERSION 3.10)

#############################################
# set the project name
#############################################
project(NVMlib LANGUAGES C CXX VERSION 0.1.0)

option(BUILD_DEBUG "Enable debug mode" OFF)

#############################################
# find the required libraries
#############################################

#-------------------------------------------#
# PTHREADS
#-------------------------------------------#
message(STATUS "Searching for PTHREAD library.")
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
if("$CMAKE_USE_PTHREADS_INIT" STREQUAL "")
    message(FATAL_ERROR "ERROR: No pthread library found.")
endif()
message(STATUS "PTHREAD library found successfully.")

#-------------------------------------------#
# LIBPMEM (shared version)
#-------------------------------------------#
message(STATUS "Searching for Intel PMDK-libpmem library.")
find_library(libpmem_LIBRARY NAMES libpmem.so PATHS /usr/lib /usr/lib/x86_64-linux-gnu)
if(NOT libpmem_LIBRARY)
    message(FATAL_ERROR "ERROR: PMDK-libpmem not found. Install libpmem-dev")
endif()
message(STATUS "PMDK-libpmem found successfully.")

#-------------------------------------------#
# LIBPMEMOBJ (shared version)
#-------------------------------------------#
message(STATUS "Searching for Inter PMDK-libpmemobj library.")
find_library(libpmemobj_LIBRARY NAMES libpmemobj.so PATHS /usr/lib /usr/lib/x86_64-linux-gnu)
if(NOT libpmemobj_LIBRARY)
    message(FATAL_ERROR "ERROR: PMDK-libpmemobj not found. Install libpmemobj-dev.")
endif()
message(STATUS "PMDK-libpmemobj found successfully.")

#-------------------------------------------#
# LIBDL (shared version)
#-------------------------------------------#
message(STATUS "Searching for LibDL library.")
find_library(libdl_LIBRARY NAMES libdl.so PATHS /usr/lib /usr/lib/x86_64-linux-gnu)
if(NOT libdl_LIBRARY)
    message(FATAL_ERROR "ERROR: LibDL not found. Install libdl.")
endif()
message(STATUS "LibDL found successfully.")

#-------------------------------------------#
# LIBC (shared version)
#-------------------------------------------#
message(STATUS "Searching for LibC library.")
find_library(libc_LIBRARY NAMES libc.so PATHS /usr/lib /usr/lib/x86_64-linux-gnu)
if(NOT libc_LIBRARY)
    message(FATAL_ERROR "ERROR: LibC not found. Install libdc.")
endif()
message(STATUS "LibC found successfully.")

#-------------------------------------------#
# LIBMATH (shared version)
#-------------------------------------------#
message(STATUS "Searching for Lib-math library.")
find_library(libmath_LIBRARY NAMES libm.so PATHS /usr/lib /usr/lib/x86_64-linux-gnu)
if(NOT libmath_LIBRARY)
    message(FATAL_ERROR "ERROR: PMDK-libpmemobj not found. Install libpmath.")
endif()
message(STATUS "Lib-math found successfully.")


#-------------------------------------------#
# LIBIBERTY (shared version)
#-------------------------------------------#
message(STATUS "Searching for LibIberty library package.")

# since static version
set(LibIberty_SEARCHORDER libiberty.so libiberty.a)
find_library(LibIberty_LIBRARIES NAMES ${LibIberty_SEARCHORDER})
find_path(LibIberty_INCLUDE_DIRS libiberty.h
	PATHS
		/usr/include /usr/include/libiberty
		/usr/local/include /usr/local/include/libiberty
	DOC "libiberty include directory containing libiberty.h")

if(LibIberty_INCLUDE_DIRS AND LibIberty_LIBRARIES)
	set(LibIberty_FOUND TRUE)
endif()

if(NOT LibIberty_FOUND)
	message(FATAL_ERROR "ERROR: LibIberty package not found. Install binutils-dev or libiberty-dev.")
endif()
message(STATUS "LibIberty package found successfully.")


#-------------------------------------------#
# GCC-PLUGIN
#-------------------------------------------#
message(STATUS "Searching for Gcc-Plugin library package.")

set(GccPlugin_SEARCHFILES gcc-plugin.h gimple.h basic-block.h)
find_path(GccPlugin_INCLUDE_DIRS
	NAMES ${GccPlugin_SEARCHFILES}
	PATHS
		/usr/lib/gcc/x86_64-linux-gnu/9/plugin/include   
		/usr/lib/gcc/x86_64-linux-gnu/8/plugin/include   
		/usr/lib/gcc/x86_64-linux-gnu/7/plugin/include   
		/usr/lib/gcc/x86_64-linux-gnu/6/plugin/include
		/usr/lib/gcc/x86_64-linux-gnu/5/plugin/include
		/usr/lib/gcc/x86_64-linux-gnu/4/plugin/include
	DOC "Gcc-Plugin include directory containing gcc-plugin.h.")

if(GccPlugin_INCLUDE_DIRS)
	set(GccPlugin_FOUND TRUE)
endif()


if(NOT GccPlugin_FOUND)
	message(FATAL_ERROR "ERROR: Gcc-Plugin package not found. Install gcc-<version>-plugin-dev.")
endif()
message(STATUS "Gcc-Plugin package found successfully.")

#-------------------------------------------#
# LIBUV (shared version)
#-------------------------------------------#
message(STATUS "Searching for LibUV library.")
find_library(libuv_LIBRARY NAMES libuv.so PATHS /usr/lib /usr/lib/x86_64-linux-gnu)
if(NOT libuv_LIBRARY)
    message(FATAL_ERROR "ERROR: LibUV library not found. Install libuv.")
endif()
message(STATUS "LibUV library found successfully.")

#############################################
# Enabling DEBUG mode
#############################################
if (BUILD_DEBUG)
  add_definitions(-DDEBUG)
endif(BUILD_DEBUG)

#############################################
# Include the directory containing the
# files for the library
#############################################
add_subdirectory(src_c_new)

#############################################
# Include the directory containing the
# files for the examples
#############################################
add_subdirectory(examples)

#############################################
# Code Experimentation
#############################################

#add_executable(basic dummy.c )
#target_compile_options(basic PRIVATE -save-temps)
#add_custom_command(TARGET basic POST_BUILD
#	COMMAND bash -c 'if [ ! -d intermediate ]; then mkdir intermediate\; fi'
#	COMMAND bash -c 'mv ./*.i intermediate/'
#	COMMAND bash -c 'rm -f ./*.s'
#	WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
#	COMMENT "Arranging the intermediate files"
#)
