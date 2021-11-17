set(GOLDEN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/goldens)

function(generate_golden)
  set(options OPTION)
  set(oneValueArgs SCHEMA OUT_DIR)
  set(multiValueArgs OPTIONS)
  cmake_parse_arguments(
    PARSE_ARGV 0 GOLDEN "${options}" "${oneValueArgs}" "${multiValueArgs}")

  # Invoke flatc 
  add_custom_command(    
    TARGET flatc
    POST_BUILD
    COMMAND ${FLATBUFFERS_FLATC_EXECUTABLE}      
            ${GOLDEN_OPTIONS}                   
            -o ${GOLDEN_DIR}/${GOLDEN_OUT_DIR}
            ${GOLDEN_DIR}/${GOLDEN_SCHEMA}
    COMMENT "Generating Golden [${GOLDEN_SCHEMA}] with '${GOLDEN_OPTIONS}'...")
endfunction()

set(GOLD_BASE_OPTS --reflect-names --gen-mutable --gen-object-api)
set(GOLD_CPP_OPTS ${GOLD_BASE_OPTS} --cpp --gen-compare)
set(GOLD_LUA_OPTS ${GOLD_BASE_OPTS} --lua)


generate_golden(SCHEMA golden.fbs OPTIONS ${GOLD_CPP_OPTS} OUT_DIR cpp)
generate_golden(SCHEMA golden.fbs OPTIONS ${GOLD_LUA_OPTS} OUT_DIR lua)

