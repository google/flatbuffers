load("@bazel_skylib//lib:unittest.bzl", "unittest")
load("@bazel_skylib//rules:build_test.bzl", "build_test")
load("@bazel_skylib//rules:write_file.bzl", "write_file")
load("//:flatbuffers.bzl", "flatbuffers_library")

_DEMO_FBS = """
namespace ns;

table Table {
  name:string;
}
"""

def _test_flatbuffers_library(name):
    """Verifies that _kt_flatbuffers_aspect produces validation output.

    Args:
        name: a unique name for this test.
    """
    fbs = "{0}_fbs".format(name)
    write_file(
        name = fbs,
        content = [_DEMO_FBS],
        out = "{0}.fbs".format(name),
        testonly = True,
    )

    subject = "{0}_subject".format(name)
    flatbuffers_library(
        name = subject,
        srcs = [fbs],
        testonly = True,
    )

    build_test(
        name = name,
        targets = [subject],
    )

def flatbuffers_test(name):
    unittest.suite(
        name,
        _test_flatbuffers_library,
    )
