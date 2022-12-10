"""
Rules for building typescript flatbuffers with Bazel.
"""

load("@build_bazel_rules_nodejs//:index.bzl", "js_library")
load("@npm//@bazel/typescript:index.bzl", "ts_project")
load(":build_defs.bzl", "DEFAULT_INCLUDE_PATHS", "flatbuffer_library_public")

DEFAULT_FLATC_TS_ARGS = [
    "--gen-object-api",
    "--gen-mutable",
    "--reflect-names",
    "--gen-name-strings",
    "--ts-flat-files",
    "--keep-prefix",
]

def flatbuffer_ts_library(
        name,
        srcs,
        compatible_with = None,
        target_compatible_with = None,
        deps = [],
        include_paths = DEFAULT_INCLUDE_PATHS,
        flatc_args = DEFAULT_FLATC_TS_ARGS,
        visibility = None,
        restricted_to = None,
        include_reflection = True,
        gen_reflections = False,
        package_name = None):
    """Generates a ts_library rule for a given flatbuffer definition.

    Args:
      name: Name of the generated ts_library rule.
      srcs: Source .fbs file(s).
      deps: Other flatbuffer_ts_library's to depend on. Note that currently
            you must specify all your transitive dependencies manually.
      include_paths: Optional, list of paths the includes files can be found in.
      flatc_args: Optional list of additional arguments to pass to flatc
          (e.g. --gen-mutable).
      visibility: The visibility of the generated cc_library. By default, use the
          default visibility of the project.
      compatible_with: Optional, The list of environments this rule can be built
        for, in addition to default-supported environments.
      restricted_to: Optional, The list of environments this rule can be built
        for, instead of default-supported environments.
      target_compatible_with: Optional, The list of target platform constraints
        to use.
      include_reflection: Optional, Whether to depend on the flatbuffer
        reflection library automatically. Only really relevant for the
        target that builds the reflection library itself.
      gen_reflections: Optional, if true this will generate the flatbuffer
        reflection binaries for the schemas.
      package_name: Optional, Package name to use for the generated code.
    """
    srcs_lib = "%s_srcs" % (name)
    out_base = [s.replace(".fbs", "").split("/")[-1].split(":")[-1] for s in srcs]

    # Because of how we have to manage the bazel rules for typescript,
    # reflection has to get special-cased to get imported when
    # run within bazel. As such, generate the code using the _pregenerate
    # suffix; then do a find/replace to fix-up all the reflection imports.
    pre_outs = ["%s_pregenerated.ts" % s for s in out_base]
    outs = ["%s_generated.ts" % s for s in out_base]
    includes = [d + "_includes" for d in deps]
    reflection_name = "%s_reflection" % name if gen_reflections else ""
    flatbuffer_library_public(
        name = srcs_lib,
        srcs = srcs,
        outs = pre_outs,
        language_flag = "--ts",
        includes = includes,
        include_paths = include_paths,
        flatc_args = flatc_args + ["--filename-suffix _pregenerated"],
        compatible_with = compatible_with,
        restricted_to = restricted_to,
        reflection_name = reflection_name,
        reflection_visibility = visibility,
        target_compatible_with = target_compatible_with,
    )
    fix_import_cmd = " ".join([
        "SRCS=($(SRCS));",
        "OUTS=($(OUTS));",
        "for i in $${!SRCS[@]}; do",
        "sed \"s/'.*reflection\\/reflection_pregenerated/'flatbuffers_reflection\\/reflection_generated/; s/_pregenerated/_generated/\" $${SRCS[i]} > $${OUTS[i]};",
        "done",
    ])
    native.genrule(
        name = name + "_reimporter",
        srcs = pre_outs,
        outs = outs,
        cmd = fix_import_cmd,
    )
    ts_project(
        name = name + "_ts",
        srcs = outs,
        declaration = True,
        visibility = visibility,
        compatible_with = compatible_with,
        restricted_to = restricted_to,
        target_compatible_with = target_compatible_with,
        tsconfig = {
            "compilerOptions": {
                "declaration": True,
                "lib": [
                    "ES2015",
                    "ES2020.BigInt",
                    "DOM",
                ],
                "module": "commonjs",
                "moduleResolution": "node",
                "noUnusedLocals": True,
                "strict": True,
                "types": ["node"],
            },
        },
        deps = deps + ["@com_github_google_flatbuffers//ts:flatbuffers"] + (["@com_github_google_flatbuffers//reflection/ts:reflection_ts_fbs"] if include_reflection else []),
    )
    js_library(
        name = name,
        visibility = visibility,
        compatible_with = compatible_with,
        restricted_to = restricted_to,
        target_compatible_with = target_compatible_with,
        deps = [name + "_ts"],
        package_name = package_name,
    )
    native.filegroup(
        name = "%s_includes" % (name),
        srcs = srcs + includes,
        compatible_with = compatible_with,
        restricted_to = restricted_to,
        visibility = visibility,
    )
