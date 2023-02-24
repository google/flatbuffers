@echo off
@REM Builds a .NET solution file, adds the project, builds it
@REM and executes it. Cleans up all generated files and directories.

@REM Preserve environment
@SETLOCAL

@REM generate monster example files from parrent directory

PUSHD ..
@REM Add CMake builds results to path so flatc.exe can be found. 
SET PATH=..;..\Debug;..\Relase;..\out\build\x64-Debug;..\out\build\x64-Release;%PATH%
@REM Execute flatbuffers compiler
flatc.exe --csharp --cs-gen-json-serializer --gen-mutable --gen-object-api -I include_test monster_test.fbs optional_scalars.fbs

POPD

@REM Local temporary directory
set TEMP_BIN=.tmp

@REM Run the .NET Core tests
set CORE_FILE=FlatBuffers.Core.Test
set CORE_PROJ_FILE=%CORE_FILE%.csproj
set CORE_SLN_FILE=%CORE_FILE%.sln

dotnet new sln --force --name %CORE_FILE%
dotnet sln %CORE_SLN_FILE% add %CORE_PROJ_FILE%
dotnet build -c Release -o %TEMP_BIN% -v quiet %CORE_PROJ_FILE%
%TEMP_BIN%\%CORE_FILE%.exe
del /f %CORE_SLN_FILE%

@REM TODO(dbaileychess): Support the other configurations in NetTest.sh

@REM remove the temp bin directory, with files (/S) and quietly (/Q)
RD /S /Q %TEMP_BIN%

@ENDLOCAL
