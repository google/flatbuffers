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
using System.Linq;
using Google.FlatSpanBuffers.Operations;
using Google.FlatSpanBuffers.Utils;

namespace Google.FlatSpanBuffers.Tests
{
    [FlatBuffersTestClass]
    public class SortedVectorTests
    {
        // Dummy struct to represent a table type for testing
        private struct TestTable
        {
        }

        [FlatBuffersTestMethod]
        public void ScalarOffsetComparer_ComparesInts_Correctly()
        {
            var builder = new FlatBufferBuilder(256);
            
            builder.StartTable(1);
            builder.Add<int>(0, 42, 0);
            var offset1 = builder.EndTable();
            
            builder.StartTable(1);
            builder.Add<int>(0, 24, 0);
            var offset2 = builder.EndTable();
            
            builder.StartTable(1);
            builder.Add<int>(0, 100, 0);
            var offset3 = builder.EndTable();
            
            var offsets = new Offset<TestTable>[] {
                new Offset<TestTable>(offset1),
                new Offset<TestTable>(offset2),
                new Offset<TestTable>(offset3)
            };
            
            var comparer = new SortedVectorUtils.ScalarOffsetComparer<TestTable, ByteBuffer, int>(
                builder.DataBuffer, 4);
            
            // Test comparisons
            Assert.IsTrue(comparer.Compare(offsets[0], offsets[1]) > 0); // 42 > 24
            Assert.IsTrue(comparer.Compare(offsets[1], offsets[2]) < 0); // 24 < 100
            Assert.AreEqual(0, comparer.Compare(offsets[0], offsets[0])); // 42 == 42
        }

        [FlatBuffersTestMethod]
        public void VectorOffsetComparer_ComparesStrings_Correctly()
        {
            var builder = new FlatBufferBuilder(256);
            
            var str1 = builder.CreateString("apple");
            var str2 = builder.CreateString("banana");  
            var str3 = builder.CreateString("cherry");
            
            builder.StartTable(1);
            builder.AddOffset(0, str1.Value, 0);
            var offset1 = builder.EndTable();
            
            builder.StartTable(1);
            builder.AddOffset(0, str2.Value, 0);
            var offset2 = builder.EndTable();
            
            builder.StartTable(1);
            builder.AddOffset(0, str3.Value, 0);
            var offset3 = builder.EndTable();
            
            var offsets = new Offset<TestTable>[] {
                new Offset<TestTable>(offset1),
                new Offset<TestTable>(offset2),
                new Offset<TestTable>(offset3)
            };
            
            var comparer = new SortedVectorUtils.VectorOffsetComparer<TestTable, ByteBuffer, byte>(
                builder.DataBuffer, 4);
            
            // Test comparisons
            Assert.IsTrue(comparer.Compare(offsets[0], offsets[1]) < 0); // "apple" < "banana"
            Assert.IsTrue(comparer.Compare(offsets[1], offsets[2]) < 0); // "banana" < "cherry"
            Assert.AreEqual(0, comparer.Compare(offsets[0], offsets[0])); // "apple" == "apple"
        }

        [FlatBuffersTestMethod]
        public void RefStructSorters_WithScalarComparer_SortsCorrectly()
        {
            var builder = new FlatBufferBuilder(512);
            var values = new int[] { 50, 10, 30, 20, 40 };
            var offsets = new List<Offset<TestTable>>();
            
            foreach (var value in values)
            {
                builder.StartTable(1);
                builder.Add<int>(0, value, 0);
                offsets.Add(new Offset<TestTable>(builder.EndTable()));
            }
            
            var offsetsArray = offsets.ToArray();
            var comparer = new SortedVectorUtils.ScalarOffsetComparer<TestTable, ByteBuffer, int>(
                builder.DataBuffer, 4);
            
            RefStructSorters.Sort(offsetsArray, ref comparer);
            
            // Verify sorted order
            var sortedValues = new List<int>();
            var dataBuffer = builder.DataBuffer;
            foreach (var offset in offsetsArray)
            {
                var tablePos = dataBuffer.Length - offset.Value;
                var fieldOffset = TableOperations.GetOffset(4, tablePos, dataBuffer);
                if (fieldOffset != 0)
                {
                    sortedValues.Add(dataBuffer.Get<int>(tablePos + fieldOffset));
                }
            }
            
            var expected = new int[] { 10, 20, 30, 40, 50 };
            Assert.SpanEqual<int>(expected, sortedValues.ToArray());
        }

        [FlatBuffersTestMethod]
        public void RefStructSorters_WithStringComparer_SortsCorrectly()
        {
            var builder = new FlatBufferBuilder(512);
            var values = new string[] { "elephant", "apple", "dog", "banana", "cat" };
            var offsets = new List<Offset<TestTable>>();
            
            foreach (var value in values)
            {
                var str = builder.CreateString(value);
                builder.StartTable(1);
                builder.AddOffset(0, str.Value, 0);
                offsets.Add(new Offset<TestTable>(builder.EndTable()));
            }
            
            var offsetsArray = offsets.ToArray();
            var comparer = new SortedVectorUtils.VectorOffsetComparer<TestTable, ByteBuffer, byte>(
                builder.DataBuffer, 4);
            
            RefStructSorters.Sort(offsetsArray, ref comparer);
            
            // Verify sorted order
            var sortedValues = new List<string>();
            var dataBuffer = builder.DataBuffer;
            foreach (var offset in offsetsArray)
            {
                var tablePos = dataBuffer.Length - offset.Value;
                var fieldOffset = TableOperations.GetOffset(4, tablePos, dataBuffer);
                if (fieldOffset != 0)
                {
                    var str = TableOperations.GetString(tablePos + fieldOffset, dataBuffer);
                    sortedValues.Add(str);
                }
            }
            
            var expected = new string[] { "apple", "banana", "cat", "dog", "elephant" };
            Assert.SpanEqual<string>(expected, sortedValues.ToArray());
        }

        [FlatBuffersTestMethod]
        public void RefStructSorters_WithLargeDataSet_SortsCorrectly()
        {
            var builder = new FlatBufferBuilder(4096);
            var random = new Random(42);
            var values = new int[100];
            var offsets = new List<Offset<TestTable>>();
            
            for (int i = 0; i < values.Length; i++)
            {
                values[i] = random.Next(1000);
            }
            
            foreach (var value in values)
            {
                builder.StartTable(1);
                builder.Add<int>(0, value, 0);
                offsets.Add(new Offset<TestTable>(builder.EndTable()));
            }
            
            var offsetsArray = offsets.ToArray();
            var comparer = new SortedVectorUtils.ScalarOffsetComparer<TestTable, ByteBuffer, int>(
                builder.DataBuffer, 4);
            
            RefStructSorters.Sort(offsetsArray, ref comparer);
            
            // Verify sorted order
            var sortedValues = new List<int>();
            var dataBuffer = builder.DataBuffer;
            foreach (var offset in offsetsArray)
            {
                var tablePos = dataBuffer.Length - offset.Value;
                var fieldOffset = TableOperations.GetOffset(4, tablePos, dataBuffer);
                if (fieldOffset != 0)
                {
                    sortedValues.Add(dataBuffer.Get<int>(tablePos + fieldOffset));
                }
            }
            
            var expectedSorted = values.OrderBy(x => x).ToArray();
            Assert.SpanEqual<int>(expectedSorted, sortedValues.ToArray());
        }
    }
}
