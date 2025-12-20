cmake_minimum_required(VERSION 3.9)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

project(FlatBuffersFuzzerTests)

option(BUILD_DEBUGGER "Compile a debugger with main() and without libFuzzer" OFF)

if(NOT DEFINED FLATBUFFERS_MAX_PARSING_DEPTH)
  # Force checking of RecursionError in the test
  set(FLATBUFFERS_MAX_PARSING_DEPTH 24)
endif()
message(STATUS "FLATBUFFERS_MAX_PARSING_DEPTH: ${FLATBUFFERS_MAX_PARSING_DEPTH}")

# Usage '-fsanitize=address' doesn't allowed with '-fsanitize=memory'.
# MemorySanitizer will not work out-of-the-box, and will instead report false
# positives coming from uninstrumented code. Need to re-build both C++ standard
# library: https://github.com/google/sanitizers/wiki/MemorySanitizerLibcxxHowTo
option(USE_ASAN "Use fuzzers with ASASN" OFF)
option(USE_MSAN "Use fuzzers with MSASN" OFF)
option(OSS_FUZZ "Set this option to use flags by oss-fuzz" OFF)

# Use Clang linker.
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld")

# add_link_options(-stdlib=libc++)

add_compile_options(
  # -stdlib=libc++ # Use Clang libc++ instead of GNU.
  -std=c++17
  -Wall
  -pedantic
  -Werror
  -Wextra
  -Wno-unused-parameter
  -fsigned-char
  -fno-omit-frame-pointer
  -g # Generate source-level debug information
  # -flto # enable link-time optimisation
)

# https://llvm.org/docs/Passes.html save IR to see call graph make one bitcode
# file:> llvm-link *.bc -o out.bc print call-graph:> opt out.bc -analyze -print-
# callgraph &> callgraph.txt set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -save-temps
# -flto")

# A special target with fuzzer+sanitizer flags.
add_library(fuzzer_config INTERFACE)

target_compile_options(
  fuzzer_config
  INTERFACE
    $<$<NOT:$<BOOL:${OSS_FUZZ}>>:
      -fsanitize-coverage=trace-cmp
    >
    $<$<BOOL:${USE_ASAN}>:
      -fsanitize=fuzzer,undefined,address
    >
    $<$<BOOL:${USE_MSAN}>:
      -fsanitize=fuzzer,undefined,memory
      -fsanitize-memory-track-origins=2
    >
    $<$<BOOL:${OSS_FUZZ}>:
      ${CXX}
      ${CXXFLAGS}
    >
)

target_link_libraries(
  fuzzer_config
  INTERFACE
    $<$<BOOL:${USE_ASAN}>:
      -fsanitize=fuzzer,undefined,address
    >
    $<$<BOOL:${USE_MSAN}>:
      -fsanitize=fuzzer,undefined,memory
    >
    $<$<BOOL:${OSS_FUZZ}>:
      $ENV{LIB_FUZZING_ENGINE}
    >
)

set(FLATBUFFERS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../")

set(FlatBuffers_Library_SRCS
    ${FLATBUFFERS_DIR}/include/flatbuffers/allocator.h
    ${FLATBUFFERS_DIR}/include/flatbuffers/array.h
    ${FLATBUFFERS_DIR}/include/flatbuffers/base.h
    ${FLATBUFFERS_DIR}/include/flatbuffers/buffer.h
    ${FLATBUFFERS_DIR}/include/flatbuffers/buffer_ref.h
    ${FLATBUFFERS_DIR}/include/flatbuffers/default_allocator.h
    ${FLATBUFFERS_DIR}/include/flatbuffers/detached_buffer.h
    ${FLATBUFFERS_DIR}/include/flatbuffers/flatbuffer_builder.h
    ${FLATBUFFERS_DIR}/include/flatbuffers/flatbuffers.h
    ${FLATBUFFERS_DIR}/include/flatbuffers/flexbuffers.h
    ${FLATBUFFERS_DIR}/include/flatbuffers/flex_flat_util.h
    ${FLATBUFFERS_DIR}/include/flatbuffers/hash.h
    ${FLATBUFFERS_DIR}/include/flatbuffers/idl.h
    ${FLATBUFFERS_DIR}/include/flatbuffers/minireflect.h
    ${FLATBUFFERS_DIR}/include/flatbuffers/reflection.h
    ${FLATBUFFERS_DIR}/include/flatbuffers/reflection_generated.h
    ${FLATBUFFERS_DIR}/include/flatbuffers/registry.h
    ${FLATBUFFERS_DIR}/include/flatbuffers/stl_emulation.h
    ${FLATBUFFERS_DIR}/include/flatbuffers/string.h
    ${FLATBUFFERS_DIR}/include/flatbuffers/struct.h
    ${FLATBUFFERS_DIR}/include/flatbuffers/table.h
    ${FLATBUFFERS_DIR}/include/flatbuffers/util.h
    ${FLATBUFFERS_DIR}/include/flatbuffers/vector.h
    ${FLATBUFFERS_DIR}/include/flatbuffers/vector_downward.h
    ${FLATBUFFERS_DIR}/include/flatbuffers/verifier.h
    ${FLATBUFFERS_DIR}/src/file_manager.cpp
    ${FLATBUFFERS_DIR}/src/file_name_manager.cpp
    ${FLATBUFFERS_DIR}/src/idl_parser.cpp
    ${FLATBUFFERS_DIR}/src/idl_gen_text.cpp
    ${FLATBUFFERS_DIR}/src/reflection.cpp
    ${FLATBUFFERS_DIR}/src/binary_annotator.h
    ${FLATBUFFERS_DIR}/src/binary_annotator.cpp
    ${FLATBUFFERS_DIR}/src/util.cpp
    ${FLATBUFFERS_DIR}/tests/test_assert.cpp
    ${FLATBUFFERS_DIR}/tests/64bit/test_64bit_bfbs_generated.h
)

set(FlatBuffers_Compiler_SRCS
  ${FLATBUFFERS_DIR}/src/idl_gen_binary.cpp
  ${FLATBUFFERS_DIR}/src/idl_gen_text.cpp
  ${FLATBUFFERS_DIR}/src/idl_gen_cpp.cpp
  ${FLATBUFFERS_DIR}/src/idl_gen_csharp.cpp
  ${FLATBUFFERS_DIR}/src/idl_gen_dart.cpp
  ${FLATBUFFERS_DIR}/src/idl_gen_kotlin.cpp
  ${FLATBUFFERS_DIR}/src/idl_gen_kotlin_kmp.cpp
  ${FLATBUFFERS_DIR}/src/idl_gen_go.cpp
  ${FLATBUFFERS_DIR}/src/idl_gen_java.cpp
  ${FLATBUFFERS_DIR}/src/idl_gen_ts.cpp
  ${FLATBUFFERS_DIR}/src/idl_gen_php.cpp
  ${FLATBUFFERS_DIR}/src/idl_gen_python.cpp
  ${FLATBUFFERS_DIR}/src/idl_gen_lobster.cpp
  ${FLATBUFFERS_DIR}/src/idl_gen_rust.cpp
  ${FLATBUFFERS_DIR}/src/idl_gen_fbs.cpp
  ${FLATBUFFERS_DIR}/src/idl_gen_grpc.cpp
  ${FLATBUFFERS_DIR}/src/idl_gen_json_schema.cpp
  ${FLATBUFFERS_DIR}/src/idl_gen_swift.cpp
  ${FLATBUFFERS_DIR}/src/file_manager.cpp
  ${FLATBUFFERS_DIR}/src/file_name_manager.cpp
  ${FLATBUFFERS_DIR}/src/idl_namer.h
  ${FLATBUFFERS_DIR}/src/namer.h
  ${FLATBUFFERS_DIR}/src/flatc.cpp
  # ${FLATBUFFERS_DIR}/src/flatc_main.cpp
  ${FLATBUFFERS_DIR}/src/bfbs_gen.h
  ${FLATBUFFERS_DIR}/src/bfbs_gen_lua.h
  ${FLATBUFFERS_DIR}/src/bfbs_gen_nim.h
  ${FLATBUFFERS_DIR}/src/bfbs_namer.h
  ${FLATBUFFERS_DIR}/include/codegen/idl_namer.h
  ${FLATBUFFERS_DIR}/include/codegen/namer.h
  ${FLATBUFFERS_DIR}/include/codegen/python.h
  ${FLATBUFFERS_DIR}/include/codegen/python.cc
  ${FLATBUFFERS_DIR}/include/flatbuffers/code_generators.h
  ${FLATBUFFERS_DIR}/src/binary_annotator.h
  ${FLATBUFFERS_DIR}/src/binary_annotator.cpp
  ${FLATBUFFERS_DIR}/src/annotated_binary_text_gen.h
  ${FLATBUFFERS_DIR}/src/annotated_binary_text_gen.cpp
  ${FLATBUFFERS_DIR}/src/bfbs_gen_lua.cpp
  ${FLATBUFFERS_DIR}/src/bfbs_gen_nim.cpp
  ${FLATBUFFERS_DIR}/src/code_generators.cpp
  ${FLATBUFFERS_DIR}/grpc/src/compiler/schema_interface.h
  ${FLATBUFFERS_DIR}/grpc/src/compiler/cpp_generator.h
  ${FLATBUFFERS_DIR}/grpc/src/compiler/cpp_generator.cc
  ${FLATBUFFERS_DIR}/grpc/src/compiler/go_generator.h
  ${FLATBUFFERS_DIR}/grpc/src/compiler/go_generator.cc
  ${FLATBUFFERS_DIR}/grpc/src/compiler/java_generator.h
  ${FLATBUFFERS_DIR}/grpc/src/compiler/java_generator.cc
  ${FLATBUFFERS_DIR}/grpc/src/compiler/python_generator.h
  ${FLATBUFFERS_DIR}/grpc/src/compiler/python_generator.cc
  ${FLATBUFFERS_DIR}/grpc/src/compiler/swift_generator.h
  ${FLATBUFFERS_DIR}/grpc/src/compiler/swift_generator.cc
  ${FLATBUFFERS_DIR}/grpc/src/compiler/ts_generator.h
  ${FLATBUFFERS_DIR}/grpc/src/compiler/ts_generator.cc
)

include_directories(${FLATBUFFERS_DIR}/include)
include_directories(${FLATBUFFERS_DIR}/tests)
include_directories(${FLATBUFFERS_DIR}/src)
include_directories(${FLATBUFFERS_DIR}/grpc)

add_library(flatbuffers_fuzzed STATIC ${FlatBuffers_Library_SRCS})
# Use PUBLIC to force 'fuzzer_config' for all dependent targets
target_link_libraries(flatbuffers_fuzzed PUBLIC fuzzer_config)

# FLATBUFFERS_ASSERT should assert in Release as well. Redefine
# FLATBUFFERS_ASSERT macro definition. Declare as PUBLIC to cover asserts in all
# included header files.
target_compile_definitions(
  flatbuffers_fuzzed
  PUBLIC
    FLATBUFFERS_ASSERT=fuzzer_assert_impl
    FLATBUFFERS_ASSERT_INCLUDE="${CMAKE_CURRENT_SOURCE_DIR}/fuzzer_assert.h"
  PRIVATE
    FLATBUFFERS_MAX_PARSING_DEPTH=${FLATBUFFERS_MAX_PARSING_DEPTH}
)

# Setup fuzzer tests.

add_executable(scalar_fuzzer flatbuffers_scalar_fuzzer.cc)
target_link_libraries(scalar_fuzzer PRIVATE flatbuffers_fuzzed)

add_executable(parser_fuzzer flatbuffers_parser_fuzzer.cc)
target_link_libraries(parser_fuzzer PRIVATE flatbuffers_fuzzed)

add_executable(verifier_fuzzer flatbuffers_verifier_fuzzer.cc)
target_link_libraries(verifier_fuzzer PRIVATE flatbuffers_fuzzed)

add_executable(flexverifier_fuzzer flexbuffers_verifier_fuzzer.cc)
target_link_libraries(flexverifier_fuzzer PRIVATE flatbuffers_fuzzed)

add_executable(monster_fuzzer flatbuffers_monster_fuzzer.cc)
target_link_libraries(monster_fuzzer PRIVATE flatbuffers_fuzzed)

add_executable(codegen_fuzzer flatbuffers_codegen_fuzzer.cc ${FlatBuffers_Compiler_SRCS})
target_link_libraries(codegen_fuzzer PRIVATE flatbuffers_fuzzed)
target_compile_definitions(codegen_fuzzer PRIVATE assert=fuzzer_assert_impl)

add_custom_command(
  TARGET monster_fuzzer PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
  ${CMAKE_SOURCE_DIR}/../monster_test.bfbs
  ${CMAKE_CURRENT_BINARY_DIR}/monster_test.bfbs)

add_executable(annotator_fuzzer flatbuffers_annotator_fuzzer.cc)
target_link_libraries(annotator_fuzzer PRIVATE flatbuffers_fuzzed)
add_custom_command(
  TARGET annotator_fuzzer PRE_BUILD

  COMMAND ${CMAKE_COMMAND} -E copy
  ${CMAKE_SOURCE_DIR}/../annotated_binary/annotated_binary.bfbs
  ${CMAKE_CURRENT_BINARY_DIR}/annotated_binary.bfbs

  COMMAND ${CMAKE_COMMAND} -E copy
  ${CMAKE_SOURCE_DIR}/../annotated_binary/annotated_binary.bin
  ${CMAKE_CURRENT_BINARY_DIR}/seed_annotator/annotated_binary.bin
)

add_executable(64bit_fuzzer flatbuffers_64bit_fuzzer.cc)
target_link_libraries(64bit_fuzzer PRIVATE flatbuffers_fuzzed)
add_custom_command(
  TARGET 64bit_fuzzer PRE_BUILD

  COMMAND ${CMAKE_COMMAND} -E copy
  ${CMAKE_SOURCE_DIR}/../64bit/test_64bit.bin
  ${CMAKE_CURRENT_BINARY_DIR}/seed_64bit/test_64bit.bin
)

# Build debugger for weird cases found with fuzzer.
if(BUILD_DEBUGGER)
  add_library(flatbuffers_nonfuzz STATIC ${FlatBuffers_Library_SRCS})
  target_compile_options(
    flatbuffers_nonfuzz
    PUBLIC
      $<$<BOOL:${USE_ASAN}>:
        -fsanitize=undefined,address
      >
      -fno-limit-debug-info
  )

  target_link_libraries(
    flatbuffers_nonfuzz
    PUBLIC
      $<$<BOOL:${USE_ASAN}>:
        -fsanitize=undefined,address
      >
  )

  target_compile_definitions(
    flatbuffers_nonfuzz
    PUBLIC
      FLATBUFFERS_ASSERT=fuzzer_assert_impl
      FLATBUFFERS_ASSERT_INCLUDE="${CMAKE_CURRENT_SOURCE_DIR}/fuzzer_assert.h"
    PRIVATE
      FLATBUFFERS_MAX_PARSING_DEPTH=${FLATBUFFERS_MAX_PARSING_DEPTH}
  )
  add_executable(scalar_debug
    flatbuffers_scalar_fuzzer.cc
    scalar_debug.cpp
  )
  target_link_libraries(scalar_debug PRIVATE flatbuffers_nonfuzz)

  add_executable(monster_debug
    flatbuffers_monster_fuzzer.cc
    monster_debug.cpp
  )
  target_link_libraries(monster_debug PRIVATE flatbuffers_nonfuzz)
  add_custom_command(
    TARGET monster_debug PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_SOURCE_DIR}/../monster_test.bfbs
    ${CMAKE_CURRENT_BINARY_DIR}/monster_test.bfbs)

endif(BUILD_DEBUGGER)
