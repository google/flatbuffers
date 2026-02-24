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
    public struct TableVector<T>
        where T : struct, IFlatbufferObject
    {
        private VectorData _data;

        public bool IsEmpty => _data.IsEmpty;
        public int Length => _data.Length;

        public TableVector(in Table p, int offset, int elementSize)
        {
            _data = new VectorData(p, offset, elementSize);
        }

        public T this[int index]
        {
            get
            {
                int pos = TableOperations.GetIndirect(_data.GetBufferIndex(index), _data.bb);
                var element = new T();
                element.__init(pos, _data.bb);
                return element;
            }
        }
    }

    public ref struct TableVectorSpan<T>
        where T : struct, IFlatbufferSpanObject, allows ref struct
    {
        private VectorSpanData _data;

        public bool IsEmpty => _data.IsEmpty;
        public int Length => _data.Length;

        public TableVectorSpan(TableSpan p, int offset, int elementSize)
        {
            _data = new VectorSpanData(p, offset, elementSize);
        }

        public T this[int index]
        {
            get
            {
                int pos = TableOperations.GetIndirect(_data.GetBufferIndex(index), _data.bb);
                var element = new T();
                element.__init(pos, _data.bb);
                return element;
            }
        }
    }
}
