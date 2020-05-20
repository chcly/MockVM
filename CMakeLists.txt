cmake_minimum_required(VERSION 3.6)
project(ToyVM)

subdirs(Source)

set(BUILD_TEST CACHE BOOL False)

if (BUILD_TEST)
	subdirs(Test)
endif()