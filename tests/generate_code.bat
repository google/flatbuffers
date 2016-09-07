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

..\%buildtype%\flatc.exe --cpp --java --csharp --go --binary --python --js --php --grpc --gen-mutable --gen-object-api --no-includes monster_test.fbs monsterdata_test.json
..\%buildtype%\flatc.exe --cpp --java --csharp --go --binary --python --js --php --gen-mutable -o namespace_test namespace_test\namespace_test1.fbs namespace_test\namespace_test2.fbs
..\%buildtype%\flatc.exe --binary --schema monster_test.fbs
