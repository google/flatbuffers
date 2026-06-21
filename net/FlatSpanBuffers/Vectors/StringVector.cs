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
    public struct StringVector
    {
        private VectorData _data;

        public bool IsEmpty => _data.IsEmpty;
        public int Length => _data.Length;

        public StringVector(Table p, int offset)
        {
            _data = new VectorData(p, offset, 4);
        }

        public string this[int index]
            => TableOperations.GetString(_data.GetBufferIndex(index), _data.bb);

        public ReadOnlySpan<byte> GetBytes(int index)
            => TableOperations.GetStringBytes(_data.GetBufferIndex(index), _data.bb);

        public Enumerator GetEnumerator() => new Enumerator(ref this);

        public struct Enumerator
        {
            private ByteBuffer _bb;
            private VectorEnumeratorData _enumeratorData;

            internal Enumerator(ref StringVector vector)
            {
                _bb = vector._data.bb;
                _enumeratorData = new VectorEnumeratorData(ref vector._data);
            }

            public bool MoveNext() => _enumeratorData.MoveNext();
            public string Current => TableOperations.GetString(_enumeratorData.Current, _bb);
        }
    }

    public ref struct StringVectorSpan
    {
        private VectorSpanData _data;

        public bool IsEmpty => _data.IsEmpty;
        public int Length => _data.Length;

        public StringVectorSpan(TableSpan p, int offset)
        {
            _data = new VectorSpanData(p, offset, 4);
        }

        public string this[int index]
            => TableOperations.GetString(_data.GetBufferIndex(index), _data.bb);

        public ReadOnlySpan<byte> GetBytes(int index)
            => TableOperations.GetStringBytes(_data.GetBufferIndex(index), _data.bb);

        public Enumerator GetEnumerator() => new Enumerator(ref this);

        public ref struct Enumerator
        {
            private ByteSpanBuffer _bb;
            private VectorEnumeratorData _enumeratorData;

            internal Enumerator(scoped ref StringVectorSpan vector)
            {
                _bb = vector._data.bb;
                _enumeratorData = new VectorEnumeratorData(ref vector._data);
            }

            public bool MoveNext() => _enumeratorData.MoveNext();
            public string Current => TableOperations.GetString(_enumeratorData.Current, _bb);
        }
    }
}