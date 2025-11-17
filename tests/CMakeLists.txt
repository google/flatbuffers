# Test that bfbs generators handle namespace-first schemas correctly
find_package(Python3 REQUIRED COMPONENTS Interpreter)

add_test(
  NAME test_bfbs_namespace_output
  COMMAND ${Python3_EXECUTABLE} -m unittest test_bfbs_namespace_output.py -v
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)