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
# * FLATBUFFERS_FLATC_EXECUTABLE the flatc compiler executable
# * FLATBUFFERS_FOUND
#
# Provides:
# * FLATBUFFERS_GENERATE_C_HEADERS(Name FLATBUFFERS_DIR OUTPUT_DIR <files>) creates the C++ headers
#   for the given flatbuffer schema files.
#   Returns the header files in ${Name}_OUTPUTS
#   Name is the CMake variable name prefix that will be used.
#        for example MY_FLATBUFFERS will set the variable 
#        MY_FLATBUFFERS_OUTPUTS with the 
#   FLATBUFFERS_DIR is the directory where the flatbuffers are stored.
#   OUTPUT_DIR is where the output generated flatbuffer files are stored.
#
#
# Usage Example:
#
# # list flatbuffer headers
# set(RFB ArmControlState.fbs Geometry.fbs JointState.fbs KUKAiiwa.fbs LinkObject.fbs Euler.fbs Time.fbs VrepControlPoint.fbs VrepPath.fbs)
# # directory to include flatbuffers
# set(GRL_FLATBUFFERS_INCLUDE_DIR ${CMAKE_BINARY_DIR}/include)
# # Generate flatbuffer message C++ headers
# flatbuffers_generate_c_headers(GRL_FLATBUFFERS include/grl/flatbuffer/  ${GRL_FLATBUFFERS_INCLUDE_DIR}/grl/flatbuffer ${RFB})
# add_custom_target(grlflatbuffers DEPENDS ${GRL_FLATBUFFERS_OUTPUTS})
# include_directories(${GRL_FLATBUFFERS_INCLUDE_DIR} )

find_program(FLATBUFFERS_FLATC_EXECUTABLE NAMES flatc)
find_path(FLATBUFFERS_INCLUDE_DIR NAMES flatbuffers/flatbuffers.h)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(flatbuffers
  DEFAULT_MSG FLATBUFFERS_FLATC_EXECUTABLE FLATBUFFERS_INCLUDE_DIR)

if(FLATBUFFERS_FOUND) 
  function(FLATBUFFERS_GENERATE_C_HEADERS Name FLATBUFFERS_DIR OUTPUT_DIR)
    set(FLATC_OUTPUTS)
    foreach(FILE ${ARGN})
      get_filename_component(FLATC_OUTPUT ${FILE} NAME_WE)
      set(FLATC_OUTPUT
        "${OUTPUT_DIR}/${FLATC_OUTPUT}_generated.h")
      list(APPEND FLATC_OUTPUTS ${FLATC_OUTPUT})

      add_custom_command(OUTPUT ${FLATC_OUTPUT}
        COMMAND ${FLATBUFFERS_FLATC_EXECUTABLE}
        ARGS -c -o "${OUTPUT_DIR}" ${FILE}
		MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${FLATBUFFERS_DIR}/${FILE}
        COMMENT "Building C++ header for ${FILE}"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${FLATBUFFERS_DIR})
    endforeach()
    set(${Name}_OUTPUTS ${FLATC_OUTPUTS} PARENT_SCOPE)
  endfunction()

  set(FLATBUFFERS_INCLUDE_DIRS ${FLATBUFFERS_INCLUDE_DIR})
  include_directories(${CMAKE_BINARY_DIR})
else()
  set(FLATBUFFERS_INCLUDE_DIR)
endif()

include("${FLATBUFFERS_CMAKE_DIR}/BuildFlatBuffers.cmake")
