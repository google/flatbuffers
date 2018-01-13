#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""Conan recipe package for Google FlatBuffers
"""
import os
import shutil
from conans import ConanFile, CMake, tools


class FlatbuffersConan(ConanFile):
    name = "flatbuffers"
    version = "1.9.0"
    license = "https://github.com/google/flatbuffers/blob/master/LICENSE.txt"
    url = "https://github.com/google/flatbuffers"
    description = "Memory Efficient Serialization Library"
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False]}
    default_options = "shared=False"
    generators = "cmake"
    exports = "LICENSE.txt"
    exports_sources = ["CMake/*", "include/*", "src/*", "grpc/*", "CMakeLists.txt", "conan/CMakeLists.txt"]

    def source(self):
        """Wrap the original CMake file to call conan_basic_setup
        """
        shutil.move("CMakeLists.txt", "CMakeListsOriginal.txt")
        shutil.move(os.path.join("conan", "CMakeLists.txt"), "CMakeLists.txt")

    def build(self):
        """Configure, build and install FlatBuffers using CMake.
        """
        cmake = CMake(self)
        cmake.definitions["FLATBUFFERS_BUILD_TESTS"] = False
        cmake.definitions["FLATBUFFERS_BUILD_SHAREDLIB"] = self.options.shared
        cmake.configure()
        cmake.build()
        cmake.install()

    def package(self):
        """Copy Flatbuffers' artifacts to package folder
        """
        self.copy(pattern="LICENSE.txt", dst="licenses")
        self.copy(pattern="flathash*", dst="bin", src="bin")
        self.copy(pattern="flatc*", dst="bin", src="bin")

    def package_info(self):
        """Collect built libraries names and solve flatc path.
        """
        self.cpp_info.libs = tools.collect_libs(self)
        self.env_info.PATH.append(os.path.join(self.package_folder, "bin"))
        self.user_info.flatc = os.path.join(self.package_folder, "bin", "flatc")
