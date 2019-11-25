licenses(["notice"])

load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

package(
    default_visibility = ["//visibility:public"],
)

exports_files([
    "LICENSE",
])

# Public flatc library to compile flatbuffer files at runtime.
cc_library(
    name = "flatbuffers",
    hdrs = ["//:public_headers"],
    linkstatic = 1,
    strip_include_prefix = "/include",
    deps = ["//src:flatbuffers"],
)

# Public C++ headers for the Flatbuffers library.
filegroup(
    name = "public_headers",
    srcs = [
        "//include:public_headers",
    ],
)

# Public flatc compiler library.
cc_library(
    name = "flatc_library",
    linkstatic = 1,
    deps = [
        "//src:flatc_library",
    ],
)

# Public flatc compiler.
cc_binary(
    name = "flatc",
    deps = [
        "//src:flatc",
    ],
)

# Library used by flatbuffer_cc_library rules.
cc_library(
    name = "runtime_cc",
    linkstatic = 1,
    deps = [
        "//include:runtime_cc",
    ],
)
