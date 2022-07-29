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

set -e

# clean node_modules to make sure we depend on latest local flatbuffers at ../
rm -rf node_modules
npm install

if [ -x ../flatc ]; then
    ../flatc --ts --reflect-names --gen-name-strings --gen-mutable --gen-object-api -I include_test monster_test.fbs
    ../flatc --ts --reflect-names --gen-name-strings --gen-mutable --gen-object-api -b -I include_test monster_test.fbs unicode_test.json
    ../flatc --ts --reflect-names --gen-name-strings --gen-mutable --gen-object-api -o union_vector union_vector/union_vector.fbs
    ../flatc --ts --gen-name-strings optional_scalars.fbs
    ../flatc --ts --reflect-names --gen-name-strings --gen-mutable --gen-object-api -I ../ ./typescript_keywords.fbs test_dir/typescript_include.fbs test_dir/typescript_transitive_include.fbs ../reflection/reflection.fbs
    ../flatc --ts --reflect-names --gen-name-strings --gen-mutable --gen-object-api --ts-flat-files -I ../ ./typescript_keywords.fbs test_dir/typescript_include.fbs test_dir/typescript_transitive_include.fbs ../reflection/reflection.fbs
fi
tsc
node JavaScriptTest
node JavaScriptUnionVectorTest
node JavaScriptFlexBuffersTest
