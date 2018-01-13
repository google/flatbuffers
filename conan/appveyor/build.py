#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys
import subprocess


def is_tag():
    return os.getenv("APPVEYOR_REPO_TAG") == "true"


def build_conan_package():
    subprocess.check_call(["python", os.path.join("conan", "build.py")])


def main():
    if not is_tag():
        print("Skip build step. It's not TAG")
        sys.exit(0)
    build_conan_package()


if __name__ == "__main__":
    main()
