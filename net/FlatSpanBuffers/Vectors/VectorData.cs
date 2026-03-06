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

namespace Google.FlatSpanBuffers.Vectors
{
    // Consolidate Vector bounds and bounds checks
    public struct VectorData
    {
        public ByteBuffer bb;
        public readonly int start;
        public readonly int length;
        public readonly int elementSize;

        public bool IsEmpty => length == 0;
        public int Length => length;

        public VectorData(Table p, int offset, int elementSize)
        {
            if (offset == 0)
                throw new Exception($"Flatbufffers: Invalid Offset");

            bb = p.bb;
            start = p.__vector(offset);
            length = p.__vector_len(offset);
            this.elementSize = elementSize;
        }

        public int GetBufferIndex(int index)
        {
            // Using this bound check for consistency with Span indexed bounds checks.
            // This would have fallen into the old 'AssertOffsetAndLength()'.
            if ((uint)index >= (uint)length)
                throw new IndexOutOfRangeException($"Flatbufffers: index out of range. index:{index} length: {length}");
            return start + index * elementSize;
        }
    }

    // Consolidate Vector bounds and bounds checks
    public ref struct VectorSpanData
    {
        public ByteSpanBuffer bb;
        public readonly int start;
        public readonly int length;
        public readonly int elementSize;

        public bool IsEmpty => length == 0;
        public int Length => length;

        public VectorSpanData(TableSpan p, int offset, int elementSize)
        {
            if (offset == 0)
                throw new Exception($"Flatbufffers: Invalid Offset");

            bb = p.bb;
            start = p.__vector(offset);
            length = p.__vector_len(offset);
            this.elementSize = elementSize;
        }

        public int GetBufferIndex(int index)
        {
            // Using this bound check for consistency with Span indexed bounds checks.
            // This would have fallen into the old 'AssertOffsetAndLength()'.
            if ((uint)index >= (uint)length)
                throw new IndexOutOfRangeException($"Flatbufffers: index out of range. index:{index} length: {length}");
            return start + index * elementSize;
        }
    }

    public struct VectorEnumeratorData
    {
        private int _current;
        private readonly int _end;
        private readonly int _elementSize;

        public int Current => _current;

        public VectorEnumeratorData(ref VectorData data)
        {
            _elementSize = data.elementSize;
            _current = data.start - data.elementSize;
            _end = data.start + data.length * data.elementSize;
        }

        public VectorEnumeratorData(scoped ref VectorSpanData data)
        {
            _elementSize = data.elementSize;
            _current = data.start - data.elementSize;
            _end = data.start + data.length * data.elementSize;
        }

        public bool MoveNext()
        {
            int next = _current + _elementSize;
            if (next >= _end)
                return false;

            _current = next;
            return true;
        }
    }
}
