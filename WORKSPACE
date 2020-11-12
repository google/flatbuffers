workspace(name = "com_github_google_flatbuffers")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "io_bazel_rules_go",
    sha256 = "d1ffd055969c8f8d431e2d439813e42326961d0942bdf734d2c95dc30c369566",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/rules_go/releases/download/v0.24.5/rules_go-v0.24.5.tar.gz",
        "https://github.com/bazelbuild/rules_go/releases/download/v0.24.5/rules_go-v0.24.5.tar.gz",
    ],
)

load("@io_bazel_rules_go//go:deps.bzl", "go_register_toolchains", "go_rules_dependencies")

go_rules_dependencies()

go_register_toolchains()

##### Protobuf
_PROTOBUF_VERSION = "3.6.1"

http_archive(
    name = "com_google_protobuf",
    patch_args = ["-p1"],
    patches = [
        "//bazel:protobuf_deps_3_6_1.patch",
    ],
    strip_prefix = "protobuf-" + _PROTOBUF_VERSION,
    urls = [
        "https://github.com/protocolbuffers/protobuf/archive/v" + _PROTOBUF_VERSION + ".tar.gz",
    ],
)

##### GRPC
_GRPC_VERSION = "1.15.1"

http_archive(
    name = "com_github_grpc_grpc",
    patch_args = ["-p1"],
    patches = [
        "//bazel:grpc_deps_1_15_1.patch",
        "//bazel:grpc_deps_1_15_1_02.patch",
    ],
    strip_prefix = "grpc-" + _GRPC_VERSION,
    urls = ["https://github.com/grpc/grpc/archive/v" + _GRPC_VERSION + ".tar.gz"],
)

load("@com_github_grpc_grpc//bazel:grpc_deps.bzl", "grpc_deps")

grpc_deps()
