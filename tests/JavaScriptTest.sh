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
../flatc -b -I include_test monster_test.fbs unicode_test.json
../flatc --js -o js --gen-name-strings --gen-mutable --no-fb-import -I include_test monster_test.fbs
node JavaScriptTest ./js/monster_test_generated

../flatc --js --gen-name-strings -o js --no-fb-import union_vector/union_vector.fbs
node JavaScriptUnionVectorTest ./js/union_vector_generated
node JavaScriptFlexBuffersTest
