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
using Google.FlatSpanBuffers.Operations;

namespace Google.FlatSpanBuffers.Vectors
{
    public struct UnionVector
    {
        private VectorData _data;

        public bool IsEmpty => _data.IsEmpty;
        public int Length => _data.Length;

        public UnionVector(Table p, int offset, int elementSize)
        {
            _data = new VectorData(p, offset, elementSize);
        }

        public T GetAs<T>(int index)
            where T : struct, IFlatbufferObject
                => TableOperations.GetUnion<T>(_data.GetBufferIndex(index), _data.bb);

        public string GetAsString(int index)
            => TableOperations.GetString(_data.GetBufferIndex(index), _data.bb);

        public Span<byte> GetAsStringBytes(int index)
            => TableOperations.GetStringBytes(_data.GetBufferIndex(index), _data.bb);
    }

    public ref struct UnionVectorSpan
    {
        private VectorSpanData _data;

        public bool IsEmpty => _data.IsEmpty;
        public int Length => _data.Length;

        public UnionVectorSpan(TableSpan p, int offset, int elementSize)
        {
            _data = new VectorSpanData(p, offset, elementSize);
        }

        public T GetAs<T>(int index)
            where T : struct, IFlatbufferSpanObject, allows ref struct
            => TableOperations.GetUnionSpan<T>(_data.GetBufferIndex(index), _data.bb);

        public string GetAsString(int index)
            => TableOperations.GetString(_data.GetBufferIndex(index), _data.bb);

        public Span<byte> GetAsStringBytes(int index)
            => TableOperations.GetStringBytes(_data.GetBufferIndex(index), _data.bb);
    }
}