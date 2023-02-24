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


@REM Preserve environment
@SETLOCAL
@REM Add CMake builds results to path so flatc.exe can be found. 
@SET PATH=..;..\Debug;..\Relase;..\out\build\x64-Debug;..\out\build\x64-Release;%PATH%

@REM Emit Go code for the example schemas in the test dir.
@REM A go.mod file in tests directory declares modules "tests" so use it as import path reference in generated code
flatc.exe -p  -I include_test monster_test.fbs --gen-object-api
flatc.exe -p  -I include_test monster_test.fbs --gen-object-api --gen-onefile
flatc.exe -p  -I include_test monster_extra.fbs --gen-object-api

@REM Restore environment
@ENDLOCAL

@REM Run test suite with these interpreters. The arguments are benchmark counts.
@CALL :run_tests python 100 100 100 false
@CALL :run_tests python2.6 100 100 100 false
@CALL :run_tests python2.7 100 100 100 false
@CALL :run_tests python2.7 100 100 100 true
@CALL :run_tests python3 100 100 100 false
@CALL :run_tests python3 100 100 100 true
@CALL :run_tests pypy 100 100 100 false

@REM Run test suite with default python intereter.
@REM (If the Python program `coverage` is available, it will be run, too.
@REM  Install `coverage` with `pip install coverage`.)
@where /q coverage
@IF NOT ERRORLEVEL 1 (
  ECHO ####################################################################################################
  SETLOCAL
  ECHO Found coverage utility, running coverage with default Python

  SET PYTHONDONTWRITEBYTECODE=1
  SET PYTHONPATH=..\python
  coverage run --source=flatbuffers,MyGame py_test.py 0 0 0
  coverage report --omit="*flatbuffers/vendor*,*py_test*"
  ENDLOCAL
) ELSE (
  ECHO Did not find coverage utility for default Python, skipping.
  ECHO Install with 'pip install coverage'.
)

@REM Exit from script
@EXIT /B


@REM Syntax: CALL run_tests <interpreter> <benchmark vtable dedupes> <benchmark read count> <benchmark build count>
:run_tests
    @ECHO ####################################################################################################
    @SETLOCAL
    @where /q %~1
    @IF NOT ERRORLEVEL 1 (
      ECHO Testing with interpreter: %~1
      ECHO ---------------------------------------------------------------------------------------------
      SET PYTHONDONTWRITEBYTECODE=1
      SET JYTHONDONTWRITEBYTECODE=1
      SET PYTHONPATH=..\python
      SET JYTHONPATH=..\python
      SET COMPARE_GENERATED_TO_GO=0
      SET COMPARE_GENERATED_TO_JAVA=0
      %~1 py_test.py %~2 %~3 %~4 %~5
      IF %~1 == python3 (
        SET PYTHONDONTWRITEBYTECODE=1
        %~1 py_flexbuffers_test.py
      )
    ) ELSE (
        ECHO The application %~1 is missing. Ensure it is installed and placed in your PATH.
    )
    @ENDLOCAL
    @EXIT /B
}



