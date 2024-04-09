"""Starlark rules for FlatBuffers."""

load("@bazel_skylib//lib:types.bzl", "types")

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

FlatBuffersInfo, _ = provider(
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

def _merge_flatbuffers_infos(infos):
    return FlatBuffersInfo(
        transitive_sources = depset(
            transitive = [info.transitive_sources for info in infos],
        ),
        transitive_schemas = depset(
            transitive = [info.transitive_schemas for info in infos],
        ),
    )

flatbuffers_common = struct(
    providers = struct(
        create_flatbuffers_info = _create_flatbuffers_info,
        merge_flatbuffers_infos = _merge_flatbuffers_infos,
    ),
)
