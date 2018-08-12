cmake_minimum_required(VERSION 3.9)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
# remove Release
set(CMAKE_CONFIGURATION_TYPES Debug CACHE TYPE INTERNAL FORCE)
message("Build type: ${CMAKE_BUILD_TYPE}")
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type was overridden to Debug" FORCE)
endif(NOT CMAKE_BUILD_TYPE)

project(FlatBuffersFuzzerTests)

set(CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -pedantic -Werror -Wextra -Wno-unused-parameter -fsigned-char")

set(CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} -g -fsigned-char -fno-omit-frame-pointer")

set(CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} -fsanitize=fuzzer,address,undefined,alignment,signed-integer-overflow")

set(CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} -fsanitize-coverage=edge,trace-cmp")

# enable link-time optimisation
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto")

# https://llvm.org/docs/Passes.html
# save IR to see call graph
# make one bitcode file:> llvm-link *.bc -o out.bc
# print call-graph:> opt out.bc -analyze -print-callgraph &> callgraph.txt
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -save-temps -flto")

# use llvm-linker lld
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} -fuse-ld=lld")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld")

set(FLATBUFFERS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../")

set(FlatBuffers_Library_SRCS
  ${FLATBUFFERS_DIR}/include/flatbuffers/code_generators.h
  ${FLATBUFFERS_DIR}/include/flatbuffers/base.h
  ${FLATBUFFERS_DIR}/include/flatbuffers/flatbuffers.h
  ${FLATBUFFERS_DIR}/include/flatbuffers/hash.h
  ${FLATBUFFERS_DIR}/include/flatbuffers/idl.h
  ${FLATBUFFERS_DIR}/include/flatbuffers/util.h
  ${FLATBUFFERS_DIR}/include/flatbuffers/reflection.h
  ${FLATBUFFERS_DIR}/include/flatbuffers/reflection_generated.h
  ${FLATBUFFERS_DIR}/include/flatbuffers/stl_emulation.h
  ${FLATBUFFERS_DIR}/include/flatbuffers/flexbuffers.h
  ${FLATBUFFERS_DIR}/include/flatbuffers/registry.h
  ${FLATBUFFERS_DIR}/include/flatbuffers/minireflect.h
  ${FLATBUFFERS_DIR}/src/code_generators.cpp
  ${FLATBUFFERS_DIR}/src/idl_parser.cpp
  ${FLATBUFFERS_DIR}/src/idl_gen_text.cpp
  ${FLATBUFFERS_DIR}/src/reflection.cpp
  ${FLATBUFFERS_DIR}/src/util.cpp
)

include_directories(${FLATBUFFERS_DIR}/include)
add_library(flatbuffers STATIC ${FlatBuffers_Library_SRCS})

# force to check the RecursionError in the test
target_compile_definitions(flatbuffers PRIVATE
  FLATBUFFERS_MAX_PARSING_DEPTH=8)

# enable locale independend code and check it
target_compile_definitions(flatbuffers PUBLIC
  FLATBUFFERS_FORCE_LOCALE_INDEPENDENT="ru_RU.CP1251")


add_executable(scalar_fuzzer flatbuffers_scalar_fuzzer.cc)
target_link_libraries(scalar_fuzzer PRIVATE flatbuffers)

add_executable(parser_fuzzer flatbuffers_parser_fuzzer.cc)
target_link_libraries(parser_fuzzer PRIVATE flatbuffers)

add_executable(verifier_fuzzer flatbuffers_verifier_fuzzer.cc)
target_link_libraries(verifier_fuzzer PRIVATE flatbuffers)
target_include_directories(verifier_fuzzer PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../")
