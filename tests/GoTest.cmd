@REM Copyright 2014 Google Inc. All rights reserved.
@REM
@REM Licensed under the Apache License, Version 2.0 (the "License");
@REM you may not use this file except in compliance with the License.
@REM You may obtain a copy of the License at
@REM
@REM     http://www.apache.org/licenses/LICENSE-2.0
@REM
@REM Unless required by applicable law or agreed to in writing, software
@REM distributed under the License is distributed on an "AS IS" BASIS,
@REM WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
@REM See the License for the specific language governing permissions and
@REM limitations under the License.


@REM Emit Go code for the example schemas in the test dir.
@REM A go.mod file in tests directory declares modules "tests" so use it as import path reference in generated code

@REM Preserve environment
@SETLOCAL

@REM Add CMake builds results to path so flatc.exe can be found. 
SET PATH=..;..\Debug;..\Relase;..\out\build\x64-Debug;..\out\build\x64-Release;%PATH%

@where /q flatc
@IF NOT ERRORLEVEL 1 (
    echo Generate GO files
    flatc -g --gen-object-api --go-module-name tests -I include_test monster_test.fbs optional_scalars.fbs
    flatc -g --gen-object-api -I include_test/sub --go-module-name tests -o . include_test/order.fbs
    flatc -g --gen-object-api --go-module-name tests -o Pizza include_test/sub/no_namespace.fbs
) ELSE (
    echo Could not find 'flatc.exe' compiler - continue with prebuilt files
)
    
@REM do not compile the gRPC generated files, which are not tested by go_test.go
@FOR %%f in (.\MyGame\Example\*_grpc.go) do @del %%f

@REM Run tests with necessary flags.
@REM Developers may wish to see more detail by appending the verbosity flag
@REM -test.v to arguments for this command, as in:
@REM   go -test -test.v ...
@REM Developers may also wish to run benchmarks, which may be achieved with the
@REM flag -test.bench and the wildcard regexp ".":
@REM   go -test -test.bench=. ...
cmd /V /C "set GO111MODULE=on&& go test go_test.go -cpp_data=./monsterdata_test.mon --out_data=./monsterdata_go_wire.mon --bench=. --benchtime=3s --fuzz=true --fuzz_fields=4 --fuzz_objects=10000"

@ENDLOCAL
