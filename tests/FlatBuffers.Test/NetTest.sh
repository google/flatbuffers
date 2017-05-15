#!/bin/sh

# Testing C# on Linux using Mono.

mcs -out:fbnettest.exe ../../net/FlatBuffers/*.cs ../MyGame/Example/*.cs FlatBuffersTestClassAttribute.cs FlatBuffersTestMethodAttribute.cs Assert.cs FlatBuffersExampleTests.cs Program.cs ByteBufferTests.cs FlatBufferBuilderTests.cs FlatBuffersFuzzTests.cs FuzzTestData.cs Lcg.cs TestTable.cs
./fbnettest.exe
rm fbnettest.exe
rm Resources/monsterdata_cstest.mon

