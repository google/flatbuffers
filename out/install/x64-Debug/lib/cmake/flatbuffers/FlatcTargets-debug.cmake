#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "flatbuffers::flatc" for configuration "Debug"
set_property(TARGET flatbuffers::flatc APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(flatbuffers::flatc PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/bin/flatc.exe"
  )

list(APPEND _IMPORT_CHECK_TARGETS flatbuffers::flatc )
list(APPEND _IMPORT_CHECK_FILES_FOR_flatbuffers::flatc "${_IMPORT_PREFIX}/bin/flatc.exe" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
