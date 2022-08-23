# Copyright 2022 Google Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from flatc_test import *


class CppTests:
    def Flatten(self):
        # Generate just foo with a "flatten" import of bar.
        flatc(["--cpp", "foo.fbs"])

        # Foo should be generated in place and include bar flatten
        assert_file_and_contents("foo_generated.h", '#include "bar_generated.h"')

    def FlattenAbsolutePath(self):
        # Generate just foo with a "flatten" import of bar.
        flatc(["--cpp", make_absolute("foo.fbs")])

        # Foo should be generated in place and include bar flatten
        assert_file_and_contents("foo_generated.h", '#include "bar_generated.h"')

    def FlattenSubDirectory(self):
        # Generate just foo with a "flatten" import of bar.
        flatc(["--cpp", "bar/bar.fbs"])

        # Bar should be generated in place and include baz
        assert_file_and_contents("bar_generated.h", '#include "baz_generated.h"')

    def FlattenOutPath(self):
        # Generate just foo with a "flatten" import of bar.
        flatc(["--cpp", "-o", ".tmp", "foo.fbs"])

        # Foo should be generated in the out path and include bar flatten to the out path.
        assert_file_and_contents(".tmp/foo_generated.h", '#include "bar_generated.h"')

    def FlattenOutPathSuperDirectory(self):
        # Generate just foo with a "flatten" import of bar.
        flatc(["--cpp", "-o", "../.tmp", "foo.fbs"])

        # Foo should be generated in the out path and include bar flatten to the out path.
        assert_file_and_contents(
            "../.tmp/foo_generated.h", '#include "bar_generated.h"'
        )

    def FlattenOutPathSubDirectory(self):
        # Generate just foo with a "flatten" import of bar.
        flatc(["--cpp", "-o", ".tmp", "bar/bar.fbs"])

        # Bar should be generated in the out path and include baz flatten to the out path.
        assert_file_and_contents(".tmp/bar_generated.h", '#include "baz_generated.h"')

    def KeepPrefix(self):
        # Generate just foo with the import of bar keeping the prefix of where it is located.
        flatc(["--cpp", "--keep-prefix", "foo.fbs"])

        assert_file_and_contents("foo_generated.h", '#include "bar/bar_generated.h"')

    def KeepPrefixAbsolutePath(self):
        # Generate just foo with the import of bar keeping the prefix of where it is located.
        flatc(["--cpp", "--keep-prefix", make_absolute("foo.fbs")])

        assert_file_and_contents("foo_generated.h", '#include "bar/bar_generated.h"')

    def KeepPrefixSubDirectory(self):
        # Generate with the import of bar keeping the prefix of where it is located.
        flatc(["--cpp", "--keep-prefix", "bar/bar.fbs"])

        assert_file_and_contents("bar_generated.h", '#include "baz/baz_generated.h"')

    def KeepPrefixOutPath(self):
        # Generate just foo with the import of bar keeping the prefix of where it is located.
        flatc(["--cpp", "--keep-prefix", "-o", ".tmp", "foo.fbs"])

        assert_file_and_contents(
            ".tmp/foo_generated.h",
            '#include "bar/bar_generated.h"',
        )

    def KeepPrefixOutPathSubDirectory(self):
        # Generate with the import of bar keeping the prefix of where it is located.
        flatc(["--cpp", "--keep-prefix", "-o", ".tmp", "bar/bar.fbs"])

        assert_file_and_contents(
            ".tmp/bar_generated.h", '#include "baz/baz_generated.h"'
        )

    def IncludePrefix(self):
        # Generate just foo with the import of bar keeping the prefix of where it is located.
        flatc(["--cpp", "--include-prefix", "test", "foo.fbs"])

        assert_file_and_contents("foo_generated.h", '#include "test/bar_generated.h"')

    def IncludePrefixAbolutePath(self):
        # Generate just foo with the import of bar keeping the prefix of where it is located.
        flatc(["--cpp", "--include-prefix", "test", make_absolute("foo.fbs")])

        assert_file_and_contents("foo_generated.h", '#include "test/bar_generated.h"')

    def IncludePrefixSubDirectory(self):
        # Generate just foo with the import of bar keeping the prefix of where it is located.
        flatc(["--cpp", "--include-prefix", "test", "bar/bar.fbs"])

        assert_file_and_contents("bar_generated.h", '#include "test/baz_generated.h"')

    def IncludePrefixOutPath(self):
        # Generate just foo with the import of bar keeping the prefix of where it is located.
        flatc(["--cpp", "--include-prefix", "test", "-o", ".tmp", "foo.fbs"])

        assert_file_and_contents(
            ".tmp/foo_generated.h", '#include "test/bar_generated.h"'
        )

    def IncludePrefixOutPathSubDirectory(self):
        # Generate just foo with the import of bar keeping the prefix of where it is located.
        flatc(["--cpp", "--include-prefix", "test", "-o", ".tmp", "bar/bar.fbs"])

        assert_file_and_contents(
            ".tmp/bar_generated.h", '#include "test/baz_generated.h"'
        )

    def KeepPrefixIncludePrefix(self):
        # Generate just foo with the import of bar keeping the prefix of where it is located.
        flatc(["--cpp", "--keep-prefix", "--include-prefix", "test", "foo.fbs"])

        # The include prefix should come first, with the kept prefix next.
        assert_file_and_contents(
            "foo_generated.h", '#include "test/bar/bar_generated.h"'
        )

    def KeepPrefixIncludePrefixAbsolutePath(self):
        # Generate just foo with the import of bar keeping the prefix of where it is located.
        flatc(
            [
                "--cpp",
                "--keep-prefix",
                "--include-prefix",
                "test",
                make_absolute("foo.fbs"),
            ]
        )

        # The include prefix should come first, with the kept prefix next.
        assert_file_and_contents(
            "foo_generated.h", '#include "test/bar/bar_generated.h"'
        )

    def KeepPrefixIncludePrefixSubDirectory(self):
        # Generate just foo with the import of bar keeping the prefix of where it is located.
        flatc(["--cpp", "--keep-prefix", "--include-prefix", "test", "bar/bar.fbs"])

        # The include prefix should come first, with the kept prefix next.
        assert_file_and_contents(
            "bar_generated.h", '#include "test/baz/baz_generated.h"'
        )

    def KeepPrefixIncludePrefixOutPathSubDirectory(self):
        # Generate just foo with the import of bar keeping the prefix of where it is located.
        flatc(
            [
                "--cpp",
                "--keep-prefix",
                "--include-prefix",
                "test",
                "-o",
                ".tmp",
                "bar/bar.fbs",
            ]
        )

        # The include prefix should come first, with the kept prefix next.
        assert_file_and_contents(
            ".tmp/bar_generated.h", '#include "test/baz/baz_generated.h"'
        )

    def KeepPrefixIncludePrefixOutPathSuperDirectory(self):
        # Generate just foo with the import of bar keeping the prefix of where it is located.
        flatc(
            [
                "--cpp",
                "--keep-prefix",
                "--include-prefix",
                "test",
                "-o",
                "../.tmp",
                "bar/bar.fbs",
            ]
        )

        # The include prefix should come first, with the kept prefix next.
        assert_file_and_contents(
            "../.tmp/bar_generated.h", '#include "test/baz/baz_generated.h"'
        )

    def KeepPrefixIncludePrefixoutPathAbsoluePaths_SuperDirectoryReference(self):
        # Generate bar_with_foo that references a type in a super directory.
        flatc(
            [
                "--cpp",
                "--keep-prefix",
                "--include-prefix",
                "generated",
                "-I",
                str(script_path.absolute()),
                "-o",
                str(Path(script_path, ".tmp").absolute()),
                str(Path(script_path, "bar/bar_with_foo.fbs").absolute()),
            ]
        )

        # The include prefix should come first, with the kept prefix next.
        assert_file_and_contents(
            ".tmp/bar_with_foo_generated.h",
            [
                '#include "generated/baz/baz_generated.h"',
                '#include "generated/foo_generated.h"',
            ],
        )

    def KeepPrefixIncludePrefixoutPath_SuperDirectoryReference(self):
        # Generate bar_with_foo that references a type in a super directory.
        flatc(
            [
                "--cpp",
                "--keep-prefix",
                "--include-prefix",
                "generated",
                "-I",
                "./",
                "-o",
                ".tmp",
                "bar/bar_with_foo.fbs",
            ]
        )

        # The include prefix should come first, with the kept prefix next.
        assert_file_and_contents(
            ".tmp/bar_with_foo_generated.h",
            [
                '#include "generated/baz/baz_generated.h"',
                '#include "generated/foo_generated.h"',
            ],
            unlink=False,
        )
