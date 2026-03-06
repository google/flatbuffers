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
using System.Buffers;
using System.Collections.Generic;
using static Google.FlatSpanBuffers.Operations.TableOperations;

namespace Google.FlatSpanBuffers.Utils
{
    public static class SortedVectorUtils
    {
        private const int StackAllocBytesThreshold = 512;

        public static bool TryGetByKey<T, TBuffer>(int vectorLocation, ReadOnlySpan<char> key, scoped ref TBuffer bb, int fieldOffset, out T value)
        where T : struct, IFlatbufferObject<TBuffer>, allows ref struct
        where TBuffer : IByteBuffer, allows ref struct
        {
            int maxByteCount = System.Text.Encoding.UTF8.GetByteCount(key);
            byte[] rentedKeyBytes = null;
            Span<byte> keyBytes = (maxByteCount <= StackAllocBytesThreshold)
                ? stackalloc byte[maxByteCount]
                : rentedKeyBytes = ArrayPool<byte>.Shared.Rent(maxByteCount);

            try
            {
                int bytesWritten = System.Text.Encoding.UTF8.GetBytes(key, keyBytes);
                keyBytes = keyBytes.Slice(0, bytesWritten);

                int span = bb.Get<int>(vectorLocation - 4);  // FlatBuffers stores vector length 4 bytes before data.
                int start = 0;

                while (span != 0)
                {
                    int middle = span / 2;
                    int tableOffset = GetIndirect(vectorLocation + 4 * (start + middle), bb);

                    // Get the string field
                    var fieldPos = GetOffset(fieldOffset, tableOffset, bb);
                    if (fieldPos == 0)
                    {
                        span = middle;
                        continue;
                    }

                    var stringBytes = GetVectorAsSpan<byte, TBuffer>(fieldOffset, tableOffset, bb);
                    int comp = stringBytes.SequenceCompareTo(keyBytes);

                    if (comp > 0)
                    {
                        span = middle;
                    }
                    else if (comp < 0)
                    {
                        middle++;
                        start += middle;
                        span -= middle;
                    }
                    else
                    {
                        value = new T();
                        value.__init(tableOffset, bb);
                        return true;
                    }
                }

                value = default;
                return false;
            }
            finally
            {
                if (rentedKeyBytes != null)
                    ArrayPool<byte>.Shared.Return(rentedKeyBytes);
            }
        }

        public static bool TryGetByKey<T, TScalar, TBuffer>(int vectorLocation, TScalar key, scoped ref TBuffer bb, int fieldOffset, TScalar defaultValue, out T value)
            where T : struct, IFlatbufferObject<TBuffer>, allows ref struct
            where TScalar : unmanaged, IComparable<TScalar>
            where TBuffer : IByteBuffer, allows ref struct
        {
            int span = bb.Get<int>(vectorLocation - 4);  // FlatBuffers stores vector length 4 bytes before data.
            int start = 0;

            while (span != 0)
            {
                int middle = span / 2;
                int tableOffset = GetIndirect(vectorLocation + 4 * (start + middle), bb);

                // Get the scalar field value, use schema default if field not present
                var fieldPos = GetOffset(fieldOffset, tableOffset, bb);
                var fieldValue = fieldPos != 0 ? bb.Get<TScalar>(tableOffset + fieldPos) : defaultValue;

                int comp = fieldValue.CompareTo(key);
                if (comp > 0)
                {
                    span = middle;
                }
                else if (comp < 0)
                {
                    middle++;
                    start += middle;
                    span -= middle;
                }
                else
                {
                    value = new T();
                    value.__init(tableOffset, bb);
                    return true;
                }
            }

            value = default;
            return false;
        }

        public ref struct ScalarOffsetComparer<T, TBuffer, TScalar> : IComparer<Offset<T>>
            where T : struct, allows ref struct
            where TBuffer : IByteBuffer, allows ref struct
            where TScalar : unmanaged, IComparable<TScalar>
        {
            public TBuffer bb;
            public int fieldOffset;

            public ScalarOffsetComparer(TBuffer bb, int fieldOffset)
            {
                this.bb = bb;
                this.fieldOffset = fieldOffset;
            }

            public int Compare(Offset<T> a, Offset<T> b)
            {
                var posA = bb.Length - a.Value;
                var fieldPosA = GetOffset(fieldOffset, posA, bb);
                var posB = bb.Length - b.Value;
                var fieldPosB = GetOffset(fieldOffset, posB, bb);

                if (fieldPosA == 0 && fieldPosB == 0)
                    return 0;
                if (fieldPosA == 0)
                    return -1;
                if (fieldPosB == 0)
                    return 1;

                return bb.Get<TScalar>(posA + fieldPosA).CompareTo(bb.Get<TScalar>(posB + fieldPosB));
            }
        }

        public ref struct VectorOffsetComparer<T, TBuffer, TElement> : IComparer<Offset<T>>
            where T : struct, allows ref struct
            where TBuffer : IByteBuffer, allows ref struct
            where TElement : unmanaged, IComparable<TElement>
        {
            public TBuffer bb;
            public int fieldOffset;

            public VectorOffsetComparer(TBuffer bb, int fieldOffset)
            {
                this.bb = bb;
                this.fieldOffset = fieldOffset;
            }

            public int Compare(Offset<T> a, Offset<T> b)
            {
                var posA = bb.Length - a.Value;
                var fieldPosA = GetOffset(fieldOffset, posA, bb);
                var posB = bb.Length - b.Value;
                var fieldPosB = GetOffset(fieldOffset, posB, bb);

                if (fieldPosA == 0 && fieldPosB == 0) return 0;
                if (fieldPosA == 0) return -1;
                if (fieldPosB == 0) return 1;

                var spanA = GetVectorAsSpan<TElement, TBuffer>(fieldOffset, posA, bb);
                var spanB = GetVectorAsSpan<TElement, TBuffer>(fieldOffset, posB, bb);

                return spanA.SequenceCompareTo(spanB);
            }
        }
    }
}
