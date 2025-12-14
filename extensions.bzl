"""Bzlmod extensions"""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_file")

def _non_module_dependencies_impl(_ctx):
    """Non module dependencies"""
    http_file(
        name = "bazel_linux_x86_64",
        downloaded_file_path = "bazel",
        executable = True,
        sha256 = "e78fc3394deae5408d6f49a15c7b1e615901969ecf6e50d55ef899996b0b8458",
        urls = [
            "https://github.com/bazelbuild/bazel/releases/download/6.3.2/bazel-6.3.2-linux-x86_64",
        ],
    )

non_module_dependencies = module_extension(
    implementation = _non_module_dependencies_impl,
)
