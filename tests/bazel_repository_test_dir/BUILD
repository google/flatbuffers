# This test doesn't actually make use of the flatbuffers library. It's just
# here to make sure we can link the library properly when it comes from an
# external repository.  You're welcome to expand this test to do more.
cc_test(
    name = "pulls_in_flatbuffers_test",
    srcs = ["pulls_in_flatbuffers_test.cpp"],
    deps = [
        "@com_github_google_flatbuffers//:flatbuffers",
    ],
)
