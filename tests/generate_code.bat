:: Copyright 2015 Google Inc. All rights reserved.
::
:: Licensed under the Apache License, Version 2.0 (the "License");
:: you may not use this file except in compliance with the License.
:: You may obtain a copy of the License at
::
::     http://www.apache.org/licenses/LICENSE-2.0
::
:: Unless required by applicable law or agreed to in writing, software
:: distributed under the License is distributed on an "AS IS" BASIS,
:: WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
:: See the License for the specific language governing permissions and
:: limitations under the License.

set buildtype=Release
if "%1"=="-b" set buildtype=%2

..\%buildtype%\flatc.exe --cpp --java --csharp --go --binary --python --js --ts --php --grpc --gen-mutable --reflect-names --gen-object-api --no-includes --cpp-ptr-type flatbuffers::unique_ptr --no-fb-import -I include_test monster_test.fbs monsterdata_test.json
..\%buildtype%\flatc.exe --cpp --java --csharp --go --binary --python --js --ts --php --gen-mutable --reflect-names --no-fb-import --cpp-ptr-type flatbuffers::unique_ptr  -o namespace_test namespace_test/namespace_test1.fbs namespace_test/namespace_test2.fbs
..\%buildtype%\flatc.exe --cpp --js --ts --php --gen-mutable --reflect-names --gen-object-api --cpp-ptr-type flatbuffers::unique_ptr -o union_vector ./union_vector/union_vector.fbs
..\%buildtype%\flatc.exe -b --schema --bfbs-comments -I include_test monster_test.fbs
..\%buildtype%\flatc.exe --jsonschema --schema -I include_test monster_test.fbs
cd ../samples
..\%buildtype%\flatc.exe --cpp --gen-mutable --reflect-names --gen-object-api --cpp-ptr-type flatbuffers::unique_ptr monster.fbs
cd ../reflection

cd ../tests