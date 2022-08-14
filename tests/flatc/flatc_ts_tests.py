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
            ["export { Bar } from './bar';", "export { Foo } from './foo';"],
        )

        # Foo should be generated in place and exports the Foo table.
        assert_file_and_contents("foo.ts", "export class Foo {")

        # Included files, like bar, should not be generated.
        assert_file_doesnt_exists("bar.ts")

    def BaseWithNamespace(self):
        # Generate foo with namespacing, with no extra arguments
        flatc(["--ts", "foo_with_ns.fbs"])

        # Should generate the module that exports both foo in its namespace
        # directory and its direct include, bar.
        assert_file_and_contents(
            "foo_with_ns_generated.ts",
            ["export { Bar } from './bar';", "export { Foo } from './something/foo';"],
        )

        # Foo should be placed in the namespaced directory. It should export
        # Foo, and the import of Bar should be relative to its location.
        assert_file_and_contents(
            "something/foo.ts",
            ["export class Foo {", "import { Bar } from '../bar';"],
        )

        # Included files, like bar, should not be generated.
        assert_file_doesnt_exists("bar.ts")

    def FlatFiles(self):
        # Generate just foo the flat files option
        flatc(["--ts", "--ts-flat-files", "foo.fbs"])

        # Should generate a single file that imports bar as a single file, and]
        # exports the Foo table.
        assert_file_and_contents(
            "foo_generated.ts",
            ["import {Bar as Bar} from './bar_generated';", "export class Foo {"],
        )

        # The root type Foo should not be generated in its own file.
        assert_file_doesnt_exists("foo.ts")
