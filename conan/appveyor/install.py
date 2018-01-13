#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import pip
import sys


def is_tag():
    return os.getenv("APPVEYOR_REPO_TAG") == "true"


def install_requirements():
    pip.main(["install", "conan", "conan-package-tools"])


def main():
    if not is_tag():
        print("Skip install step. It's not TAG")
        sys.exit(0)
    install_requirements()


if __name__ == "__main__":
    main()
