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
   subdirs(Test)
endif()