#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""Conan recipe package for Google FlatBuffers
"""
import os
from conans import ConanFile, CMake, tools


class FlatbuffersConan(ConanFile):
    name = "flatbuffers"
    version = "1.9.0"
    license = "https://github.com/google/flatbuffers/blob/master/LICENSE.txt"
    url = "https://github.com/google/flatbuffers"
    description = "Memory Efficient Serialization Library"
    settings = "os", "compiler", "build_type", "arch", "os_build", "arch_build"
    options = {"shared": [True, False]}
    default_options = "shared=False"
    generators = "cmake"
    exports = "LICENSE.txt"
    exports_sources = ["CMake/*", "include/*", "src/*", "grpc/*", "CMakeLists.txt"]

    def _inject_magic_lines(self):
        """Inject Conan setup in cmake file to solve exteral dependencies.
        """
        conan_magic_lines = '''project(FlatBuffers)
        include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
        conan_basic_setup()
        '''
        tools.replace_in_file("CMakeLists.txt", "project(FlatBuffers)", conan_magic_lines)

    def build(self):
        """Configure, build and install FlatBuffers using CMake.
        """
        self._inject_magic_lines()
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

    def package_info(self):
        """Collect built libraries names and solve flatc path.
        """
        self.cpp_info.libs = tools.collect_libs(self)
        self.env_info.PATH.append(os.path.join(self.package_folder, "bin"))
