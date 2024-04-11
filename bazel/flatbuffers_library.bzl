"""Starlark rules for FlatBuffers."""

load("@bazel_skylib//lib:dicts.bzl", "dicts")
load("@bazel_skylib//lib:paths.bzl", "paths")
load("@bazel_skylib//lib:types.bzl", "types")

def _init_flatbuffers_info(*, direct_sources = [], direct_schemas = [], direct_includes = [], transitive_sources = depset(), transitive_schemas = depset(), transitive_includes = depset()):
    """_init_flatbuffers_info is a public constructor for FlatBuffersInfo."""
    if not types.is_list(direct_sources):
        fail("direct_sources must be a list (got %s)" % type(direct_sources))

    if not types.is_list(direct_schemas):
        fail("direct_schemas must be a list (got %s)" % type(direct_schemas))

    if not types.is_list(direct_includes):
        fail("direct_includes must be a list (got %s)" % type(direct_includes))

    if not types.is_depset(transitive_sources):
        fail("transitive_sources must be a depset (got %s)" % type(transitive_sources))

    if not types.is_depset(transitive_schemas):
        fail("transitive_schemas must be a depset (got %s)" % type(transitive_schemas))

    if not types.is_depset(transitive_includes):
        fail("transitive_includes must be a depset (got %s)" % type(transitive_includes))

    return {
        "direct_sources": direct_sources,
        "direct_schemas": direct_schemas,
        "direct_includes": direct_includes,
        "transitive_sources": transitive_sources,
        "transitive_schemas": transitive_schemas,
        "transitive_includes": transitive_includes,
    }

FlatBuffersInfo, _ = provider(
    doc = "Encapsulates information provided by flatbuffers_library.",
    fields = {
        "direct_sources": "FlatBuffers sources (i.e. .fbs) from the \"srcs\" attribute that contain text-based schema.",
        "direct_schemas": "The binary serialized schema files (i.e. .bfbs) of the direct sources.",
        "transitive_sources": "FlatBuffers sources (i.e. .fbs) for this and all its dependent FlatBuffers targets.",
        "transitive_schemas": "A set of binary serialized schema files (i.e. .bfbs) for this and all its dependent FlatBuffers targets.",
        "direct_includes": "extra include dirs",
        "transitive_includes": "extra include dirs",
    },
    init = _init_flatbuffers_info,
)

def _create_flatbuffers_info(*, srcs, schemas, deps = None, includes = None):
    deps = deps or []
    includes = includes or []
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
        direct_includes = includes,
        transitive_includes = depset(direct = includes, transitive = [dep[FlatBuffersInfo].transitive_includes for dep in deps]),
    )

def _merge_flatbuffers_infos(infos):
    return FlatBuffersInfo(
        transitive_sources = depset(
            transitive = [info.transitive_sources for info in infos],
        ),
        transitive_schemas = depset(
            transitive = [info.transitive_schemas for info in infos],
        ),
        transitive_includes = depset(
            transitive = [info.transitive_includes for info in infos],
        ),
    )

_flatc = {
    "_flatc": attr.label(
        default = Label("@//:flatc"),
        executable = True,
        cfg = "exec",
    ),
}

def _emit_compile(*, ctx, srcs, deps = None, includes = None):
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

    includes = includes or []
    transitive_includes = depset()

    args = ctx.actions.args()
    args.add("--binary")
    args.add("--schema")
    args.add("-I", ".")

    args.add_all(includes, before_each = "-I")

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
    )

    return _create_flatbuffers_info(
        srcs = srcs,
        schemas = generated_schemas,
        deps = deps,
    )

def _flatbuffers_library_impl(ctx):
    flatbuffers_info = _emit_compile(
        ctx = ctx,
        srcs = ctx.files.srcs,
        deps = ctx.attr.deps,
        includes = ctx.attr.includes,
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
load("//third_party/flatbuffers:flatbuffers.bzl", "cc_flatbuffers_library", "flatbuffers_library")

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
-  `cc_lite_flatbuffers_library`
-  `kt_flatbuffers_library`""",
    attrs = dicts.add({
        "srcs": attr.label_list(
            allow_files = [".fbs"],
        ),
        "deps": attr.label_list(
            providers = [FlatBuffersInfo],
        ),
        "includes": attr.string_list(),
    }, _flatc),
    provides = [FlatBuffersInfo],
    implementation = _flatbuffers_library_impl,
)

flatbuffers_common = struct(
    actions = struct(
        compile = _emit_compile,
    ),
    attrs = dicts.add(_flatc),
    providers = struct(
        create_flatbuffers_info = _create_flatbuffers_info,
        merge_flatbuffers_infos = _merge_flatbuffers_infos,
    ),
)
