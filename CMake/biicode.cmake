# Initializes block variables
INIT_BIICODE_BLOCK()

# Copying data files to project/bin folder
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/samples")
  file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/samples/monster.fbs"
            "${CMAKE_CURRENT_SOURCE_DIR}/samples/monsterdata.json"
      DESTINATION
            "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/samples")
endif()
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests")
  file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/tests"
       DESTINATION
            "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
endif()

# Actually create targets: EXEcutables and libraries.
ADD_BIICODE_TARGETS()

string(REPLACE " " ";" REPLACED_FLAGS ${CMAKE_CXX_FLAGS})
target_compile_options(${BII_BLOCK_TARGET} INTERFACE ${REPLACED_FLAGS})
target_include_directories(${BII_BLOCK_TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)