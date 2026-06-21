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
using Google.FlatSpanBuffers.Utils;

namespace Google.FlatSpanBuffers.Tests
{
    [FlatBuffersTestClass]
    public class RefStructSortersTests
    {
        private struct IntComparer : IComparer<int>
        {
            public int Compare(int x, int y) => x.CompareTo(y);
        }

        private struct StringComparer : IComparer<string>
        {
            public int Compare(string x, string y) => string.Compare(x, y, StringComparison.Ordinal);
        }

        private struct ReverseIntComparer : IComparer<int>
        {
            public int Compare(int x, int y) => y.CompareTo(x);
        }

        private ref struct RefStructComparer : IComparer<int>
        {
            public int Compare(int x, int y)
            {
                return (x).CompareTo(y);
            }
        }

        [FlatBuffersTestMethod]
        public void Sort_EmptyArray_DoesNothing()
        {
            var data = new int[0];
            var span = data.AsSpan();
            var comparer = new IntComparer();
            
            RefStructSorters.Sort(span, ref comparer);
            
            Assert.AreEqual(0, data.Length);
        }

        [FlatBuffersTestMethod]
        public void Sort_SingleElement_DoesNothing()
        {
            var data = new int[] { 42 };
            var span = data.AsSpan();
            var comparer = new IntComparer();
            
            RefStructSorters.Sort(span, ref comparer);
            
            Assert.AreEqual(1, data.Length);
            Assert.AreEqual(42, data[0]);
        }

        [FlatBuffersTestMethod]
        public void Sort_SmallArray_UsesInsertionSort()
        {
            var data = new int[] { 3, 1, 4, 1, 5, 9, 2, 6 };
            var expected = new int[] { 1, 1, 2, 3, 4, 5, 6, 9 };
            var span = data.AsSpan();
            var comparer = new IntComparer();
            
            RefStructSorters.Sort(span, ref comparer);
            
            Assert.ArrayEqual(expected, data);
        }

        [FlatBuffersTestMethod]
        public void Sort_LargeArray_UsesIntroSort()
        {
            // Create array larger than InsertionSortThreshold (16)
            var random = new Random(42); // Fixed seed for reproducibility
            var data = new int[100];
            for (int i = 0; i < data.Length; i++)
            {
                data[i] = random.Next(1000);
            }
            
            var expected = data.OrderBy(x => x).ToArray();
            var span = data.AsSpan();
            var comparer = new IntComparer();
            
            RefStructSorters.Sort(span, ref comparer);
            
            Assert.ArrayEqual(expected, data);
        }

        [FlatBuffersTestMethod]
        public void Sort_AlreadySorted_RemainsStable()
        {
            var data = new int[] { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
            var expected = new int[] { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
            var span = data.AsSpan();
            var comparer = new IntComparer();
            
            RefStructSorters.Sort(span, ref comparer);
            
            Assert.ArrayEqual(expected, data);
        }

        [FlatBuffersTestMethod]
        public void Sort_ReverseSorted_SortsCorrectly()
        {
            var data = new int[] { 10, 9, 8, 7, 6, 5, 4, 3, 2, 1 };
            var expected = new int[] { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
            var span = data.AsSpan();
            var comparer = new IntComparer();
            
            RefStructSorters.Sort(span, ref comparer);
            
            Assert.ArrayEqual(expected, data);
        }

        [FlatBuffersTestMethod]
        public void Sort_WithDuplicates_SortsCorrectly()
        {
            var data = new int[] { 3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5 };
            var expected = new int[] { 1, 1, 2, 3, 3, 4, 5, 5, 5, 6, 9 };
            var span = data.AsSpan();
            var comparer = new IntComparer();
            
            RefStructSorters.Sort(span, ref comparer);
            
            Assert.ArrayEqual(expected, data);
        }

        [FlatBuffersTestMethod]
        public void Sort_StringArray_SortsCorrectly()
        {
            var data = new string[] { "banana", "apple", "cherry", "date", "elderberry" };
            var expected = new string[] { "apple", "banana", "cherry", "date", "elderberry" };
            var span = data.AsSpan();
            var comparer = new StringComparer();
            
            RefStructSorters.Sort(span, ref comparer);
            
            Assert.SpanEqual<string>(expected, data);
        }

        [FlatBuffersTestMethod]
        public void Sort_Size15_SortsCorrectly()
        {
            var size = 15;
            var random = new Random(size);
            var data = new int[size];
            for (int i = 0; i < size; i++)
            {
                data[i] = random.Next(size * 2);
            }
            
            var expected = data.OrderBy(x => x).ToArray();
            var span = data.AsSpan();
            var comparer = new IntComparer();
            
            RefStructSorters.Sort(span, ref comparer);
            
            Assert.ArrayEqual(expected, data);
        }

        [FlatBuffersTestMethod]
        public void Sort_Size16_SortsCorrectly()
        {
            var size = 16;
            var random = new Random(size);
            var data = new int[size];
            for (int i = 0; i < size; i++)
            {
                data[i] = random.Next(size * 2);
            }
            
            var expected = data.OrderBy(x => x).ToArray();
            var span = data.AsSpan();
            var comparer = new IntComparer();
            
            RefStructSorters.Sort(span, ref comparer);
            
            Assert.ArrayEqual(expected, data);
        }

        [FlatBuffersTestMethod]
        public void Sort_ReverseComparer_SortsInReverseOrder()
        {
            var data = new int[] { 3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5 };
            var expected = data.OrderByDescending(x => x).ToArray();
            var span = data.AsSpan();
            var comparer = new ReverseIntComparer();
            
            RefStructSorters.Sort(span, ref comparer);
            
            Assert.ArrayEqual(expected, data);
        }

        [FlatBuffersTestMethod]
        public void Sort_AllSameElements_HandlesCorrectly()
        {
            var data = new int[50];
            Array.Fill(data, 42);
            var expected = (int[])data.Clone();
            var span = data.AsSpan();
            var comparer = new IntComparer();
            
            RefStructSorters.Sort(span, ref comparer);
            
            Assert.ArrayEqual(expected, data);
        }

        [FlatBuffersTestMethod]
        public void Sort_WithNegativeNumbers_SortsCorrectly()
        {
            var data = new int[] { -5, 3, -1, 0, 2, -3, 1, -2, 4, -4 };
            var expected = data.OrderBy(x => x).ToArray();
            var span = data.AsSpan();
            var comparer = new IntComparer();
            
            RefStructSorters.Sort(span, ref comparer);
            
            Assert.ArrayEqual(expected, data);
        }

        [FlatBuffersTestMethod]
        public void Sort_LargeRangeValues_SortsCorrectly()
        {
            var data = new int[] { int.MaxValue, int.MinValue, 0, 1000000, -1000000 };
            var expected = data.OrderBy(x => x).ToArray();
            var span = data.AsSpan();
            var comparer = new IntComparer();
            
            RefStructSorters.Sort(span, ref comparer);
            
            Assert.ArrayEqual(expected, data);
        }

        [FlatBuffersTestMethod]
        public void Sort_PatternedData_SortsCorrectly()
        {
            var data = new int[100];
            for (int i = 0; i < data.Length; i++)
            {
                data[i] = i % 10; // Repeating pattern 0,1,2,3,4,5,6,7,8,9,0,1,2...
            }
            
            var expected = data.OrderBy(x => x).ToArray();
            var span = data.AsSpan();
            var comparer = new IntComparer();
            
            RefStructSorters.Sort(span, ref comparer);
            
            Assert.ArrayEqual(expected, data);
        }

        [FlatBuffersTestMethod]
        public void Sort_WithRefStructComparer_WorksCorrectly()
        {
            var data = new int[] { 3, 1, 4, 1, 5, 9, 2, 6, 5 };
            var expected = data.OrderBy(x => x).ToArray();
            var span = data.AsSpan();
            var comparer = new RefStructComparer();
            
            RefStructSorters.Sort(span, ref comparer);
            
            Assert.ArrayEqual(expected, data);
        }

        [FlatBuffersTestMethod]
        public void Sort_TriggerHeapSortFallback_SortsCorrectly()
        {
            // Create a worst-case scenario that might trigger heapsort fallback
            // by creating data that could cause deep recursion in quicksort
            var data = new int[500];
            
            // Fill with values that create poor pivot choices
            for (int i = 0; i < data.Length; i++)
            {
                data[i] = i % 2 == 0 ? i : data.Length - i;
            }
            
            var expected = data.OrderBy(x => x).ToArray();
            var span = data.AsSpan();
            var comparer = new IntComparer();
            
            RefStructSorters.Sort(span, ref comparer);
            
            Assert.ArrayEqual(expected, data);
        }
    }
}
