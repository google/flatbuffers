set(VERSION_MAJOR 2)
set(VERSION_MINOR 0)
set(VERSION_PATCH 5)
set(VERSION_COMMIT 0)

find_program(GIT git)
if(GIT AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
  execute_process(
      COMMAND ${GIT} describe --tags
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      OUTPUT_VARIABLE GIT_DESCRIBE_DIRTY
      OUTPUT_STRIP_TRAILING_WHITESPACE
      RESULT_VARIABLE GIT_DESCRIBE_RESULT
  )

  if(GIT_DESCRIBE_RESULT EQUAL 0)
    string(REGEX REPLACE "^v([0-9]+)\\..*" "\\1" VERSION_MAJOR "${GIT_DESCRIBE_DIRTY}")
    string(REGEX REPLACE "^v[0-9]+\\.([0-9]+).*" "\\1" VERSION_MINOR "${GIT_DESCRIBE_DIRTY}")
    string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" VERSION_PATCH "${GIT_DESCRIBE_DIRTY}")
    string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.[0-9]+\\-([0-9]+).*" "\\1" VERSION_COMMIT "${GIT_DESCRIBE_DIRTY}")
  else()
    message(WARNING "git describe failed with exit code: ${GIT_DESCRIBE_RESULT}")
  endif()
else()
  message(WARNING "git is not found")
endif()

message(STATUS "Proceeding with version: ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${VERSION_COMMIT}")
