#!/bin/sh
#
# Copyright 2016 Google Inc. All rights reserved.
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
../flatc --ts --no-fb-import --gen-mutable monster_test.fbs
../flatc -b monster_test.fbs unicode_test.json
npm install @types/flatbuffers
mv monster_test_generated.js monster_test_generated.js.bak
tsc monster_test_generated.ts
npm uninstall @types/flatbuffers
node JavaScriptTest
mv monster_test_generated.js.bak monster_test_generated.js
