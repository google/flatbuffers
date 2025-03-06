"""Provides Bazel build rules for FlatBuffers."""

load("@bazel_skylib//lib:paths.bzl", "paths")
load("@bazel_skylib//lib:types.bzl", "types")
load("@rules_cc//cc:defs.bzl", "CcInfo", "cc_common")
load("@rules_cc//cc:find_cc_toolchain.bzl", "find_cc_toolchain", "use_cc_toolchain")
load("@rules_python//python:py_cc_link_params_info.bzl", "PyCcLinkParamsInfo")

################################## providers ###################################

def _init_flatbuffers_info(*, direct_sources = [], direct_schemas = [], transitive_sources = depset(), transitive_schemas = depset()):
    """_init_flatbuffers_info is a public constructor for FlatBuffersInfo."""
    if not types.is_list(direct_sources):
        fail("direct_sources must be a list (got %s)" % type(direct_sources))

    if not types.is_list(direct_schemas):
        fail("direct_schemas must be a list (got %s)" % type(direct_schemas))

    if not types.is_depset(transitive_sources):
        fail("transitive_sources must be a depset (got %s)" % type(transitive_sources))

    if not types.is_depset(transitive_schemas):
        fail("transitive_schemas must be a depset (got %s)" % type(transitive_schemas))

    return {
        "direct_sources": direct_sources,
        "direct_schemas": direct_schemas,
        "transitive_sources": transitive_sources,
        "transitive_schemas": transitive_schemas,
    }

FlatBuffersInfo, _new_flatbuffers_info = provider(
    doc = "Encapsulates information provided by flatbuffers_library.",
    fields = {
        "direct_sources": "FlatBuffers sources (i.e. .fbs) from the \"srcs\" attribute that contain text-based schema.",
        "direct_schemas": "The binary serialized schema files (i.e. .bfbs) of the direct sources.",
        "transitive_sources": "FlatBuffers sources (i.e. .fbs) for this and all its dependent FlatBuffers targets.",
        "transitive_schemas": "A set of binary serialized schema files (i.e. .bfbs) for this and all its dependent FlatBuffers targets.",
    },
    init = _init_flatbuffers_info,
)

def _create_flatbuffers_info(*, srcs, schemas, deps = None):
    deps = deps or []
    return FlatBuffersInfo(
        direct_sources = srcs,
        direct_schemas = schemas,
        transitive_sources = depset(
            direct = srcs,
            transitive = [dep[FlatBuffersInfo].transitive_sources for dep in deps],
        ),
        transitive_schemas = depset(
            direct = schemas,
            transitive = [dep[FlatBuffersInfo].transitive_schemas for dep in deps],
        ),
    )

#################################### attrs #####################################

_flatc_attr = {
    "_flatc": attr.label(
        default = Label("//:flatc"),
        executable = True,
        cfg = "exec",
    ),
}

################################### actions ####################################

def _compile(*, ctx, srcs, deps = None):
    """Emits an action that triggers the compilation of the provided .fbs files.

    Args:
        ctx: Starlark context that is used to emit actions.
        srcs: a list of .fbs files to compile.
        deps: an optional list of targets that provide FlatBuffersInfo.

    Returns:
        FlatBuffersInfo that contains the result of compiling srcs.
    """
    deps = deps or []
    transitive_sources = depset(
        direct = srcs,
        transitive = [dep[FlatBuffersInfo].transitive_sources for dep in deps],
    )

    generated_schemas = []
    for src in srcs:
        schema = ctx.actions.declare_file(paths.replace_extension(src.basename, "") + ".bfbs")
        generated_schemas.append(schema)

    args = ctx.actions.args()
    args.add("--binary")
    args.add("--schema")
    args.add("-I", ".")
    args.add("-I", ctx.bin_dir.path)
    args.add("-I", ctx.genfiles_dir.path)
    args.add("-o", paths.join(ctx.bin_dir.path, ctx.label.package))
    args.add_all(srcs)

    ctx.actions.run(
        executable = ctx.executable._flatc,
        inputs = transitive_sources,
        outputs = generated_schemas,
        arguments = [args],
        progress_message = "Generating schemas for {0}".format(ctx.label),
        mnemonic = "CompileFlatBuffers",
    )

    return _create_flatbuffers_info(
        srcs = srcs,
        schemas = generated_schemas,
        deps = deps,
    )

#################################### rules #####################################

def _flatbuffers_library_impl(ctx):
    flatbuffers_info = _compile(
        ctx = ctx,
        srcs = ctx.files.srcs,
        deps = ctx.attr.deps,
    )

    return [
        flatbuffers_info,
        DefaultInfo(
            files = depset(flatbuffers_info.direct_schemas),
            runfiles = ctx.runfiles(files = flatbuffers_info.direct_schemas),
        ),
    ]

flatbuffers_library = rule(
    doc = """\
Use `flatbuffers_library` to define libraries of FlatBuffers which may be used from multiple
languages. A `flatbuffers_library` may be used in `deps` of language-specific rules, such as
`cc_flatbuffers_library`.

A `flatbuffers_library` can also be used in `data` for any supported target. In this case, the
binary serialized schema (i.e. `.bfbs`) for files directly mentioned by a `flatbuffers_library`
target will be provided to the target at runtime.

The code should be organized in the following way:

-  one `flatbuffers_library` target per `.fbs` file;
-  a file named `foo.fbs` should be the only source for a target named `foo_fbs`, which is located
   in the same package;
-  a `[language]_flatbuffers_library` that wraps a `flatbuffers_library` named `foo_fbs` should be
   called `foo_[language]_fbs`, and be located in the same package.

Example:

```build
load("@flatbuffers//:flatbuffers.bzl", "cc_flatbuffers_library", "flatbuffers_library")

flatbuffers_library(
    name = "bar_fbs",
    srcs = ["bar.fbs"],
)

flatbuffers_library(
    name = "foo_fbs",
    srcs = ["foo.fbs"],
    deps = [":bar_fbs"],
)

cc_flatbuffers_library(
    name = "foo_cc_fbs",
    deps = [":foo_fbs"],
)
```

The following rules provide language-specific implementation of FlatBuffers:

-  `cc_flatbuffers_library`
""",
    attrs = {
        "srcs": attr.label_list(
            allow_files = [".fbs"],
        ),
        "deps": attr.label_list(
            providers = [FlatBuffersInfo],
        ),
    } | _flatc_attr,
    provides = [FlatBuffersInfo],
    implementation = _flatbuffers_library_impl,
)

def _cc_flatbuffers_aspect_impl(target, ctx):
    if CcInfo in target:  # target already provides CcInfo.
        return []

    filename_suffix = ".fbs"

    hdrs = [
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

    args.add("--gen-object-api")
    args.add("--gen-compare")
    args.add("--gen-mutable")
    args.add("--reflect-names")

    args.add_all(target[FlatBuffersInfo].direct_sources)

    ctx.actions.run(
        executable = ctx.executable._flatc,
        inputs = target[FlatBuffersInfo].transitive_sources,
        outputs = hdrs,
        arguments = [args],
        progress_message = "Generating FlatBuffers C++ code for {0}".format(ctx.label),
        mnemonic = "GenerateFlatBuffersCc",
    )

    cc_toolchain = find_cc_toolchain(ctx)
    feature_configuration = cc_common.configure_features(
        ctx = ctx,
        cc_toolchain = cc_toolchain,
        requested_features = ctx.features + ["parse_headers"],
        unsupported_features = ctx.disabled_features,
    )

    deps = getattr(ctx.rule.attr, "deps", [])
    compilation_contexts = [dep[CcInfo].compilation_context for dep in [ctx.attr._runtime] + deps]

    compilation_context, _ = cc_common.compile(
        name = ctx.label.name,
        actions = ctx.actions,
        feature_configuration = feature_configuration,
        cc_toolchain = cc_toolchain,
        public_hdrs = hdrs,
        compilation_contexts = compilation_contexts,
    )

    return [
        CcInfo(
            compilation_context = compilation_context,
        ),
        OutputGroupInfo(
            srcs = depset(hdrs),
        ),
    ]

_cc_flatbuffers_aspect = aspect(
    attrs = {
        "_runtime": attr.label(
            default = Label("//:runtime_cc"),
            providers = [CcInfo],
        ),
    } | _flatc_attr,
    attr_aspects = ["deps"],
    fragments = ["cpp"],
    implementation = _cc_flatbuffers_aspect_impl,
    toolchains = use_cc_toolchain(),
)

def _cc_flatbuffers_library_impl(ctx):
    if len(ctx.attr.deps) != 1:
        fail("deps requires exactly one target (got %d)" % len(ctx.attr.deps))

    dep = ctx.attr.deps[0]
    return [
        dep[CcInfo],
        dep[OutputGroupInfo],
    ]

cc_flatbuffers_library = rule(
    attrs = {
        "deps": attr.label_list(
            providers = [FlatBuffersInfo],
            aspects = [_cc_flatbuffers_aspect],
        ),
    },
    doc = """\
`cc_flatbuffers_library` generates C++ code from `.fbs` files.

`cc_flatbuffers_library` does:

-  generate additional object-based API;
-  generate operator== for object-based API types;
-  generate accessors that can mutate buffers in-place;
-  add minimal type/name reflection.

Example:

```build
load("@flatbuffers//:flatbuffers.bzl", "cc_flatbuffers_library", "flatbuffers_library")

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

Where `foo_user.h` would include the generated code via: `#include "path/to/project/foo.fbs.h"`.
""",
    implementation = _cc_flatbuffers_library_impl,
)

PyFlatbuffersInfo = provider(
    doc = "An internal provider used to pass information from py_flatbuffers_aspect to py_flatbuffers_library.",
    fields = {
        "direct_py_sources": "A list of all .py files generated for this flatbuffers_library.",
        "transitive_py_sources": "A transitive closure of all .py files generated for this library all all its dependencies.",
        "direct_pyi_sources": "A list of all .pyi files generated for this flatbuffers_library.",
        "transitive_pyi_sources": "A transitive closure of all .pyi files generated for this library all all its dependencies.",
        "cc_info": "A CcInfo provider that propagates the compilation and linking context of the transitive C++ dependencies.",
        "transitive_runfiles": "A list of runfiles for this library and all its dependencies.",
        "imports": "A depset of import path strings to be added to the `PYTHONPATH` of executable Python targets.",
    },
)

def _py_flatbuffers_aspect_impl(target, ctx):
    if PyInfo in target:  # target already provides PyInfo.
        return []

    direct_py_sources = [
        ctx.actions.declare_file(paths.replace_extension(src.basename, "_fbs.py"))
        for src in target[FlatBuffersInfo].direct_sources
    ]
    direct_pyi_sources = [
        ctx.actions.declare_file(paths.replace_extension(src.basename, "_fbs.pyi"))
        for src in target[FlatBuffersInfo].direct_sources
    ]

    args = ctx.actions.args()
    args.add("-c")
    args.add("-I", paths.join(".", ctx.label.workspace_root))
    args.add("-I", ".")
    args.add("-I", ctx.label.package)
    args.add("-I", ctx.bin_dir.path)
    args.add("-I", paths.join(ctx.bin_dir.path, ctx.label.workspace_root))
    args.add("-I", ctx.genfiles_dir.path)
    args.add("-o", paths.join(ctx.bin_dir.path, ctx.label.workspace_root, ctx.label.package))
    args.add("--gen-onefile")
    args.add("--gen-compare")
    args.add("--gen-object-api")
    args.add("--filename-suffix", "_fbs")
    args.add("--python")
    args.add("--python-typing")

    args.add("--python-version", "3")
    args.add("--python-gen-numpy")

    args.add_all(target[FlatBuffersInfo].direct_sources)

    ctx.actions.run(
        arguments = [args],
        executable = ctx.executable._flatc,
        inputs = target[FlatBuffersInfo].transitive_sources,
        outputs = direct_py_sources + direct_pyi_sources,
        progress_message = "Generating FlatBuffers Python code for {0}".format(ctx.label),
        mnemonic = "GenerateFlatBuffersPy",
    )

    runtime = ctx.attr._runtime[PyInfo]
    transitive_py_sources = [runtime.transitive_sources]
    transitive_pyi_sources = [runtime.transitive_pyi_files]
    for dep in getattr(ctx.rule.attr, "deps", []):
        transitive_py_sources.append(dep[PyFlatbuffersInfo].transitive_py_sources)
        transitive_pyi_sources.append(dep[PyFlatbuffersInfo].transitive_pyi_sources)

    return [
        PyFlatbuffersInfo(
            direct_py_sources = direct_py_sources,
            transitive_py_sources = depset(
                direct = direct_py_sources,
                transitive = transitive_py_sources,
            ),
            direct_pyi_sources = direct_pyi_sources,
            transitive_pyi_sources = depset(
                direct = direct_pyi_sources,
                transitive = transitive_pyi_sources,
            ),
            cc_info = ctx.attr._runtime[PyCcLinkParamsInfo].cc_info,
            transitive_runfiles = [
                ctx.runfiles(
                    files = direct_py_sources,
                    transitive_files = depset(transitive = transitive_py_sources),
                    collect_default = True,
                ),
                ctx.attr._runtime[DefaultInfo].default_runfiles,
                ctx.attr._runtime[DefaultInfo].data_runfiles,
            ],
            imports = runtime.imports,
        ),
        OutputGroupInfo(
            srcs = depset(direct_py_sources + direct_pyi_sources),
        ),
    ]

_py_flatbuffers_aspect = aspect(
    attrs = {
        "_runtime": attr.label(
            default = Label("//python:flatbuffers"),
            providers = [PyInfo],
        ),
    } | _flatc_attr,
    attr_aspects = ["deps"],
    fragments = ["py"],
    implementation = _py_flatbuffers_aspect_impl,
)

def _py_flatbuffers_library_impl(ctx):
    if len(ctx.attr.deps) != 1:
        fail("deps requires exactly one target (got %d)" % len(ctx.attr.deps))

    py_flatbuffers_info = ctx.attr.deps[0][PyFlatbuffersInfo]
    return [
        PyInfo(
            transitive_sources = py_flatbuffers_info.transitive_py_sources,
            direct_original_sources = depset(py_flatbuffers_info.direct_py_sources),
            transitive_original_sources = py_flatbuffers_info.transitive_py_sources,
            direct_pyi_files = depset(py_flatbuffers_info.direct_pyi_sources),
            transitive_pyi_files = py_flatbuffers_info.transitive_pyi_sources,
            imports = py_flatbuffers_info.imports,
        ),
        PyCcLinkParamsInfo(
            cc_info = py_flatbuffers_info.cc_info,
        ),
        DefaultInfo(
            files = py_flatbuffers_info.transitive_py_sources,
            runfiles = ctx.runfiles(
                transitive_files = py_flatbuffers_info.transitive_py_sources,
                collect_default = True,
            ).merge_all(py_flatbuffers_info.transitive_runfiles),
        ),
    ]

py_flatbuffers_library = rule(
    attrs = {
        "deps": attr.label_list(
            aspects = [_py_flatbuffers_aspect],
            providers = [FlatBuffersInfo],
        ),
    },
    doc = """\
`py_flatbuffers_library` generates Python code from `.fbs` files.

Example:

```build
load("@flatbuffers//:flatbuffers.bzl", "py_flatbuffers_library", "flatbuffers_library")

flatbuffers_library(
    name = "foo_fbs",
    srcs = ["foo.fbs"],
)

py_flatbuffers_library(
    name = "foo_py_fbs",
    deps = [":foo_fbs"],
)

# An example library that uses the generated Python code from `foo_py_fbs`.
py_library(
    name = "foo_user",
    hdrs = ["foo_user.py"],
    deps = [":foo_py_fbs"],
)
```

Where `foo_user.py` would include the generated code via: `from path.to.project import foo_fbs`.
""",
    provides = [PyInfo],
    implementation = _py_flatbuffers_library_impl,
)

flatbuffers_common = struct(
    actions = struct(
        compile = _compile,
    ),
    attrs = _flatc_attr,
)
