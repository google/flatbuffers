load("@//bazel:flatbuffers_library.bzl", "FlatBuffersInfo", "flatbuffers_common")
load("@bazel_skylib//lib:dicts.bzl", "dicts")
load("@bazel_skylib//lib:paths.bzl", "paths")
load("@bazel_skylib//lib:types.bzl", "types")
load("@bazel_tools//tools/cpp:toolchain_utils.bzl", "find_cpp_toolchain", "use_cpp_toolchain")

def _cc_flatbuffers_aspect_impl(target, ctx):
    if CcInfo in target:  # target already provides CcInfo.
        return []

    filename_suffix = ("." + ctx.attr._suffix if ctx.attr._suffix else "") + ".fbs"

    headers = [
        ctx.actions.declare_file(paths.replace_extension(src.basename, "") + filename_suffix + ".h")
        for src in target[FlatBuffersInfo].direct_sources
    ]

    args = ctx.actions.args()
    args.add("-c")
    args.add("-I", ".")
    args.add("-I", ctx.bin_dir.path)
    args.add("-I", ctx.genfiles_dir.path)
    args.add("-o", paths.join(ctx.bin_dir.path, ctx.label.package))
    args.add("--filename-suffix", filename_suffix)
    args.add("--cpp-std", "C++17")
    args.add("--no-union-value-namespacing")
    args.add("--keep-prefix")

    if ctx.attr._generate_object_api:
        args.add("--gen-object-api")
    if ctx.attr._generate_compare:
        args.add("--gen-compare")
    if ctx.attr._generate_mutable:
        args.add("--gen-mutable")
    if ctx.attr._generate_reflection:
        args.add("--reflect-names")

    args.add_all(target[FlatBuffersInfo].direct_sources)

    ctx.actions.run(
        executable = ctx.executable._flatc,
        inputs = target[FlatBuffersInfo].transitive_sources,
        outputs = headers,
        arguments = [args],
        progress_message = "Generating FlatBuffers C++ code for {0}".format(ctx.label),
    )

    cc_toolchain = find_cpp_toolchain(ctx)
    feature_configuration = cc_common.configure_features(
        ctx = ctx,
        cc_toolchain = cc_toolchain,
        requested_features = ctx.features + ["parse_headers"],
        unsupported_features = ctx.disabled_features,
    )

    deps = getattr(ctx.rule.attr, "deps", [])
    compilation_contexts = [dep[CcInfo].compilation_context for dep in [ctx.attr._runtime] + deps]

    compilation_context, _ = cc_common.compile(
        name = ctx.label.name + ("_" + ctx.attr._suffix if ctx.attr._suffix else ""),
        actions = ctx.actions,
        feature_configuration = feature_configuration,
        cc_toolchain = cc_toolchain,
        public_hdrs = headers,
        compilation_contexts = compilation_contexts,
    )

    return [CcInfo(
        compilation_context = compilation_context,
    )]

def _make_cc_flatbuffers_aspect(*, suffix = "", generate_object_api, generate_compare, generate_mutable, generate_reflection):
    """Create a new cc_flatbuffers_aspect.

    Args:
        suffix: str, an optional suffix used in all targets that this aspect is applied to.
        generate_object_api: bool, whether to generate additional object-based API.
        generate_compare: bool, whether to generate operator== for object-based API types.
        generate_mutable: bool, whether to generate accessors that can mutate buffers in-place.
        generate_reflection: bool, whether to add minimal type/name reflection.

    Return:
        A new cc_flatbuffers_aspect with the provided configuration.
    """
    return aspect(
        attrs = dicts.add(
            {
                "_suffix": attr.string(
                    doc = "An optional suffix used in all targets that this aspect is applied to.",
                    default = suffix,
                ),
                "_generate_object_api": attr.bool(
                    doc = "Whether to generate additional object-based API.",
                    default = generate_object_api,
                ),
                "_generate_compare": attr.bool(
                    doc = "Whether to generate operator== for object-based API types.",
                    default = generate_compare,
                ),
                "_generate_mutable": attr.bool(
                    doc = "Whether to generate accessors that can mutate buffers in-place.",
                    default = generate_mutable,
                ),
                "_generate_reflection": attr.bool(
                    doc = "Whether to add minimal type/name reflection.",
                    default = generate_reflection,
                ),
                "_runtime": attr.label(
                    default = Label("@//:runtime_cc"),
                    providers = [CcInfo],
                ),
                "_cc_toolchain": attr.label(
                    default = Label("@bazel_tools//tools/cpp:current_cc_toolchain"),
                ),
                "_flatc": flatbuffers_common.attrs["_flatc"],
            },
        ),
        attr_aspects = ["deps"],
        fragments = ["google_cpp", "cpp"],
        implementation = _cc_flatbuffers_aspect_impl,
        toolchains = use_cpp_toolchain(),
    )

_cc_flatbuffers_aspect = _make_cc_flatbuffers_aspect(
    generate_object_api = True,
    generate_compare = True,
    generate_mutable = True,
    generate_reflection = True,
)

def _cc_flatbuffers_library_impl(ctx):
    return [cc_common.merge_cc_infos(
        direct_cc_infos = [dep[CcInfo] for dep in ctx.attr.deps],
    )]

def _make_cc_flatbuffers_library(*, doc, aspect):
    return rule(
        attrs = {
            "deps": attr.label_list(
                providers = [FlatBuffersInfo],
                aspects = [aspect],
            ),
            "includes": attr.string_list(),
        },
        doc = doc,
        implementation = _cc_flatbuffers_library_impl,
    )

cc_flatbuffers_library = _make_cc_flatbuffers_library(
    doc = """\
`cc_flatbuffers_library` generates C++ code from `.fbs` files.

`cc_flatbuffers_library` does:

-  generate additional object-based API;
-  generate operator== for object-based API types;
-  generate accessors that can mutate buffers in-place;
-  add minimal type/name reflection.

If those things aren't needed, consider using `cc_lite_flatbuffers_library` instead.

Example:

```build
load("//third_party/flatbuffers:flatbuffers.bzl", "cc_flatbuffers_library", "flatbuffers_library")

flatbuffers_library(
    name = "foo_fbs",
    srcs = ["foo.fbs"],
)

cc_flatbuffers_library(
    name = "foo_cc_fbs",
    deps = [":foo_fbs"],
)

# An example library that uses the generated C++ code from `foo_cc_fbs`.
cc_library(
    name = "foo_user",
    hdrs = ["foo_user.h"],
    deps = [":foo_cc_fbs"],
)
```

Where `foo_user.h` would include the generated code via:  `#include "//path/to/project/foo.fbs.h"`
""",
    aspect = _cc_flatbuffers_aspect,
)
