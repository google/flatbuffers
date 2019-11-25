package(
    default_visibility = ["//:__subpackages__"],
)

load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

filegroup(
    name = "public_headers",
    srcs = [
        "flatbuffers/base.h",
        "flatbuffers/code_generators.h",
        "flatbuffers/flatbuffers.h",
        "flatbuffers/flexbuffers.h",
        "flatbuffers/hash.h",
        "flatbuffers/idl.h",
        "flatbuffers/minireflect.h",
        "flatbuffers/reflection.h",
        "flatbuffers/reflection_generated.h",
        "flatbuffers/registry.h",
        "flatbuffers/stl_emulation.h",
        "flatbuffers/util.h",
    ],
)

filegroup(
    name = "flatc",
    srcs = [
        "flatbuffers/flatc.h",
    ],
)

# Library used by flatbuffer_cc_library rules.
cc_library(
    name = "runtime_cc",
    hdrs = [
        "flatbuffers/base.h",
        "flatbuffers/flatbuffers.h",
        "flatbuffers/flexbuffers.h",
        "flatbuffers/stl_emulation.h",
        "flatbuffers/util.h",
    ],
    linkstatic = 1,
    visibility = ["//:__pkg__"],
)
