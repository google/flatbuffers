# Find the flatbuffers schema compiler
# Copyright 2014 Stefan.Eilemann@epfl.ch
#
# Output Variables:
# * FLATC_EXECUTABLE the flatc compiler executable
# * FLATBUFFERS_FOUND
#
# Provides:
# * FLATC_TARGET(Name <files>) creates the C++ headers for the given flatbuffer
#   schema files. Returns the header files in ${Name}_OUTPUTS

find_program(FLATC_EXECUTABLE NAMES flatc)
find_path(FLATBUFFERS_INCLUDE_DIR NAMES flatbuffers/flatbuffers.h)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(flatbuffers DEFAULT_MSG FLATC_EXECUTABLE
  FLATBUFFERS_INCLUDE_DIR)

if(FLATBUFFERS_FOUND)
  function(FLATC_TARGET Name)
    set(FLATC_OUTPUTS)
    foreach(FILE ${ARGN})
      get_filename_component(FLATC_OUTPUT ${FILE} NAME_WE)
      set(FLATC_OUTPUT
        "${CMAKE_CURRENT_BINARY_DIR}/${FLATC_OUTPUT}_generated.h")
      list(APPEND FLATC_OUTPUTS ${FLATC_OUTPUT})

      add_custom_command(OUTPUT ${FLATC_OUTPUT}
        COMMAND ${FLATC_EXECUTABLE}
        ARGS -c -o "${CMAKE_CURRENT_BINARY_DIR}/" ${FILE}
        COMMENT "Building C++ header for ${FILE}"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    endforeach()
    set(${Name}_OUTPUTS ${FLATC_OUTPUTS} PARENT_SCOPE)
  endfunction()

  set(FLATBUFFERS_INCLUDE_DIRS ${FLATBUFFERS_INCLUDE_DIR})
  include_directories(${CMAKE_BINARY_DIR})
else()
  set(FLATBUFFERS_INCLUDE_DIR)
endif()
