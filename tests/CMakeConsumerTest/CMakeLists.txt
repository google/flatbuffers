# Minimal downstream consumer used by CMakePackageTest.sh to exercise
# find_package(FlatBuffers) against differently-packaged installs (static
# only, shared only, both) and the component/variable/BUILD_SHARED_LIBS
# precedence chain in flatbuffers-config.cmake.
#
# This project is never add_subdirectory()'d into the main flatbuffers
# build; it is always configured standalone, against an install tree, with
# CMAKE_PREFIX_PATH pointing at the package under test.
cmake_minimum_required(VERSION 3.11)
project(flatbuffers_consumer_test CXX)

# Optional COMPONENTS argument to pass through to find_package(FlatBuffers),
# e.g. -DFLATBUFFERS_CONSUMER_TEST_COMPONENTS=static
set(FLATBUFFERS_CONSUMER_TEST_COMPONENTS "" CACHE STRING
    "COMPONENTS argument to pass to find_package(FlatBuffers), if any")

if(FLATBUFFERS_CONSUMER_TEST_COMPONENTS)
  find_package(FlatBuffers REQUIRED COMPONENTS ${FLATBUFFERS_CONSUMER_TEST_COMPONENTS})
else()
  find_package(FlatBuffers REQUIRED)
endif()

add_executable(consumer_canonical consumer.cpp)
target_link_libraries(consumer_canonical PRIVATE flatbuffers::flatbuffers)

if(TARGET flatbuffers::flatbuffers_shared)
  add_executable(consumer_legacy_shared consumer.cpp)
  target_link_libraries(consumer_legacy_shared PRIVATE flatbuffers::flatbuffers_shared)
endif()

get_target_property(FLATBUFFERS_CONSUMER_TEST_TYPE flatbuffers::flatbuffers TYPE)
message(STATUS "flatbuffers::flatbuffers TYPE = ${FLATBUFFERS_CONSUMER_TEST_TYPE}")
