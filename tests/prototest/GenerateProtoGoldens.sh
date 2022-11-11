#!/bin/bash
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

pushd "$(dirname $0)" >/dev/null

./../../flatc --proto test.proto && mv test.fbs test_include.golden
./../../flatc --proto --gen-all test.proto && mv test.fbs test.golden
./../../flatc --proto --oneof-union test.proto && mv test.fbs test_union_include.golden
./../../flatc --proto --gen-all --oneof-union test.proto && mv test.fbs test_union.golden
./../../flatc --proto --gen-all --proto-namespace-suffix test_namespace_suffix test.proto && mv test.fbs test_suffix.golden
./../../flatc --proto --gen-all --proto-namespace-suffix test_namespace_suffix --oneof-union test.proto && mv test.fbs test_union_suffix.golden
