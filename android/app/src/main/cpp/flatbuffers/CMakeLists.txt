# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

include_directories(${FLATBUFFERS_SRC}/include)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -fexceptions -Wall -DFLATBUFFERS_TRACK_VERIFIER_BUFFER_SIZE")

# Certain platforms such as ARM do not use signed chars by default
# which causes issues with certain bounds checks.
set(CMAKE_CXX_FLAGS
   "${CMAKE_CXX_FLAGS} -fsigned-char")

set(FlatBuffers_Library_SRCS
        ${FLATBUFFERS_DIR}/include/flatbuffers/allocator.h
        ${FLATBUFFERS_DIR}/include/flatbuffers/array.h
        ${FLATBUFFERS_DIR}/include/flatbuffers/base.h
        ${FLATBUFFERS_DIR}/include/flatbuffers/buffer_ref.h
        ${FLATBUFFERS_DIR}/include/flatbuffers/default_allocator.h
        ${FLATBUFFERS_DIR}/include/flatbuffers/detached_buffer.h
        ${FLATBUFFERS_DIR}/include/flatbuffers/flatbuffer_builder.h
        ${FLATBUFFERS_DIR}/include/flatbuffers/flatbuffers.h
        ${FLATBUFFERS_DIR}/include/flatbuffers/flexbuffers.h
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
        ${FLATBUFFERS_SRC}/src/idl_parser.cpp
        ${FLATBUFFERS_SRC}/src/idl_gen_text.cpp
        ${FLATBUFFERS_SRC}/src/reflection.cpp
        ${FLATBUFFERS_SRC}/src/util.cpp
        ${FLATBUFFERS_SRC}/src/idl_gen_fbs.cpp
        ${FLATBUFFERS_SRC}/src/code_generators.cpp
        )

set(FlatBuffers_Test_SRCS
        ${FLATBUFFERS_SRC}/tests/test.cpp
        ${FLATBUFFERS_SRC}/tests/test_assert.h
        ${FLATBUFFERS_SRC}/tests/test_builder.h
        ${FLATBUFFERS_SRC}/tests/test_assert.cpp
        ${FLATBUFFERS_SRC}/tests/test_builder.cpp
        ${FLATBUFFERS_SRC}/tests/native_type_test_impl.h
        ${FLATBUFFERS_SRC}/tests/native_type_test_impl.cpp
)

add_library( # Sets the name of the library.
             flatbuffers

             ${FlatBuffers_Library_SRCS}
             ${FlatBuffers_Test_SRCS}
             ${Generated_SRCS}
)

add_library( # Sets the name of the library.
             flatbuffers_tests

             ${FlatBuffers_Test_SRCS}
)
