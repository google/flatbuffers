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


def get_env_vars():
    username = os.getenv("CONAN_USERNAME", "flatbuffers")
    channel = os.getenv("CONAN_CHANNEL", "testing")
    name = get_name_from_recipe()
    version = get_version_from_recipe()
    return name, version, username, channel


if __name__ == "__main__":
    name, version, username, channel = get_env_vars()
    reference = "{0}/{1}".format(name, version)
    upload = "https://api.bintray.com/conan/{0}/conan".format(username)

    builder = ConanMultiPackager(
        args="-tf %s" % os.path.join("conan", "test_package"),
        username=username,
        channel=channel,
        reference=reference,
        upload=upload,
        remotes=upload,
        upload_only_when_stable=True,
        stable_branch_pattern=r"v\d+\.\d+\.\d+")

    builder.add_common_builds(shared_option_name="%s:shared" % name)
    builder.run()
