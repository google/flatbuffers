#!/usr/bin/env python
# -*- coding: utf-8 -*-

from conan.packager import ConanMultiPackager
import os, re


def get_value_from_recipe(search_string):
    with open("conanfile.py", "r") as conanfile:
        contents = conanfile.read()
        result = re.search(search_string, contents)
    return result


def get_name_from_recipe():
    return get_value_from_recipe(r'''name\s*=\s*["'](\S*)["']''').groups()[0]


def get_version_from_recipe():
    return get_value_from_recipe(r'''version\s*=\s*["'](\S*)["']''').groups()[0]


def running_on_appveyor():
    return os.getenv("APPVEYOR", False)


def get_env_vars():
    username = os.getenv("CONAN_USERNAME", "flatbuffers")
    channel = os.getenv("CONAN_CHANNEL", "testing")
    name = get_name_from_recipe()
    version = get_version_from_recipe()
    return name, version, username, channel


def get_stable_branch_pattern():
    return "master" if running_on_appveyor() else r"v\d+\.\d+\.\d+"


def set_appveyor_environment():
    compiler_version = os.getenv("CMAKE_VS_VERSION").split(" ")[0].replace('"', '')
    os.environ["CONAN_VISUAL_VERSIONS"] = compiler_version

    ci_platform = os.getenv("Platform").replace('"', '')
    ci_platform = "x86" if ci_platform == "x86" else "x86_64"
    os.environ["CONAN_ARCHS"] = ci_platform

    os.environ["CONAN_BUILD_TYPES"] = os.getenv("Configuration").replace('"', '')


def add_visual_build(builder, version, arch, build_type, runtime, shared):
    settings = {'compiler.version': version, 'arch': arch, 'build_type': build_type,
                'compiler.runtime': '%s%s' % (runtime, ('d' if build_type == "Debug" else '')),
                'compiler': 'Visual Studio'}
    options= {'flatbuffers:shared': shared}
    builder.add(settings=settings, options=options, env_vars={}, build_requires={})
    return builder


def add_extra_appveyor_builds(builder):
    if os.getenv("CONAN_ARCHS") == "x86_64" and os.getenv("CONAN_VISUAL_VERSIONS") == "10":
        build_type = os.getenv("CONAN_BUILD_TYPES")
        builder = add_visual_build(builder, '10', 'x86_64', build_type, 'MT', False)
        builder = add_visual_build(builder, '10', 'x86_64', build_type, 'MD', False)
        builder = add_visual_build(builder, '10', 'x86_64', build_type, 'MD', True)
    return builder


if __name__ == "__main__":
    name, version, username, channel = get_env_vars()
    reference = "{0}/{1}".format(name, version)
    upload = "https://api.bintray.com/conan/{0}/conan".format(username)
    stable_branch_pattern = get_stable_branch_pattern()
    test_folder = "-tf %s" % os.path.join("conan", "test_package")

    if running_on_appveyor():
        set_appveyor_environment()

    builder = ConanMultiPackager(
        args=test_folder,
        username=username,
        channel=channel,
        reference=reference,
        upload=upload,
        remotes=upload,
        upload_only_when_stable=True,
        stable_branch_pattern=stable_branch_pattern)

    builder.add_common_builds(shared_option_name="%s:shared" % name, pure_c=False)

    if running_on_appveyor():
        builder = add_extra_appveyor_builds(builder)

    builder.run()
