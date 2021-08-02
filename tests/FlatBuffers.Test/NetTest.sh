#!/bin/sh

PROJ_FILE=FlatBuffers.Test.csproj
CORE_PROJ_FILE=FlatBuffers.Core.Test.csproj

TEMP_DOTNET_DIR=.dotnet_tmp
TEMP_BIN=.tmp

[ -d $TEMP_DOTNET_DIR ] || mkdir $TEMP_DOTNET_DIR

[ -f dotnet-install.sh ] || curl -OL https://dot.net/v1/dotnet-install.sh

./dotnet-install.sh --version latest --install-dir $TEMP_DOTNET_DIR

DOTNET=$TEMP_DOTNET_DIR/dotnet

$DOTNET new sln
$DOTNET sln add $PROJ_FILE
$DOTNET restore -r linux-x64 $PROJ_FILE

# Testing C# on Linux using Mono.
msbuild -property:Configuration=Release,OutputPath=$TEMP_BIN -verbosity:minimal $PROJ_FILE
mono $TEMP_BIN/FlatBuffers.Test.exe
rm -fr $TEMP_BIN

# Repeat with unsafe versions
msbuild -property:Configuration=Release,UnsafeByteBuffer=true,OutputPath=$TEMP_BIN -verbosity:minimal $PROJ_FILE
mono $TEMP_BIN/FlatBuffers.Test.exe
rm -fr $TEMP_BIN

rm FlatBuffers.Test.sln
rm -rf obj

$DOTNET new sln
$DOTNET sln add $CORE_PROJ_FILE
$DOTNET restore -r linux-x64 $CORE_PROJ_FILE

# Testing C# on Linux using .Net Core.
msbuild -property:Configuration=Release,OutputPath=$TEMP_BIN -verbosity:minimal $CORE_PROJ_FILE
$TEMP_BIN/FlatBuffers.Core.Test.exe
rm -fr $TEMP_BIN

# Repeat with unsafe versions
msbuild -property:Configuration=Release,UnsafeByteBuffer=true,OutputPath=$TEMP_BIN -verbosity:minimal $CORE_PROJ_FILE
$TEMP_BIN/FlatBuffers.Core.Test.exe
rm -fr $TEMP_BIN

# Repeat with SpanT versions
msbuild -property:Configuration=Release,EnableSpanT=true,OutputPath=$TEMP_BIN -verbosity:minimal $CORE_PROJ_FILE
$TEMP_BIN/FlatBuffers.Core.Test.exe
rm -fr $TEMP_BIN

rm FlatBuffers.Core.Test.sln
rm -rf obj
