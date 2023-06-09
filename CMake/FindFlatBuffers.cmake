# Copyright 2014 Stefan.Eilemann@epfl.ch
# Copyright 2014 Google Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Find the flatbuffers schema compiler
#
# Output Variables:
# * FLATBUFFERS_FLATC_EXECUTABLE - The flatc compiler executable
# * FLATBUFFERS_FLATC_VERSION - The version of flatc found.
# * FlatBuffers_FOUND - Whether flatc was found.
# * FLATBUFFERS_FOUND - Legacy alias of the above (all caps).
#
# If flatc is found, the following imported target is created:
# * flatbuffers::flatc - Imported target for the compiler

set(FLATBUFFERS_CMAKE_DIR ${CMAKE_CURRENT_LIST_DIR})

find_program(FLATBUFFERS_FLATC_EXECUTABLE NAMES flatc)

if(EXISTS ${FLATBUFFERS_FLATC_EXECUTABLE})
  # detect version
  execute_process(COMMAND ${FLATBUFFERS_FLATC_EXECUTABLE} --version
    RESULT_VARIABLE FLATBUFFERS_FLATC_VERSION_RESULT
    OUTPUT_VARIABLE FLATBUFFERS_FLATC_VERSION_OUTPUT)

  if(FLATBUFFERS_FLATC_VERSION_RESULT EQUAL 0)
    # The output looks like "flatc version 23.3.3", so use a regex to trim out the part we need
    string(REGEX REPLACE "flatc version ([0-9]+\\.[0-9]+\\.[0-9]+).*" "\\1" FLATBUFFERS_FLATC_VERSION ${FLATBUFFERS_FLATC_VERSION_OUTPUT})
  else()
    message(WARNING "Failed to execute flatc to check version")
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FlatBuffers
  VERSION_VAR FLATBUFFERS_FLATC_VERSION
  REQUIRED_VARS FLATBUFFERS_FLATC_EXECUTABLE)

# CMake standard requires the found var to match the case of the filename.
# Provide legacy alias for all-caps.
set(FLATBUFFERS_FOUND ${FlatBuffers_FOUND})

if(FlatBuffers_FOUND)
  # Provide imported target for the executable
  add_executable(flatbuffers::flatc IMPORTED GLOBAL)
  set_property(TARGET flatbuffers::flatc PROPERTY IMPORTED_LOCATION ${FLATBUFFERS_FLATC_EXECUTABLE})
endif()

include("${FLATBUFFERS_CMAKE_DIR}/BuildFlatBuffers.cmake")
