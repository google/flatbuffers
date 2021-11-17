set(GOLDEN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/goldens)

include(CMakeParseArguments)

function(generate_golden)
  set(options OPTION)
  set(oneValueArgs TARGET OUT_DIR)
  set(multiValueArgs OPTIONS)
  cmake_parse_arguments(PARSE_ARGV 0 GOLDEN "${options}" "${oneValueArgs}" "${multiValueArgs}")
  message(STATUS ${GOLDEN_TARGET})
  add_custom_command(    
    TARGET flatc
    POST_BUILD
    COMMAND "${FLATBUFFERS_FLATC_EXECUTABLE}"      
            ${GOLDEN_OPTIONS}                   
            -o ${GOLDEN_DIR}/${GOLDEN_OUT_DIR}
            "${GOLDEN_DIR}/${GOLDEN_TARGET}"
    COMMENT "Generate Golden...")
endfunction()

generate_golden(TARGET golden.fbs OPTIONS --cpp OUT_DIR cpp/)

