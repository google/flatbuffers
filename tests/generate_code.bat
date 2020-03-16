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

@SETLOCAL

set buildtype=Release
if "%1"=="-b" set buildtype=%2

set commandline=%*


if NOT "%commandline%"=="%commandline:--cpp-std c++0x=%" (
  set TEST_CPP_FLAGS=--cpp-std c++0x
) else (
  @rem --cpp-std is defined by flatc default settings.
  set TEST_CPP_FLAGS=
)

set TEST_CPP_FLAGS=--gen-compare --cpp-ptr-type flatbuffers::unique_ptr %TEST_CPP_FLAGS%
set TEST_CS_FLAGS=--cs-gen-json-serializer
set TEST_RUST_FLAGS=--gen-name-strings
set TEST_BASE_FLAGS=--reflect-names --gen-mutable --gen-object-api
set TEST_NOINCL_FLAGS=%TEST_BASE_FLAGS% --no-includes --no-fb-import

..\%buildtype%\flatc.exe --binary --cpp --java --kotlin --csharp --dart --go --lobster --lua --js --ts --php --grpc ^
%TEST_NOINCL_FLAGS% %TEST_CPP_FLAGS% %TEST_CS_FLAGS% -I include_test monster_test.fbs monsterdata_test.json || goto FAIL
..\%buildtype%\flatc.exe --rust %TEST_NOINCL_FLAGS% %TEST_RUST_FLAGS% -I include_test monster_test.fbs monsterdata_test.json || goto FAIL

..\%buildtype%\flatc.exe --python %TEST_BASE_FLAGS% --no-fb-import -I include_test monster_test.fbs monsterdata_test.json || goto FAIL

..\%buildtype%\flatc.exe --binary --cpp --java --csharp --dart --go --lobster --lua --js --ts --php --python --rust ^
%TEST_NOINCL_FLAGS% %TEST_CPP_FLAGS% %TEST_CS_FLAGS% -o namespace_test namespace_test/namespace_test1.fbs namespace_test/namespace_test2.fbs || goto FAIL

..\%buildtype%\flatc.exe --cpp --java --csharp --js --ts --php %TEST_BASE_FLAGS% %TEST_CPP_FLAGS% %TEST_CS_FLAGS% -o union_vector ./union_vector/union_vector.fbs || goto FAIL
..\%buildtype%\flatc.exe --rust -I include_test -o include_test include_test/include_test1.fbs || goto FAIL
..\%buildtype%\flatc.exe --rust -I include_test -o include_test/sub include_test/sub/include_test2.fbs || goto FAIL
..\%buildtype%\flatc.exe -b --schema --bfbs-comments --bfbs-builtins -I include_test monster_test.fbs || goto FAIL
..\%buildtype%\flatc.exe --cpp --bfbs-comments --bfbs-builtins --bfbs-gen-embed %TEST_NOINCL_FLAGS% %TEST_CPP_FLAGS% -I include_test monster_test.fbs || goto FAIL
..\%buildtype%\flatc.exe -b --schema --bfbs-comments --bfbs-builtins -I include_test arrays_test.fbs || goto FAIL
..\%buildtype%\flatc.exe --jsonschema --schema -I include_test monster_test.fbs || goto FAIL
..\%buildtype%\flatc.exe --cpp --java --csharp --jsonschema %TEST_NOINCL_FLAGS% %TEST_CPP_FLAGS% %TEST_CS_FLAGS% --scoped-enums arrays_test.fbs || goto FAIL
..\%buildtype%\flatc.exe --python %TEST_BASE_FLAGS% arrays_test.fbs || goto FAIL
..\%buildtype%\flatc.exe --cpp %TEST_BASE_FLAGS% --cpp-ptr-type flatbuffers::unique_ptr native_type_test.fbs || goto FAIL

if NOT "%MONSTER_EXTRA%"=="skip" (
  @echo Generate MosterExtra
  ..\%buildtype%\flatc.exe --cpp --java --csharp %TEST_NOINCL_FLAGS% %TEST_CPP_FLAGS% %TEST_CS_FLAGS% monster_extra.fbs monsterdata_extra.json || goto FAIL
  ..\%buildtype%\flatc.exe --python %TEST_BASE_FLAGS% monster_extra.fbs monsterdata_extra.json || goto FAIL
) else (
  @echo monster_extra.fbs skipped (the strtod function from MSVC2013 or older doesn't support NaN/Inf arguments)
)

set TEST_CPP17_FLAGS=--cpp --cpp-std c++17 -o ./cpp17/generated_cpp17 %TEST_NOINCL_FLAGS%
if NOT "%MONSTER_EXTRA%"=="skip" (
  @rem Flag c++17 requires Clang6, GCC7, MSVC2017 (_MSC_VER >= 1914)  or higher.
  ..\%buildtype%\flatc.exe %TEST_CPP17_FLAGS% -I include_test monster_test.fbs  || goto FAIL
  @rem..\%buildtype%\flatc.exe %TEST_CPP17_FLAGS% arrays_test.fbs                   || goto FAIL
  @rem..\%buildtype%\flatc.exe %TEST_CPP17_FLAGS% native_type_test.fbs              || goto FAIL
  @rem..\%buildtype%\flatc.exe %TEST_CPP17_FLAGS% monster_extra.fbs                 || goto FAIL
  @rem..\%buildtype%\flatc.exe %TEST_CPP17_FLAGS% ./union_vector/union_vector.fbs   || goto FAIL
)

cd ../samples
..\%buildtype%\flatc.exe --cpp --lobster %TEST_BASE_FLAGS% %TEST_CPP_FLAGS% monster.fbs || goto FAIL
..\%buildtype%\flatc.exe -b --schema --bfbs-comments --bfbs-builtins monster.fbs || goto FAIL
cd ../reflection
call generate_code.bat %1 %2 || goto FAIL

set EXITCODE=0
goto SUCCESS
:FAIL
set EXITCODE=1
:SUCCESS
cd ../tests
EXIT /B %EXITCODE%
