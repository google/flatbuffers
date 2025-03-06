load("@bazel_skylib//lib:unittest.bzl", "unittest")
load("@bazel_skylib//rules:build_test.bzl", "build_test")
load("@bazel_skylib//rules:write_file.bzl", "write_file")
load("@rules_cc//cc:defs.bzl", "cc_test")
load("@rules_python//python:defs.bzl", "py_test")
load("//:flatbuffers.bzl", "cc_flatbuffers_library", "flatbuffers_library", "py_flatbuffers_library")

_DEMO_FBS = """
namespace ns;

table DemoTable {
  name:string;
  uint8s:[uint8];
}
"""

def _test_flatbuffers_library(name):
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

def _test_cc_flatbuffers_library(name):
    fbs = "{0}_fbs".format(name)
    write_file(
        name = fbs,
        content = [_DEMO_FBS],
        out = "{0}.fbs".format(name),
        testonly = True,
    )

    flatbuffers_library(
        name = "{0}_fbs_lib".format(name),
        srcs = [fbs],
        testonly = True,
    )

    cc_flatbuffers_library(
        name = "{0}_cc_fbs_lib".format(name),
        deps = ["{0}_fbs_lib".format(name)],
        testonly = True,
    )

    test_cc = "{0}_test_cc".format(name)
    write_file(
        name = test_cc,
        content = ["""
            #include <type_traits>
            #include <utility>
            #include <gtest/gtest.h>

            #include "%s.fbs.h"

            TEST(FlatBuffersLibraryTest, Cc) {
              EXPECT_TRUE(std::is_member_function_pointer_v<decltype(&::ns::DemoTable::name)>);
              EXPECT_TRUE(std::is_member_function_pointer_v<decltype(&::ns::DemoTableBuilder::add_name)>);
            }
        """ % (name)],
        out = "{0}_test.cc".format(name),
        testonly = True,
    )

    cc_test(
        name = name,
        srcs = [test_cc],
        deps = [
            "{0}_cc_fbs_lib".format(name),
            "@googletest//:gtest_main",
        ],
    )

def _test_py_flatbuffers_library(name):
    fbs = "{0}_fbs".format(name)
    write_file(
        name = fbs,
        content = [_DEMO_FBS],
        out = "{0}.fbs".format(name),
        testonly = True,
    )

    flatbuffers_library(
        name = "{0}_fbs_lib".format(name),
        srcs = [fbs],
        testonly = True,
    )

    py_flatbuffers_library(
        name = "{0}_py_fbs_lib".format(name),
        deps = ["{0}_fbs_lib".format(name)],
        testonly = True,
    )

    test_py = "{0}_test_py".format(name)
    write_file(
        name = test_py,
        content = ["""
import unittest

from %s_fbs import DemoTable

class TestFlatBuffersLibrary(unittest.TestCase):

  def test_declares_types(self):
    self.assertTrue(hasattr(DemoTable, "Uint8sAsNumpy"))

if __name__ == "__main__":
  unittest.main()
""" % (name)],
        out = "{0}.py".format(name),
        testonly = True,
    )

    py_test(
        name = name,
        srcs = [test_py],
        deps = ["{0}_py_fbs_lib".format(name)],
    )

def flatbuffers_test(name):
    unittest.suite(
        name,
        _test_flatbuffers_library,
        _test_cc_flatbuffers_library,
        _test_py_flatbuffers_library,
    )
