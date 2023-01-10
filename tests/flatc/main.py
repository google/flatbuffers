#!/usr/bin/env python3
#
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

import sys

from flatc_test import run_all
from flatc_cpp_tests import CppTests
from flatc_kotlin_tests import KotlinTests
from flatc_ts_tests import TsTests
from flatc_schema_tests import SchemaTests

passing, failing = run_all(CppTests, KotlinTests, TsTests, SchemaTests)

print("")
print("{0} of {1} tests passed".format(passing, passing + failing))

if failing > 0:
    sys.exit(1)
