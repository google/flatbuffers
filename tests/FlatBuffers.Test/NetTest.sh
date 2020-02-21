#!/bin/sh

# Restore nuget packages
mkdir dotnet_tmp
curl -OL https://dot.net/v1/dotnet-install.sh
chmod +x dotnet-install.sh
./dotnet-install.sh --version 3.1.101 --install-dir dotnet_tmp
dotnet_tmp/dotnet new sln
dotnet_tmp/dotnet sln add FlatBuffers.Test.csproj
curl -OL https://dist.nuget.org/win-x86-commandline/v5.4.0/nuget.exe
mono nuget.exe restore

# Copy Test Files
cp ../monsterdata_test.mon Resources/
cp ../monsterdata_test.json Resources/

# Testing C# on Linux using Mono.

msbuild -property:Configuration=Release,OutputPath=tempcs -verbosity:minimal FlatBuffers.Test.csproj
mono tempcs/FlatBuffers.Test.exe
rm -fr tempcs 
rm Resources/monsterdata_cstest.mon
rm Resources/monsterdata_cstest_sp.mon

# Repeat with unsafe versions

msbuild -property:Configuration=Release,UnsafeByteBuffer=true,OutputPath=tempcsUnsafe -verbosity:minimal FlatBuffers.Test.csproj
mono tempcsUnsafe/FlatBuffers.Test.exe
rm -fr tempcsUnsafe 
rm Resources/monsterdata_cstest.mon
rm Resources/monsterdata_cstest_sp.mon

# Remove Temp Files
rm -fr dotnet_tmp
rm -fr packages
rm dotnet-install.sh
rm nuget.exe
rm FlatBuffers.Test.sln
rm Resources/monsterdata_test.mon
rm Resources/monsterdata_test.json

