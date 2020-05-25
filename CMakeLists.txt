# -----------------------------------------------------------------------------
#   Copyright (c) 2020 Charles Carley.
#
#   This software is provided 'as-is', without any express or implied
# warranty. In no event will the authors be held liable for any damages
# arising from the use of this software.
#
#   Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.
# ------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.0)
project(ToyVM)

set(CMAKE_MODULE_PATH 
    ${ToyVM_SOURCE_DIR}/CMake
)

set(ToyVM_INSTALL_PATH CACHE STRING "")
set(ToyVM_TEST         CACHE BOOL   OFF)
set(BUILD_TEST         CACHE BOOL   OFF)

if (BUILD_TEST)
    set(ToyVM_TEST CACHE FORCE BOOL ON)
else ()
    if (ToyVM_TEST)
        set(ToyVM_TEST CACHE FORCE BOOL OFF)
    endif()
endif()


subdirs(CMake)
include (StaticRuntime)
include (CopyTarget)
set_static_runtime()

set(ToyVM_BIN_DIR ${ToyVM_BINARY_DIR}/bin)
set(ToyVM_LIB_DIR ${ToyVM_BIN_DIR}/lib)


subdirs(Source)

if (ToyVM_TEST)
    set(TestDirectory ${ToyVM_SOURCE_DIR}/Test)


    configure_file(${ToyVM_SOURCE_DIR}/CMake/TestConfig.h.in  
                   ${ToyVM_BINARY_DIR}/TestConfig.h)

    subdirs(Test)
endif()