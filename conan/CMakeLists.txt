cmake_minimum_required(VERSION 2.8)

message("Conan FlatBuffers Wrapper")

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

include(${CMAKE_SOURCE_DIR}/CMakeListsOriginal.txt)
