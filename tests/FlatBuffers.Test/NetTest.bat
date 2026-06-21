@echo off
@REM Builds a .NET solution file, adds the project, builds it
@REM and executes it. Cleans up all generated files and directories.

set CORE_NAME=FlatBuffers.Test
set PROJ_FILE=%CORE_NAME%.csproj
set PROJ_DIR=%cd%
set TEMP_BIN=.tmp

@REM Run the .NET tests
dotnet new sln
dotnet sln add %PROJ_FILE%

@REM Testing with default options.
dotnet msbuild -restore -target:Build -property:Configuration=Release,OutputDir=%TEMP_BIN% -verbosity:quiet %PROJ_FILE%
cd %TEMP_BIN%
.\%CORE_NAME%.exe
cd %PROJ_DIR%
rd /S /Q %TEMP_BIN% bin obj

@REM Repeat with unsafe versions
dotnet msbuild -restore -target:Build -property:Configuration=Release,UnsafeByteBuffer=true,OutputDir=%TEMP_BIN% -verbosity:quiet %PROJ_FILE%
cd %TEMP_BIN%
.\%CORE_NAME%.exe
cd %PROJ_DIR%
rd /S /Q %TEMP_BIN% bin obj

@REM Repeat with SpanT versions
dotnet msbuild -restore -target:Build -property:Configuration=Release,EnableSpanT=true,OutputDir=%TEMP_BIN% -verbosity:quiet %PROJ_FILE%
cd %TEMP_BIN%
.\%CORE_NAME%.exe
cd %PROJ_DIR%
rd /S /Q %TEMP_BIN% bin obj

del /f %CORE_NAME%.sln*

