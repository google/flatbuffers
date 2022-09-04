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


class TsTests():

    def Base(self):
        # Generate just foo with no extra arguments
        flatc(["--ts", "foo.fbs"])

        # Should generate the module that exports both foo and its direct
        # include, bar.
        assert_file_and_contents(
            "foo_generated.ts",
            [
                "export { Bar } from './bar.js';",
                "export { Foo } from './foo.js';",
            ],
        )

        # Foo should be generated in place and exports the Foo table.
        assert_file_and_contents("foo.ts", "export class Foo {")

        # Included files, like bar, should not be generated.
        assert_file_doesnt_exists("bar.ts")

    def BaseMultipleFiles(self):
        # Generate both foo and bar with no extra arguments
        flatc(["--ts", "foo.fbs", "bar/bar.fbs"])

        # Should generate the module that exports both foo and its direct
        # include, bar.
        assert_file_and_contents(
            "foo_generated.ts",
            [
                "export { Bar } from './bar.js';",
                "export { Foo } from './foo.js';",
            ],
        )

        # Foo should be generated in place and exports the Foo table.
        assert_file_and_contents("foo.ts", "export class Foo {")

        # Bar should also be generatd in place and exports the Bar table.
        assert_file_and_contents("bar.ts", "export class Bar {")

    def BaseWithNamespace(self):
        # Generate foo with namespacing, with no extra arguments
        flatc(["--ts", "foo_with_ns.fbs"])

        # Should generate the module that exports both foo in its namespace
        # directory and its direct include, bar.
        assert_file_and_contents(
            "foo_with_ns_generated.ts",
            [
                "export { Bar } from './bar/bar.js';",
                "export { Foo } from './something/foo.js';",
            ],
        )

        # Foo should be placed in the namespaced directory. It should export
        # Foo, and the import of Bar should be relative to its location.
        assert_file_and_contents(
            "something/foo.ts",
            [
                "export class Foo {",
                "import { Bar } from '../bar/bar.js';",
            ],
        )

        # Included files, like bar, should not be generated.
        assert_file_doesnt_exists("bar.ts")

    def GenAll(self):
        # Generate foo with generate all options
        flatc(["--ts", "--gen-all", "foo.fbs"])

        # Should generate a single file that exports all the generated types.
        assert_file_and_contents(
            "foo_generated.ts",
            [
                "export { Bar } from './bar.js'",
                "export { Baz } from './baz.js'",
                "export { Foo } from './foo.js'",
            ],
        )

        # Foo should be generated with an import to Bar and an export of itself.
        assert_file_and_contents(
            "foo.ts",
            [
                "import { Bar } from './bar.js';",
                "export class Foo {",
            ],
        )

        # Bar should be generated with an import to Baz and an export of itself.
        assert_file_and_contents(
            "bar.ts",
            [
                "import { Baz } from './baz.js';",
                "export class Bar {",
            ],
        )

        # Baz should be generated with an export of itself.
        assert_file_and_contents(
            "baz.ts",
            [
                "export enum Baz {",
            ],
        )


    def FlatFiles(self):
        # Generate just foo with the flat files option
        flatc(["--ts", "--ts-flat-files", "foo.fbs"])

        # Should generate a single file that imports bar as a single file, and
        # exports the Foo table.
        assert_file_and_contents(
            "foo_generated.ts",
            [
                "import {Bar as Bar} from './bar_generated.js';",
                "export class Foo {",
            ],
        )

        # The root type Foo should not be generated in its own file.
        assert_file_doesnt_exists("foo.ts")

    def FlatFilesWithNamespace(self):
        # Generate just foo with the flat files option
        flatc(["--ts", "--ts-flat-files", "foo_with_ns.fbs"])

        # Should generate a single file that imports bar as a single file, and
        # exports the Foo table.
        assert_file_and_contents(
            "foo_with_ns_generated.ts",
            [
                "import {Bar as Bar} from './bar_with_ns_generated.js';",
                "export class Foo {",
            ],
        )

        # The root type Foo should not be generated in its own file.
        assert_file_doesnt_exists("foo.ts")

    def FlatFilesMultipleFiles(self):
        # Generate both foo and bar with the flat files option
        flatc(["--ts", "--ts-flat-files", "foo.fbs", "bar/bar.fbs"])

        # Should generate a single foo file that imports bar as a single file,
        # and exports the Foo table.
        assert_file_and_contents(
            "foo_generated.ts",
            [
                "import {Bar as Bar} from './bar_generated.js';",
                "export class Foo {",
            ],
        )

        # Should generate a single bar file that imports bar as a single file,
        # and exports the Bar table.
        assert_file_and_contents(
            "bar_generated.ts",
            [
                "import {Baz as Baz} from './baz_generated.js';",
                "export class Bar {",
            ],
        )

        # The types Foo and Bar should not be generated in their own files
        assert_file_doesnt_exists("foo.ts")
        assert_file_doesnt_exists("bar.ts")

    def FlatFilesGenAll(self):
        # Generate foo with all of its dependents with the flat files option
        flatc(["--ts", "--ts-flat-files", "--gen-all", "foo.fbs"])

        # Should generate a single foo file
        assert_file_and_contents(
            "foo_generated.ts",
            # Should export each of the types within the single file
            [
                "export class Foo {",
                "export class Bar {",
                "export enum Baz {",
            ],
            # No includes for the dependent types should be present.
            doesnt_contain=[
                "import {Bar as Bar}",
                "import {Baz as Baz}",
            ],
        )

        # The types Foo, Bar and Baz should not be generated in their own files.
        assert_file_doesnt_exists("foo.ts")
        assert_file_doesnt_exists("bar.ts")
        assert_file_doesnt_exists("baz.ts")


    def ZFlatFilesGenAllWithNamespacing(self):
        # Generate foo with all of its dependents with the flat files option
        flatc(["--ts", "--ts-flat-files", "--gen-all", "foo_with_ns.fbs"])

        # Should generate a single foo file
        assert_file_and_contents(
            "foo_with_ns_generated.ts",
            # Should export each of the types within the single file
            [
                "export class bar_Bar {",
                "export class bar_Foo {",
                "export enum Baz {",
                "export enum baz_Baz {",
                "export class something_Foo {"
            ],
            # No includes for the dependent types should be present.
            doesnt_contain=[
                "import {Bar as Bar}",
                "import {Baz as Baz}",
            ],
        )

        # The types Foo, Bar and Baz should not be generated in their own files.
        assert_file_doesnt_exists("foo.ts")
        assert_file_doesnt_exists("bar.ts")
        assert_file_doesnt_exists("baz.ts")

