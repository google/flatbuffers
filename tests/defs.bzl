"""Helper macros and rules for tests."""

load("@bazel_skylib//lib:paths.bzl", "paths")
load("@bazel_skylib//rules:expand_template.bzl", "expand_template")

def flatbuffers_as_external_repo_test(name, directory):
    """Run all tests in a bazel workspace that imports flatbuffers as an external repository.

    Args:
        name: The name of the test target.
        directory: The directory in which the bazel workspace is located. This is the directory
            that imports flatbuffers as an external repository.
    """
    expand_template(
        name = name + "__template_expansion",
        out = name + ".sh",
        substitutions = {
            "{{REPOSITORY_DIR}}": paths.join(native.package_name(), directory),
        },
        template = "//tests:bazel_repository_test_template.sh",
    )

    native.sh_test(
        name = name,
        srcs = [":%s.sh" % name],
        data = [
            "//:distribution",
            "@bazel_linux_x86_64//file",
        ] + native.glob(
            [
                directory + "/**/*",
            ],
            exclude = [
                directory + "/bazel-*/**",
            ],
        ),
        tags = [
            # Since we have bazel downloading external repositories inside this
            # test, we need to give it access to the internet.
            "requires-network",
        ],
        # We only have x86_64 Linux bazel exposed so restrict the test to that.
        target_compatible_with = [
            "@platforms//cpu:x86_64",
            "@platforms//os:linux",
        ],
        deps = ["@bazel_tools//tools/bash/runfiles"],
    )
