add_executable(test_prog test.c)

target_include_directories(test_prog PRIVATE ${PROJECT_BINARY_DIR}/src_c_new/include/)
#target_compile_options(test_prog PRIVATE -static-libasan)
#set_target_properties(test_prog PROPERTIES LINK_FLAGS "-static-libasan")

#target_compile_options(test_prog PRIVATE -fsanitize=address)
#set_target_properties(test_prog PROPERTIES LINK_FLAGS "-fsanitize=address")

target_link_libraries(test_prog PRIVATE NVMlib)
target_compile_options(test_prog PRIVATE -save-temps)
target_compile_options(test_prog PRIVATE -g3)
target_compile_options(test_prog PRIVATE -fplugin=${PROJECT_BINARY_DIR}/src_c_new/libmem_track_plugin.so)

#target_compile_options(test_prog PRIVATE -ldl)
