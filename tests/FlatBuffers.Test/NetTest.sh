#!/bin/sh

CORE_NAME=FlatBuffers.Test
PROJ_FILE=$CORE_NAME.csproj
PROJ_DIR=$PWD

TEMP_DOTNET_DIR=$PWD/.dotnet_tmp
TEMP_BIN=.tmp

[ -d $TEMP_DOTNET_DIR ] || mkdir $TEMP_DOTNET_DIR

[ -f dotnet-install.sh ] || curl -OL https://dot.net/v1/dotnet-install.sh

chmod +x ./dotnet-install.sh
./dotnet-install.sh --version Latest --install-dir $TEMP_DOTNET_DIR || exit 1

export DOTNET_ROOT=$TEMP_DOTNET_DIR
export PATH=$PATH:$DOTNET_DIR
DOTNET=$TEMP_DOTNET_DIR/dotnet

$DOTNET new sln
$DOTNET sln add $PROJ_FILE

# Testing with default options.
$DOTNET msbuild -restore -target:Build -property:Configuration=Release,OutputDir=$TEMP_BIN -verbosity:quiet $PROJ_FILE
cd $TEMP_BIN
./$CORE_NAME
cd $PROJ_DIR
rm -fr $TEMP_BIN bin obj

# Repeat with unsafe versions
$DOTNET msbuild -restore -target:Build -property:Configuration=Release,UnsafeByteBuffer=true,OutputDir=$TEMP_BIN -verbosity:quiet $PROJ_FILE
cd $TEMP_BIN
./$CORE_NAME
cd $PROJ_DIR
rm -fr $TEMP_BIN bin obj

# Repeat with SpanT versions
$DOTNET msbuild -restore -target:Build -property:Configuration=Release,EnableSpanT=true,OutputDir=$TEMP_BIN -verbosity:quiet $PROJ_FILE
cd $TEMP_BIN
./$CORE_NAME
cd $PROJ_DIR
rm -fr $TEMP_BIN bin obj

rm $CORE_NAME.sln*

