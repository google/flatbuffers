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
    /// <summary>
    /// This is the base for both structs and tables.
    /// This creates API consistency for FlatBuffers using different types of IByteBuffer
    /// </summary>
    public interface IFlatbufferObject<TBuffer> 
        where TBuffer : IByteBuffer, allows ref struct
    {
        void __init(int _i, TBuffer _bb);

        TBuffer ByteBuffer { get; }
    }

    public interface IFlatbufferObject : IFlatbufferObject<ByteBuffer>
    {
    }

    public interface IFlatbufferSpanObject : IFlatbufferObject<ByteSpanBuffer>
    {
    }
}
