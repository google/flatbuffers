function(flatbuffers_adversarial_namespaces_test)
  set(options)
  set(args CASE)
  set(list_args FLAGS)

  cmake_parse_arguments("ARG" "${options}" "${args}" "${list_args}" ${ARGN})

  add_custom_target(flattests.adversarial_namespace.${ARG_CASE}.run_flatc
    COMMAND
      flatc --cpp
      -o ${CMAKE_CURRENT_BINARY_DIR}/generated
      ${ARG_FLAGS}
      ${CMAKE_CURRENT_SOURCE_DIR}/test_cases/${ARG_CASE}.fbs
    DEPENDS flatc ${CMAKE_CURRENT_SOURCE_DIR}/test_cases/${ARG_CASE}.fbs
    BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/generated/${ARG_CASE}_generated.h
  )
  add_executable(flattests.adversarial_namespace.${ARG_CASE}
    ${CMAKE_CURRENT_BINARY_DIR}/generated/${ARG_CASE}_generated.h
    test_cases/${ARG_CASE}.cpp
  )
  add_dependencies(flattests.adversarial_namespace.${ARG_CASE} flattests.adversarial_namespace.${ARG_CASE}.run_flatc)
  set_target_properties(
      flattests.adversarial_namespace.${ARG_CASE}
    PROPERTIES
      EXCLUDE_FROM_ALL TRUE
      EXCLUDE_FROM_DEFAULT_BUILD TRUE
  )
  add_test(
    NAME flatbuffers.adversarial_namespace.${ARG_CASE}
    COMMAND
      ${CMAKE_COMMAND}
      --build ${CMAKE_BINARY_DIR}
      --config $<CONFIG>
      --target flattests.adversarial_namespace.${ARG_CASE}
  )
endfunction()

include_directories(${CMAKE_CURRENT_BINARY_DIR}/generated)

flatbuffers_adversarial_namespaces_test(CASE flatbuffers)
flatbuffers_adversarial_namespaces_test(CASE external_flatbuffers)
flatbuffers_adversarial_namespaces_test(CASE std                      FLAGS --gen-object-api)
flatbuffers_adversarial_namespaces_test(CASE custom_string_attribute  FLAGS --gen-object-api)
flatbuffers_adversarial_namespaces_test(CASE custom_ptr_attribute     FLAGS --gen-object-api)
flatbuffers_adversarial_namespaces_test(CASE custom_string            FLAGS --gen-object-api --cpp-str-type some_namespace::custom_string)
flatbuffers_adversarial_namespaces_test(CASE custom_ptr               FLAGS --gen-object-api --cpp-ptr-type some_namespace::custom_ptr)
