load("@rules_cc//cc:defs.bzl", "cc_test")

cc_test(
    name = "grpc_test",
    srcs = [
        "grpctest.cpp",
        "message_builder_test.cpp",
    ],
    copts = ["-Itests"],
    # This is required.
    linkstatic = 1,
    deps = [
        "//tests:monster_test_cc_fbs",
        "//tests:monster_test_grpc",
        "//tests:test_assert",
        "//tests:test_builder",
        "@com_github_grpc_grpc//:grpc++",
    ],
)

cc_test(
    name = "grpc_callback_compile_test",
    srcs = ["grpctest_callback_compile.cpp"],
    copts = ["-Itests"],
    linkstatic = 1,
    deps = [
        "//tests:monster_test_cc_fbs",
        "//tests:monster_test_grpc",
        "@com_github_grpc_grpc//:grpc++",
    ],
)

cc_test(
    name = "grpc_callback_client_compile_test",
    srcs = ["grpctest_callback_client_compile.cpp"],
    copts = ["-Itests"],
    linkstatic = 1,
    deps = [
        "//tests:monster_test_cc_fbs",
        "//tests:monster_test_grpc",
        "@com_github_grpc_grpc//:grpc++",
    ],
)
