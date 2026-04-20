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
using System.Runtime.CompilerServices;

namespace Google.FlatSpanBuffers.Utils
{
    // Custom sorting implementation that avoids boxing when
    // using Span<T>.Sort() with ref struct comparers.
    public static class RefStructSorters
    {
        private const int InsertionSortThreshold = 16;
        private const int MaxStackDepth = 64;

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static void Sort<T, TComparer>(scoped Span<T> span, ref TComparer comparer)
            where TComparer : IComparer<T>, allows ref struct
        {
            if (span.Length <= 1)
                return;

            if (span.Length <= InsertionSortThreshold)
            {
                InsertionSort(span, ref comparer);
            }
            else
            {
                IntroSort(span, ref comparer);
            }
        }

        private static void IntroSort<T, TComparer>(scoped Span<T> span, ref TComparer comparer)
            where TComparer : IComparer<T>, allows ref struct
        {
            Span<(int low, int high)> stack = stackalloc (int, int)[MaxStackDepth];
            int stackSize = 0;
            
            stack[stackSize++] = (0, span.Length - 1);

            while (stackSize > 0)
            {
                var (low, high) = stack[--stackSize];        
                if (high <= low)
                    continue;

                int size = high - low + 1;
                if (size <= InsertionSortThreshold)
                {
                    InsertionSort(span.Slice(low, size), ref comparer);
                    continue;
                }

                if (stackSize >= MaxStackDepth - 2)
                {
                    HeapSort(span.Slice(low, size), ref comparer);
                    continue;
                }

                int pivotIndex = Partition(span, low, high, ref comparer);
                int leftSize = pivotIndex - low;
                int rightSize = high - pivotIndex;

                if (leftSize > rightSize)
                {
                    if (leftSize > 1)
                        stack[stackSize++] = (low, pivotIndex - 1);
                    if (rightSize > 1)
                        stack[stackSize++] = (pivotIndex + 1, high);
                }
                else
                {
                    if (rightSize > 1)
                        stack[stackSize++] = (pivotIndex + 1, high);
                    if (leftSize > 1)
                        stack[stackSize++] = (low, pivotIndex - 1);
                }
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        private static void InsertionSort<T, TComparer>(scoped Span<T> span, ref TComparer comparer)
            where TComparer : IComparer<T>, allows ref struct
        {
            for (int i = 1; i < span.Length; i++)
            {
                T key = span[i];
                int j = i - 1;
                while (j >= 0 && comparer.Compare(span[j], key) > 0)
                {
                    span[j + 1] = span[j];
                    j--;
                }
                span[j + 1] = key;
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        private static int Partition<T, TComparer>(scoped Span<T> span, int low, int high, ref TComparer comparer)
            where TComparer : IComparer<T>, allows ref struct
        {
            // Median of three pivot selection
            int mid = low + (high - low) / 2;
            
            if (comparer.Compare(span[mid], span[low]) < 0)
                Swap(ref span[low], ref span[mid]);
            if (comparer.Compare(span[high], span[low]) < 0)
                Swap(ref span[low], ref span[high]);
            if (comparer.Compare(span[high], span[mid]) < 0)
                Swap(ref span[mid], ref span[high]);
            
            Swap(ref span[mid], ref span[high]);
            T pivot = span[high];
            int i = low - 1;
            for (int j = low; j < high; j++)
            {
                if (comparer.Compare(span[j], pivot) <= 0)
                {
                    i++;
                    Swap(ref span[i], ref span[j]);
                }
            }

            Swap(ref span[i + 1], ref span[high]);
            return i + 1;
        }

        private static void HeapSort<T, TComparer>(scoped Span<T> span, ref TComparer comparer)
            where TComparer : IComparer<T>, allows ref struct
        {
            int n = span.Length;

            for (int i = n / 2 - 1; i >= 0; i--)
                Heapify(span, i, n, ref comparer);

            for (int i = n - 1; i > 0; i--)
            {
                Swap(ref span[0], ref span[i]);
                Heapify(span, 0, i, ref comparer);
            }
        }

        private static void Heapify<T, TComparer>(scoped Span<T> span, int parent, int heapSize, ref TComparer comparer)
            where TComparer : IComparer<T>, allows ref struct
        {
            int largest = parent;
            int left = 2 * parent + 1;
            int right = 2 * parent + 2;

            if (left < heapSize && comparer.Compare(span[left], span[largest]) > 0)
                largest = left;

            if (right < heapSize && comparer.Compare(span[right], span[largest]) > 0)
                largest = right;

            if (largest != parent)
            {
                Swap(ref span[parent], ref span[largest]);
                Heapify(span, largest, heapSize, ref comparer);
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        private static void Swap<T>(ref T a, ref T b)
        {
            (a, b) = (b, a);
        }
    }
}
