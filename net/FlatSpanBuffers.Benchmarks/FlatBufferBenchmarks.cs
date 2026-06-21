// Copyright 2014 Google Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

using System;
using BenchmarkDotNet.Attributes;
using BenchmarkDotNet.Jobs;

// Aliases for the compared implementations.
using OriginalFlatBufferBuilder = Google.FlatBuffers.FlatBufferBuilder;
using OriginalByteBuffer = Google.FlatBuffers.ByteBuffer;
using FlatBufferBuilder = Google.FlatSpanBuffers.FlatBufferBuilder;
using ByteBuffer = Google.FlatSpanBuffers.ByteBuffer;

using Google.FlatSpanBuffers;

// Generated types for Google.FlatBuffers (original)
using OriginalFooBarContainer = Benchmarks.OriginalFlatBuffers.FooBarContainer;
using OriginalFooBar = Benchmarks.OriginalFlatBuffers.FooBar;
using OriginalBar = Benchmarks.OriginalFlatBuffers.Bar;
using OriginalFruit = Benchmarks.OriginalFlatBuffers.Fruit;
using OriginalSimpleMonster = Benchmarks.OriginalFlatBuffers.SimpleMonster;

// Generated types for FlatSpanBuffers (ByteBuffer).
using FlatSpanFooBarContainer = Benchmarks.FlatSpanBuffers.FooBarContainer;
using FlatSpanFooBar = Benchmarks.FlatSpanBuffers.FooBar;
using FlatSpanBar = Benchmarks.FlatSpanBuffers.Bar;
using FlatSpanFruit = Benchmarks.FlatSpanBuffers.Fruit;
using FlatSpanSimpleMonster = Benchmarks.FlatSpanBuffers.SimpleMonster;

// Generated types for FlatSpanBuffers StackBuffer (span/ref struct).
using StackFooBarContainer = Benchmarks.FlatSpanBuffers.StackBuffer.FooBarContainer;
using StackFooBar = Benchmarks.FlatSpanBuffers.StackBuffer.FooBar;
using StackBar = Benchmarks.FlatSpanBuffers.StackBuffer.Bar;
using StackSimpleMonster = Benchmarks.FlatSpanBuffers.StackBuffer.SimpleMonster;

// Object API types for Google.FlatBuffers.
using OriginalFooBarContainerT = Benchmarks.OriginalFlatBuffers.FooBarContainerT;
using OriginalFooBarT = Benchmarks.OriginalFlatBuffers.FooBarT;
using OriginalBarT = Benchmarks.OriginalFlatBuffers.BarT;
using OriginalFooT = Benchmarks.OriginalFlatBuffers.FooT;

// Object API types for FlatSpanBuffers.
using FlatSpanFooBarContainerT = Benchmarks.FlatSpanBuffers.FooBarContainerT;
using FlatSpanFooBarT = Benchmarks.FlatSpanBuffers.FooBarT;
using FlatSpanBarT = Benchmarks.FlatSpanBuffers.BarT;
using FlatSpanFooT = Benchmarks.FlatSpanBuffers.FooT;

namespace FlatSpanBuffers.Benchmarks;


[MemoryDiagnoser]
[SimpleJob(RuntimeMoniker.HostProcess)]
public class EncodeBenchmarks
{
    private string[] _encodeStrings = new string[3];
    private OriginalFlatBufferBuilder _ogfbb = null;
    private FlatBufferBuilder _fbb = null;

    [GlobalSetup]
    public void Setup()
    {
        _ogfbb = new OriginalFlatBufferBuilder(512);
        _fbb = new FlatBufferBuilder(512);

        for (int i = 0; i < _encodeStrings.Length; i++)
        {
            _encodeStrings[i] = $"FooBar{i}";
        }
    }

    [Benchmark(Baseline = true)]
    public void Original_FlatBuffers_Encode()
    {
        var builder = _ogfbb;
        builder.Clear();

        var fooBarOffsets = new Google.FlatBuffers.Offset<OriginalFooBar>[3];
        for (int j = 0; j < _encodeStrings.Length; j++)
        {
            var nameOffset = builder.CreateString(_encodeStrings[j]);
            OriginalFooBar.StartFooBar(builder);
            OriginalFooBar.AddSibling(builder, OriginalBar.CreateBar(builder,
                (ulong)(3 + j), (short)j, (sbyte)j, (uint)(j * 10),
                j * 1000, 0.5f + j, (ushort)(100 + j)));
            OriginalFooBar.AddName(builder, nameOffset);
            OriginalFooBar.AddRating(builder, 3.14159 + j);
            OriginalFooBar.AddPostfix(builder, (byte)j);
            fooBarOffsets[j] = OriginalFooBar.EndFooBar(builder);
        }

        var listOffset = OriginalFooBarContainer.CreateListVector(builder, fooBarOffsets);
        var locationOffset = builder.CreateString("SomeLocation");

        OriginalFooBarContainer.StartFooBarContainer(builder);
        OriginalFooBarContainer.AddList(builder, listOffset);
        OriginalFooBarContainer.AddInitialized(builder, true);
        OriginalFooBarContainer.AddFruit(builder, OriginalFruit.Bananas);
        OriginalFooBarContainer.AddLocation(builder, locationOffset);
        var rootOffset = OriginalFooBarContainer.EndFooBarContainer(builder);
        OriginalFooBarContainer.FinishFooBarContainerBuffer(builder, rootOffset);
    }

    [Benchmark]
    public void FlatSpanBuffers_Encode()
    {
        var builder = _fbb;
        builder.Clear();

        Span<Google.FlatSpanBuffers.Offset<FlatSpanFooBar>> fooBarOffsets = stackalloc Google.FlatSpanBuffers.Offset<FlatSpanFooBar>[3];
        for (int j = 0; j < _encodeStrings.Length; j++)
        {
            var nameOffset = builder.CreateString(_encodeStrings[j]);
            FlatSpanFooBar.StartFooBar(builder);
            FlatSpanFooBar.AddSibling(builder, FlatSpanBar.CreateBar(builder,
                (ulong)(3 + j), (short)j, (sbyte)j, (uint)(j * 10),
                j * 1000, 0.5f + j, (ushort)(100 + j)));
            FlatSpanFooBar.AddName(builder, nameOffset);
            FlatSpanFooBar.AddRating(builder, 3.14159 + j);
            FlatSpanFooBar.AddPostfix(builder, (byte)j);
            fooBarOffsets[j] = FlatSpanFooBar.EndFooBar(builder);
        }

        var listOffset = FlatSpanFooBarContainer.CreateListVectorBlock(builder, fooBarOffsets);
        var locationOffset = builder.CreateString("SomeLocation");

        FlatSpanFooBarContainer.StartFooBarContainer(builder);
        FlatSpanFooBarContainer.AddList(builder, listOffset);
        FlatSpanFooBarContainer.AddInitialized(builder, true);
        FlatSpanFooBarContainer.AddFruit(builder, FlatSpanFruit.Bananas);
        FlatSpanFooBarContainer.AddLocation(builder, locationOffset);
        var rootOffset = FlatSpanFooBarContainer.EndFooBarContainer(builder);
        FlatSpanFooBarContainer.FinishFooBarContainerBuffer(builder, rootOffset);
    }

    [Benchmark]
    public void FlatStackBuf_Encode()
    {
        Span<byte> buffer = stackalloc byte[512];
        Span<int> vtableSpace = stackalloc int[16];
        Span<int> vtableOffsetSpace = stackalloc int[16];
        var byteSpanBuffer = new ByteSpanBuffer(buffer);
        var builder = new FlatSpanBufferBuilder(byteSpanBuffer, vtableSpace, vtableOffsetSpace);

        Span<Google.FlatSpanBuffers.Offset<StackFooBar>> fooBarOffsets = stackalloc Google.FlatSpanBuffers.Offset<StackFooBar>[3];
        for (int j = 0; j < _encodeStrings.Length; j++)
        {
            var nameOffset = builder.CreateString(_encodeStrings[j]);
            StackFooBar.StartFooBar(ref builder);
            StackFooBar.AddSibling(ref builder, StackBar.CreateBar(ref builder,
                (ulong)(3 + j), (short)j, (sbyte)j, (uint)(j * 10),
                j * 1000, 0.5f + j, (ushort)(100 + j)));
            StackFooBar.AddName(ref builder, nameOffset);
            StackFooBar.AddRating(ref builder, 3.14159 + j);
            StackFooBar.AddPostfix(ref builder, (byte)j);
            fooBarOffsets[j] = StackFooBar.EndFooBar(ref builder);
        }

        var listOffset = StackFooBarContainer.CreateListVector(ref builder, fooBarOffsets);
        var locationOffset = builder.CreateString("SomeLocation");

        // Build FooBarContainer.
        StackFooBarContainer.StartFooBarContainer(ref builder);
        StackFooBarContainer.AddList(ref builder, listOffset);
        StackFooBarContainer.AddInitialized(ref builder, true);
        StackFooBarContainer.AddFruit(ref builder, FlatSpanFruit.Bananas);
        StackFooBarContainer.AddLocation(ref builder, locationOffset);
        var rootOffset = StackFooBarContainer.EndFooBarContainer(ref builder);
        builder.Finish(rootOffset.Value);
    }
}

[MemoryDiagnoser]
[SimpleJob(RuntimeMoniker.HostProcess)]
public class DecodeBenchmarks
{
    private byte[] _encodedData = null!;

    [GlobalSetup]
    public void Setup()
    {
        var builder = new OriginalFlatBufferBuilder(256);

        var fooBarOffsets = new Google.FlatBuffers.Offset<OriginalFooBar>[3];
        for (int j = 0; j < 3; j++)
        {
            var nameOffset = builder.CreateString($"FooBar{j}");
            OriginalFooBar.StartFooBar(builder);
            OriginalFooBar.AddSibling(builder, OriginalBar.CreateBar(builder,
                (ulong)j, (short)j, (sbyte)j, (uint)(j * 10),
                j * 1000, 0.5f + j, (ushort)(100 + j)));
            OriginalFooBar.AddName(builder, nameOffset);
            OriginalFooBar.AddRating(builder, 3.14159 + j);
            OriginalFooBar.AddPostfix(builder, (byte)j);
            fooBarOffsets[j] = OriginalFooBar.EndFooBar(builder);
        }

        var listOffset = OriginalFooBarContainer.CreateListVector(builder, fooBarOffsets);
        var locationOffset = builder.CreateString("SomeLocation");

        OriginalFooBarContainer.StartFooBarContainer(builder);
        OriginalFooBarContainer.AddList(builder, listOffset);
        OriginalFooBarContainer.AddInitialized(builder, true);
        OriginalFooBarContainer.AddFruit(builder, OriginalFruit.Bananas);
        OriginalFooBarContainer.AddLocation(builder, locationOffset);
        var rootOffset = OriginalFooBarContainer.EndFooBarContainer(builder);
        OriginalFooBarContainer.FinishFooBarContainerBuffer(builder, rootOffset);

        _encodedData = builder.SizedByteArray();
    }

    [Benchmark(Baseline = true)]
    public long Original_FlatBuffers_Decode()
    {
        long sum = 0;
        var bb = new OriginalByteBuffer(_encodedData);
        var container = OriginalFooBarContainer.GetRootAsFooBarContainer(bb);

        sum += container.Initialized ? 1 : 0;
        sum += (long)container.Fruit;

        var listLength = container.ListLength;
        for (int j = 0; j < listLength; j++)
        {
            var fooBar = container.List(j);
            if (fooBar.HasValue)
            {
                sum += (long)fooBar.Value.Rating;
                sum += fooBar.Value.Postfix;
                var sibling = fooBar.Value.Sibling;
                if (sibling.HasValue)
                {
                    sum += sibling.Value.Time;
                    sum += sibling.Value.Size;
                    var parent = sibling.Value.Parent;
                    sum += (long)parent.Id;
                    sum += parent.Count;
                }
            }
        }
        return sum;
    }

    [Benchmark]
    public long FlatSpanBuffers_Decode()
    {
        long sum = 0;
        var bb = new ByteBuffer(_encodedData);
        var container = FlatSpanFooBarContainer.GetRootAsFooBarContainer(bb);

        sum += container.Initialized ? 1 : 0;
        sum += (long)container.Fruit;

        var list = container.List;
        if (list.HasValue)
        {
            var listValue = list.Value;
            for (int j = 0; j < listValue.Length; j++)
            {
                var fooBar = listValue[j];
                sum += (long)fooBar.Rating;
                sum += fooBar.Postfix;
                var sibling = fooBar.Sibling;
                if (sibling.HasValue)
                {
                    var siblingValue = sibling.Value;
                    sum += siblingValue.Time;
                    sum += siblingValue.Size;
                    var parent = siblingValue.Parent;
                    sum += (long)parent.Id;
                    sum += parent.Count;
                }
            }
        }
        return sum;
    }

    [Benchmark]
    public long FlatStackBuf_Decode()
    {
        long sum = 0;
        var bb = new ByteSpanBuffer(_encodedData);
        var container = StackFooBarContainer.GetRootAsFooBarContainer(bb);

        sum += container.Initialized ? 1 : 0;
        sum += (long)container.Fruit;

        var list = container.List;
        if (list.HasValue)
        {
            var listValue = list.Value;
            for (int j = 0; j < listValue.Length; j++)
            {
                var fooBar = listValue[j];
                sum += (long)fooBar.Rating;
                sum += fooBar.Postfix;
                var sibling = fooBar.Sibling;
                if (sibling.HasValue)
                {
                    var siblingValue = sibling.Value;
                    sum += siblingValue.Time;
                    sum += siblingValue.Size;
                    var parent = siblingValue.Parent;
                    sum += (long)parent.Id;
                    sum += parent.Count;
                }
            }
        }
        return sum;
    }
}

[MemoryDiagnoser]
[SimpleJob(RuntimeMoniker.HostProcess)]
public class SimpleMonsterBenchmarks
{
    // Preallocating builders/buffers to remove heap allocaiton ovehead from the benchmark.
    private OriginalFlatBufferBuilder _ogfbb = null!;
    private FlatBufferBuilder _fbb = null;
    private byte[] _spanBuffer = null!;
    private int[] _vtableSpace = null!;
    private int[] _vtableOffsetSpace = null!;

    [GlobalSetup]
    public void Setup()
    {
        _ogfbb = new OriginalFlatBufferBuilder(256);
        _fbb = new FlatBufferBuilder(256);

        _spanBuffer = new byte[256];
        _vtableSpace = new int[16];
        _vtableOffsetSpace = new int[16];
    }

    [Benchmark(Baseline = true)]
    public void Original_FlatBuffers_BuildSimpleMonster()
    {
        _ogfbb.Clear();
        var nameOffset = _ogfbb.CreateString("MonsterName");
        OriginalSimpleMonster.StartSimpleMonster(_ogfbb);
        OriginalSimpleMonster.AddName(_ogfbb, nameOffset);
        OriginalSimpleMonster.AddHp(_ogfbb, 600);
        OriginalSimpleMonster.AddMana(_ogfbb, 1024);
        OriginalSimpleMonster.AddColor(_ogfbb, 2);
        OriginalSimpleMonster.AddTestbool(_ogfbb, true);
        OriginalSimpleMonster.AddTestf(_ogfbb, 0.3f);
        OriginalSimpleMonster.AddTestf2(_ogfbb, 0.2f);
        OriginalSimpleMonster.AddTestf3(_ogfbb, 0.1f);
        var monsterOffset = OriginalSimpleMonster.EndSimpleMonster(_ogfbb);
        _ogfbb.Finish(monsterOffset.Value);
    }

    [Benchmark]
    public void FlatSpanBuffers_BuildSimpleMonster()
    {
        _fbb.Clear();
        var nameOffset = _fbb.CreateString("MonsterName");
        FlatSpanSimpleMonster.StartSimpleMonster(_fbb);
        FlatSpanSimpleMonster.AddName(_fbb, nameOffset);
        FlatSpanSimpleMonster.AddHp(_fbb, 600);
        FlatSpanSimpleMonster.AddMana(_fbb, 1024);
        FlatSpanSimpleMonster.AddColor(_fbb, 2);
        FlatSpanSimpleMonster.AddTestbool(_fbb, true);
        FlatSpanSimpleMonster.AddTestf(_fbb, 0.3f);
        FlatSpanSimpleMonster.AddTestf2(_fbb, 0.2f);
        FlatSpanSimpleMonster.AddTestf3(_fbb, 0.1f);
        var monsterOffset = FlatSpanSimpleMonster.EndSimpleMonster(_fbb);
        _fbb.Finish(monsterOffset.Value);
    }

    [Benchmark]
    public void FlatStackBuf_BuildSimpleMonster()
    {
        var spanBb = new ByteSpanBuffer(_spanBuffer);
        var spanFbb = new FlatSpanBufferBuilder(spanBb, _vtableSpace, _vtableOffsetSpace);
        spanFbb.Clear();

        var nameOffset = spanFbb.CreateString("MonsterName");
        StackSimpleMonster.StartSimpleMonster(ref spanFbb);
        StackSimpleMonster.AddName(ref spanFbb, nameOffset);
        StackSimpleMonster.AddHp(ref spanFbb, 600);
        StackSimpleMonster.AddMana(ref spanFbb, 1024);
        StackSimpleMonster.AddColor(ref spanFbb, 2);
        StackSimpleMonster.AddTestbool(ref spanFbb, true);
        StackSimpleMonster.AddTestf(ref spanFbb, 0.3f);
        StackSimpleMonster.AddTestf2(ref spanFbb, 0.2f);
        StackSimpleMonster.AddTestf3(ref spanFbb, 0.1f);
        var monsterOffset = StackSimpleMonster.EndSimpleMonster(ref spanFbb);
        spanFbb.Finish(monsterOffset.Value);
    }
}

[MemoryDiagnoser]
[SimpleJob(RuntimeMoniker.HostProcess)]
public class EncodeObjectApiBenchmarks
{
    const int ListCount = 3;

    private OriginalFlatBufferBuilder _ogfbb = null;
    private FlatBufferBuilder _fbb = null;

    private OriginalFooBarContainerT _originalContainerT = null!;
    private FlatSpanFooBarContainerT _flatSpanContainerT = null!;

    [GlobalSetup]
    public void Setup()
    {
        _ogfbb = new OriginalFlatBufferBuilder(512);
        _fbb = new FlatBufferBuilder(512);

        // Create Original FlatBuffers Object API container
        _originalContainerT = new OriginalFooBarContainerT
        {
            List = new System.Collections.Generic.List<OriginalFooBarT>(),
            Initialized = true,
            Fruit = OriginalFruit.Bananas,
            Location = "SomeLocation"
        };

        for (int j = 0; j < ListCount; j++)
        {
            _originalContainerT.List.Add(new OriginalFooBarT
            {
                Sibling = new OriginalBarT
                {
                    Parent = new OriginalFooT
                    {
                        Id = (ulong)(3 + j),
                        Count = (short)j,
                        Prefix = (sbyte)j,
                        Length = (uint)(j * 10)
                    },
                    Time = j * 1000,
                    Ratio = 0.5f + j,
                    Size = (ushort)(100 + j)
                },
                Name = $"FooBar{j}",
                Rating = 3.14159 + j,
                Postfix = (byte)j
            });
        }

        // Create Stack FlatBuffers Object API container (same structure)
        _flatSpanContainerT = new FlatSpanFooBarContainerT
        {
            List = new System.Collections.Generic.List<FlatSpanFooBarT>(),
            Initialized = true,
            Fruit = FlatSpanFruit.Bananas,
            Location = "SomeLocation"
        };

        for (int j = 0; j < ListCount; j++)
        {
            _flatSpanContainerT.List.Add(new FlatSpanFooBarT
            {
                Sibling = new FlatSpanBarT
                {
                    Parent = new FlatSpanFooT
                    {
                        Id = (ulong)(3 + j),
                        Count = (short)j,
                        Prefix = (sbyte)j,
                        Length = (uint)(j * 10)
                    },
                    Time = j * 1000,
                    Ratio = 0.5f + j,
                    Size = (ushort)(100 + j)
                },
                Name = $"FooBar{j}",
                Rating = 3.14159 + j,
                Postfix = (byte)j
            });
        }
    }

    [Benchmark(Baseline = true)]
    public void Original_FlatBuffers_ObjectApi_Encode()
    {
        var builder = _ogfbb;
        builder.Clear();
        var offset = OriginalFooBarContainer.Pack(builder, _originalContainerT);
        OriginalFooBarContainer.FinishFooBarContainerBuffer(builder, offset);
    }

    [Benchmark]
    public void FlatSpanBuffers_ObjectApi_Encode()
    {
        var builder = _fbb;
        builder.Clear();
        var offset = FlatSpanFooBarContainer.Pack(builder, _flatSpanContainerT);
        FlatSpanFooBarContainer.FinishFooBarContainerBuffer(builder, offset);
    }

    [Benchmark]
    public void FlatStackBuf_ObjectApi_Encode()
    {
        Span<byte> buffer = stackalloc byte[512];
        Span<int> vtableSpace = stackalloc int[16];
        Span<int> vtableOffsetSpace = stackalloc int[16];
        var byteSpanBuffer = new ByteSpanBuffer(buffer);
        var builder = new FlatSpanBufferBuilder(byteSpanBuffer, vtableSpace, vtableOffsetSpace);

        var offset = StackFooBarContainer.Pack(ref builder, _flatSpanContainerT);
        builder.Finish(offset.Value);
    }
}

[MemoryDiagnoser]
[SimpleJob(RuntimeMoniker.HostProcess)]
public class DecodeObjectApiBenchmarks
{
    private byte[] _encodedData = null!;

    [GlobalSetup]
    public void Setup()
    {
        var builder = new OriginalFlatBufferBuilder(256);

        var fooBarOffsets = new Google.FlatBuffers.Offset<OriginalFooBar>[3];
        for (int j = 0; j < 3; j++)
        {
            var nameOffset = builder.CreateString($"FooBar{j}");
            OriginalFooBar.StartFooBar(builder);
            OriginalFooBar.AddSibling(builder, OriginalBar.CreateBar(builder,
                (ulong)j, (short)j, (sbyte)j, (uint)(j * 10),
                j * 1000, 0.5f + j, (ushort)(100 + j)));
            OriginalFooBar.AddName(builder, nameOffset);
            OriginalFooBar.AddRating(builder, 3.14159 + j);
            OriginalFooBar.AddPostfix(builder, (byte)j);
            fooBarOffsets[j] = OriginalFooBar.EndFooBar(builder);
        }

        var listOffset = OriginalFooBarContainer.CreateListVector(builder, fooBarOffsets);
        var locationOffset = builder.CreateString("SomeLocation");

        OriginalFooBarContainer.StartFooBarContainer(builder);
        OriginalFooBarContainer.AddList(builder, listOffset);
        OriginalFooBarContainer.AddInitialized(builder, true);
        OriginalFooBarContainer.AddFruit(builder, OriginalFruit.Bananas);
        OriginalFooBarContainer.AddLocation(builder, locationOffset);
        var rootOffset = OriginalFooBarContainer.EndFooBarContainer(builder);
        OriginalFooBarContainer.FinishFooBarContainerBuffer(builder, rootOffset);

        _encodedData = builder.SizedByteArray();
    }

    [Benchmark(Baseline = true)]
    public OriginalFooBarContainerT Original_FlatBuffers_ObjectApi_Decode()
    {
        var bb = new OriginalByteBuffer(_encodedData);
        var container = OriginalFooBarContainer.GetRootAsFooBarContainer(bb);
        return container.UnPack();
    }

    [Benchmark]
    public FlatSpanFooBarContainerT FlatSpanBuffers_ObjectApi_Decode()
    {
        var bb = new ByteBuffer(_encodedData);
        var container = FlatSpanFooBarContainer.GetRootAsFooBarContainer(bb);
        return container.UnPack();
    }

    [Benchmark]
    public FlatSpanFooBarContainerT FlatStackBuf_ObjectApi_Decode()
    {
        var bb = new ByteSpanBuffer(_encodedData);
        var container = StackFooBarContainer.GetRootAsFooBarContainer(bb);
        return container.UnPack();
    }
}

[MemoryDiagnoser]
[SimpleJob(RuntimeMoniker.HostProcess)]
public class VerifyBenchmarks
{
    private byte[] _encodedData = null!;

    [GlobalSetup]
    public void Setup()
    {
        var builder = new OriginalFlatBufferBuilder(256);

        var fooBarOffsets = new Google.FlatBuffers.Offset<OriginalFooBar>[3];
        for (int j = 0; j < 3; j++)
        {
            var nameOffset = builder.CreateString($"FooBar{j}");
            OriginalFooBar.StartFooBar(builder);
            OriginalFooBar.AddSibling(builder, OriginalBar.CreateBar(builder,
                (ulong)j, (short)j, (sbyte)j, (uint)(j * 10),
                j * 1000, 0.5f + j, (ushort)(100 + j)));
            OriginalFooBar.AddName(builder, nameOffset);
            OriginalFooBar.AddRating(builder, 3.14159 + j);
            OriginalFooBar.AddPostfix(builder, (byte)j);
            fooBarOffsets[j] = OriginalFooBar.EndFooBar(builder);
        }

        var listOffset = OriginalFooBarContainer.CreateListVector(builder, fooBarOffsets);
        var locationOffset = builder.CreateString("SomeLocation");

        OriginalFooBarContainer.StartFooBarContainer(builder);
        OriginalFooBarContainer.AddList(builder, listOffset);
        OriginalFooBarContainer.AddInitialized(builder, true);
        OriginalFooBarContainer.AddFruit(builder, OriginalFruit.Bananas);
        OriginalFooBarContainer.AddLocation(builder, locationOffset);
        var rootOffset = OriginalFooBarContainer.EndFooBarContainer(builder);
        OriginalFooBarContainer.FinishFooBarContainerBuffer(builder, rootOffset);

        _encodedData = builder.SizedByteArray();
    }

    [Benchmark(Baseline = true)]
    public bool Original_FlatBuffers_Verify()
    {
        var bb = new OriginalByteBuffer(_encodedData);
        // Work around original Google.FlatBuffers.Verifier.CheckBufferFromStart
        // where empty string identifier causes ArgumentException in BufferHasIdentifier.
        var verifier = new Google.FlatBuffers.Verifier(bb);
        return verifier.VerifyBuffer(null, false,
            global::Benchmarks.OriginalFlatBuffers.FooBarContainerVerify.Verify);
    }

    // this bench function is only here for consistency... ByteBuffer is converted into ByteSpanBuffer then verification runs.
    [Benchmark]
    public bool FlatSpanBuffers_Verify()
    {
        var bb = new ByteBuffer(_encodedData);
        return FlatSpanFooBarContainer.VerifyFooBarContainer(bb);
    }

    [Benchmark]
    public bool FlatStackBuf_Verify()
    {
        var bb = new ByteSpanBuffer(_encodedData);
        return StackFooBarContainer.VerifyFooBarContainer(bb);
    }
}
