add_executable(test_prog test.c)

target_include_directories(test_prog PRIVATE ${PROJECT_BINARY_DIR}/src_c_new/include/)
target_link_libraries(test_prog PRIVATE NVMlib)
target_compile_options(test_prog PRIVATE -g)
target_compile_options(test_prog PRIVATE -save-temps)