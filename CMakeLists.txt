cmake_minimum_required(VERSION 2.8)

project(FlatBuffers)

# NOTE: Code coverage only works on Linux & OSX.
option(FLATBUFFERS_CODE_COVERAGE "Enable the code coverage build option." OFF)

set(FlatBuffers_Compiler_SRCS
  include/flatbuffers/flatbuffers.h
  include/flatbuffers/idl.h
  include/flatbuffers/util.h
  src/idl_parser.cpp
  src/idl_gen_cpp.cpp
  src/idl_gen_java.cpp
  src/idl_gen_csharp.cpp
  src/idl_gen_go.cpp
  src/idl_gen_text.cpp
  src/flatc.cpp
)

set(FlatBuffers_Tests_SRCS
  include/flatbuffers/flatbuffers.h
  include/flatbuffers/idl.h
  include/flatbuffers/util.h
  src/idl_parser.cpp
  src/idl_gen_text.cpp
  tests/test.cpp
  # file generate by running compiler on tests/monster_test.fbs
  tests/monster_test_generated.h
)

set(FlatBuffers_Sample_Binary_SRCS
  include/flatbuffers/flatbuffers.h
  samples/sample_binary.cpp
  # file generate by running compiler on samples/monster.fbs
  samples/monster_generated.h
)

set(FlatBuffers_Sample_Text_SRCS
  include/flatbuffers/flatbuffers.h
  include/flatbuffers/idl.h
  include/flatbuffers/util.h
  src/idl_parser.cpp
  src/idl_gen_text.cpp
  samples/sample_text.cpp
  # file generate by running compiler on samples/monster.fbs
  samples/monster_generated.h
)

set(CMAKE_BUILD_TYPE Debug)

# source_group(Compiler FILES ${FlatBuffers_Compiler_SRCS})
# source_group(Tests FILES ${FlatBuffers_Tests_SRCS})

if(APPLE)
  set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++ -Wall -pedantic -Werror -Wextra")
elseif(CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -pedantic -Werror -Wextra")
endif()

if(FLATBUFFERS_CODE_COVERAGE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fprofile-arcs -ftest-coverage")
  set(CMAKE_EXE_LINKER_FLAGS
      "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
endif()

include_directories(include)

add_executable(flatc ${FlatBuffers_Compiler_SRCS})
add_executable(flattests ${FlatBuffers_Tests_SRCS})
add_executable(flatsamplebinary ${FlatBuffers_Sample_Binary_SRCS})
add_executable(flatsampletext ${FlatBuffers_Sample_Text_SRCS})

install(DIRECTORY include/flatbuffers DESTINATION include)
install(TARGETS flatc DESTINATION bin)

add_test(NAME flattest
         CONFIGURATIONS Debug
         WORKING_DIRECTORY tests
         COMMAND flattests)
