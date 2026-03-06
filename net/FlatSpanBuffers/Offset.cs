/*
 * Copyright 2014 Google Inc. All rights reserved.
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

namespace Google.FlatSpanBuffers
{
    public interface IFlatBufferOffset
    {
        public int Value { get; }
    }

    /// <summary>
    /// Offset class for typesafe assignments.
    /// </summary>
    public struct Offset<T> : IFlatBufferOffset
        where T : struct, allows ref struct
    {
        public int Value;
        int IFlatBufferOffset.Value => Value;

        public Offset(int value)
        {
            Value = value;
        }
    }

    public struct StringOffset : IFlatBufferOffset
    {
        public int Value;
        int IFlatBufferOffset.Value => Value;

        public StringOffset(int value)
        {
            Value = value;
        }
    }

    public struct VectorOffset : IFlatBufferOffset
    {
        public int Value;
        int IFlatBufferOffset.Value => Value;

        public VectorOffset(int value)
        {
            Value = value;
        }
    }
}
