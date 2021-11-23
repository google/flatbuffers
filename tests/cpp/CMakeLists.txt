cmake_minimum_required(VERSION 3.14)
include(FetchContent)

IF(MSVC)
    # Skip treating warnings as errors for depenedncies
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX-")
ENDIF()

# No particular reason for the specific GIT_TAGs for the following repos, they
# were just the latest releases when this was added.
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        e2239ee6043f73722e7aa812a459f54a28552929 # release-1.11.0
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(cpp_test_srcs
    vector_downward_test.cpp)

add_executable(cpp_test ${cpp_test_srcs})

target_link_libraries(cpp_test 
    PRIVATE
    gtest_main 
    flatbuffers)

add_test(NAME cpp_unit_test COMMAND cpp_test)