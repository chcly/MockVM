cmake_minimum_required(VERSION 3.6)
project(ToyVM)

set(CMAKE_MODULE_PATH 
    ${ToyVM_SOURCE_DIR}/CMake
)
subdirs(CMake)
include (StaticRuntime)
set_static_runtime()
disable_crt_warnings()

subdirs(Source)
set(BUILD_TEST CACHE BOOL False)

if (BUILD_TEST)

    set(TestDirectory ${ToyVM_SOURCE_DIR}/Test)
    configure_file(${ToyVM_SOURCE_DIR}/CMake/TestConfig.h.in  ${ToyVM_BINARY_DIR}/TestConfig.h)
    subdirs(Test)
endif()