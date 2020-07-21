load("@rules_cc//cc:defs.bzl", "cc_library")

package(
    default_visibility = ["//visibility:public"],
)

filegroup(
    name = "common_headers",
    srcs = [
        "config.h",
        "schema_interface.h",
    ],
)

cc_library(
    name = "cpp_generator",
    srcs = [
        "cpp_generator.cc",
    ],
    hdrs = [
        "cpp_generator.h",
        ":common_headers",
    ],
    include_prefix = "src/compiler",
    strip_include_prefix = "/grpc/src/compiler",
    deps = [
        "//:flatbuffers",
    ],
)

cc_library(
    name = "go_generator",
    srcs = [
        "go_generator.cc",
    ],
    hdrs = [
        "go_generator.h",
        ":common_headers",
    ],
    include_prefix = "src/compiler",
    strip_include_prefix = "/grpc/src/compiler",
    deps = [
        "//:flatbuffers",
    ],
)

cc_library(
    name = "java_generator",
    srcs = [
        "java_generator.cc",
    ],
    hdrs = [
        "java_generator.h",
        ":common_headers",
    ],
    include_prefix = "src/compiler",
    strip_include_prefix = "/grpc/src/compiler",
    deps = [
        "//:flatbuffers",
    ],
)

cc_library(
    name = "python_generator",
    hdrs = [
        "python_generator.h",
    ],
    include_prefix = "src/compiler",
    strip_include_prefix = "/grpc/src/compiler",
    deps = [
        ":python_generator_private",
    ],
)

cc_library(
    name = "python_generator_private",
    srcs = [
        "python_generator.cc",
    ],
    hdrs = [
        "python_generator.h",
        "python_private_generator.h",
        ":common_headers",
    ],
    include_prefix = "src/compiler",
    strip_include_prefix = "/grpc/src/compiler",
    visibility = ["//visibility:private"],
    deps = [
        "//:flatbuffers",
    ],
)

cc_library(
    name = "swift_generator",
    srcs = [
        "swift_generator.cc",
    ],
    hdrs = [
        "swift_generator.h",
        ":common_headers",
    ],
    include_prefix = "src/compiler",
    strip_include_prefix = "/grpc/src/compiler",
    deps = [
        "//:flatbuffers",
    ],
)
