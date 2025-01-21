"""Helper macros and rules for tests."""

load("@bazel_binaries//:defs.bzl", "bazel_binaries")
load("@bazel_skylib//lib:paths.bzl", "paths")
load("@bazel_skylib//rules:expand_template.bzl", "expand_template")

def repo_name(label):
    if hasattr(label, "repo_name"):  # Added in Bazel 7.1
        return label.repo_name
    else:
        return "build_bazel_bazel_6_3_2"

def rlocationpath(label):
    """Returns the rlocationpath for a label

    Args:
        label (Label): The label to determine the rlocationpath for

    Returns:
        str: The rlocationpath for label
    """
    path = ""
    if repo_name(label):
        path += repo_name(label) + "/"
    if label.package:
        path += label.package + "/"
    path += label.name
    return path

def flatbuffers_as_external_repo_test(name, directory):
    """Run all tests in a bazel workspace that imports flatbuffers as an external repository.

    Args:
        name: The name of the test target.
        directory: The directory in which the bazel workspace is located. This is the directory
            that imports flatbuffers as an external repository.
    """
    bazel_binary_label = Label(bazel_binaries.label(bazel_binaries.versions.current))
    expand_template(
        name = name + "__template_expansion",
        out = name + ".sh",
        substitutions = {
            "{{REPOSITORY_DIR}}": paths.join(native.package_name(), directory),
            "{{BAZEL_PATH}}": rlocationpath(bazel_binary_label),
        },
        template = "//tests:bazel_repository_test_template.sh",
    )

    native.sh_test(
        name = name,
        srcs = [":%s.sh" % name],
        data = [
            "//:distribution",
            bazel_binary_label,
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
