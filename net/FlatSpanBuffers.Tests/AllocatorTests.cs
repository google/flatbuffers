/*
 * Copyright 2026 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

using System;
using System.Collections.Generic;
using MonsterTest;

namespace Google.FlatSpanBuffers.Tests
{
    [FlatBuffersTestClass]
    public class AllocatorTests
    {
        [FlatBuffersTestMethod]
        public void DefaultAllocator_BasicGrowth_Success()
        {
            // Start with a very small buffer that will need to grow
            var builder = new FlatBufferBuilder(8);
            
            var longString = new string('A', 100);
            var stringOffset = builder.CreateString(longString);
            Monster.StartMonster(builder);
            Monster.AddName(builder, stringOffset);
            Monster.AddHp(builder, 100);
            var monsterOffset = Monster.EndMonster(builder);
            
            builder.Finish(monsterOffset.Value);
            
            // Verify the buffer grew and data is correct
            var buffer = builder.DataBuffer;
            Assert.IsTrue(buffer.Length > 8);
            
            var monster = Monster.GetRootAsMonster(buffer);
            Assert.AreEqual(longString, monster.Name);
            Assert.AreEqual(100, monster.Hp);
        }

        [FlatBuffersTestMethod]
        public void DefaultAllocator_MultipleGrowths_InLoop_EdgeCase()
        {
            int initialLen = 16;
            // Start with a tiny buffer
            var builder = new FlatBufferBuilder(initialLen);
            
            var names = new List<StringOffset>();
            for (int i = 0; i < 3; i++)
            {
                var name = builder.CreateString($"Monster_{i:D3}_with_a_long_name");
                names.Add(name);
            }
            
            Span<Offset<Monster>> monsters = stackalloc Offset<Monster>[3];
            for (int i = 0; i < 3; i++)
            {
                Monster.StartMonster(builder);
                Monster.AddName(builder, names[i]);
                Monster.AddHp(builder, (short)(i * 10));
                monsters[i] = Monster.EndMonster(builder);
            }
            
            var vectorOffset = builder.CreateVectorOfTables<Monster>(monsters);
            
            builder.StartTable(1);
            builder.AddOffset(0, vectorOffset.Value, 0);
            var rootOffset = builder.EndTable();
            builder.Finish(rootOffset);
            
            // Verify buffer grew
            var buffer = builder.DataBuffer;
            Assert.IsTrue(buffer.Length > initialLen);
            
            // Verify we can read the data after multiple growths
            var rootTable = new Table(buffer.Get<int>(buffer.Position) + buffer.Position, buffer);
            var vectorLocation = rootTable.__vector(4);
            var vecLen = rootTable.__vector_len(4);
            Assert.IsTrue(vectorLocation != 0);
            Assert.AreEqual(3, vecLen);
        }

        [FlatBuffersTestMethod]
        public void DefaultAllocator_GrowthPreservesExistingData_Success()
        {
            int initialLen = 16;
            var builder = new FlatBufferBuilder(initialLen);
            
            var firstName = builder.CreateString("First");
            var largeName = new string('B', 200);
            var secondName = builder.CreateString(largeName);
            
            Monster.StartMonster(builder);
            Monster.AddName(builder, firstName);
            Monster.AddHp(builder, 100);
            var monster1 = Monster.EndMonster(builder);
            
            Monster.StartMonster(builder);
            Monster.AddName(builder, secondName);
            Monster.AddHp(builder, 200);
            var monster2 = Monster.EndMonster(builder);
            
            Span<Offset<Monster>> monsters = stackalloc Offset<Monster>[] { monster1, monster2 };
            var vectorOffset = builder.CreateVectorOfTables<Monster>(monsters);
            
            builder.StartTable(1);
            builder.AddOffset(0, vectorOffset.Value, 0);
            var rootOffset = builder.EndTable();
            builder.Finish(rootOffset);
            
            // Verify data is intact after growth
            var buffer = builder.DataBuffer;
            Assert.IsTrue(buffer.Length > initialLen);
            
            // Verify we can read the root table
            var rootTable = new Table(buffer.Get<int>(buffer.Position) + buffer.Position, buffer);
            var vectorLocation = rootTable.__vector(4);
            Assert.IsTrue(vectorLocation != 0);
            
            // Verify vector has 2 elements
            var vecLen = rootTable.__vector_len(4);
            Assert.AreEqual(2, vecLen);
        }

        /// <summary>
        /// Custom allocator that tracks growth operations and uses a specific growth strategy
        /// </summary>
        private class TrackingAllocator : IByteBufferAllocator<ByteBuffer>
        {
            public int GrowthCount { get; private set; }
            public List<int> RequestedSizes { get; } = new List<int>();
            public List<int> AllocatedSizes { get; } = new List<int>();
            
            private readonly int _growthFactor;

            public TrackingAllocator(int growthFactor = 2)
            {
                _growthFactor = growthFactor;
            }

            public int GrowFront(ref ByteBuffer bb, int requestedSize)
            {
                GrowthCount++;
                RequestedSizes.Add(requestedSize);
                
                if ((bb.Length & 0xC0000000) != 0)
                    throw new Exception("ByteBuffer: cannot grow buffer beyond 2 gigabytes.");

                if (requestedSize < bb.Length)
                    throw new Exception("ByteBuffer: cannot truncate buffer.");

                // Use custom growth strategy
                var newSize = Math.Max(bb.Length * _growthFactor, requestedSize);
                AllocatedSizes.Add(newSize);
                
                var oldBuffer = bb.Buffer;
                var newBuffer = new byte[newSize];
                var dataStart = newSize - bb.Length;
                System.Buffer.BlockCopy(oldBuffer, 0, newBuffer, dataStart, oldBuffer.Length);
                bb.Reset(newBuffer, 0, newSize);
                
                return newSize;
            }
        }

        [FlatBuffersTestMethod]
        public void CustomAllocator_TracksGrowthOperations_Success()
        {
            var allocator = new TrackingAllocator();
            var buffer = new ByteBuffer(16);
            var builder = new FlatBufferBuilder(buffer, new int[16], new int[16], allocator);
            
            // Force growth with large data
            var longString = new string('A', 100);
            builder.CreateString(longString);
            
            // Verify allocator was called
            Assert.IsTrue(allocator.GrowthCount > 0);
            Assert.IsTrue(allocator.RequestedSizes.Count > 0);
            Assert.IsTrue(allocator.AllocatedSizes.Count > 0);
        }

        [FlatBuffersTestMethod]
        public void CustomAllocator_CustomGrowthStrategy_Success()
        {
            // Allocator that triples buffer size instead of doubling
            var allocator = new TrackingAllocator(growthFactor: 3);
            var buffer = new ByteBuffer(32);
            var initialSize = buffer.Length;
            
            var builder = new FlatBufferBuilder(buffer, new int[16], new int[16], allocator);
            
            // Force growth
            var largeString = new string('X', 60);
            builder.CreateString(largeString);
            
            // Verify custom growth strategy was used
            Assert.AreEqual(1, allocator.GrowthCount);
            Assert.IsTrue(allocator.AllocatedSizes[0] >= initialSize * 3);
        }

        [FlatBuffersTestMethod]
        public void DefaultAllocator_SmallInitialSize_Success()
        {
            // Start with minimal practical size
            var builder = new FlatBufferBuilder(16);
            
            var str = builder.CreateString("A");
            Monster.StartMonster(builder);
            Monster.AddName(builder, str);
            var monsterOffset = Monster.EndMonster(builder);
            builder.Finish(monsterOffset.Value);
            
            var buffer = builder.DataBuffer;
            var monster = Monster.GetRootAsMonster(buffer);
            Assert.AreEqual("A", monster.Name);
        }

        [FlatBuffersTestMethod]
        public void CustomAllocator_PreservesDuringMultipleGrowths_EdgeCase()
        {
            var allocator = new TrackingAllocator();
            var buffer = new ByteBuffer(8);
            var builder = new FlatBufferBuilder(buffer, new int[16], new int[16], allocator);
            
            // Add data that will cause multiple growth operations (reduced count)
            var strings = new StringOffset[5];
            for (int i = 0; i < 5; i++)
            {
                strings[i] = builder.CreateString($"String_number_{i:D4}_with_extra_padding");
            }
            
            // Create monsters with all strings
            Span<Offset<Monster>> monsters = stackalloc Offset<Monster>[5];
            for (int i = 0; i < 5; i++)
            {
                Monster.StartMonster(builder);
                Monster.AddName(builder, strings[i]);
                Monster.AddHp(builder, (short)i);
                monsters[i] = Monster.EndMonster(builder);
            }
            
            var vectorOffset = builder.CreateVectorOfTables<Monster>(monsters);
            builder.StartTable(1);
            builder.AddOffset(0, vectorOffset.Value, 0);
            var rootOffset = builder.EndTable();
            builder.Finish(rootOffset);
            
            // Verify multiple growths occurred
            Assert.IsTrue(allocator.GrowthCount > 1);
            
            // Verify all data is intact by checking vector length
            var readBuffer = builder.DataBuffer;
            var rootTable = new Table(readBuffer.Get<int>(readBuffer.Position) + readBuffer.Position, readBuffer);
            var vectorLocation = rootTable.__vector(4);
            var vecLen = rootTable.__vector_len(4);
            
            Assert.IsTrue(vectorLocation != 0);
            Assert.AreEqual(5, vecLen);
        }
    }
}
