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
/*
 * This file contains the Table struct and functionality to handle FlatBuffer tables.
 * Modifications by JT.
    // removed:__vector_as_arraysegment
    // Removed bb_pos and bb property getters.
 */

using System;
using System.Text;
using System.Runtime.InteropServices;
using Google.FlatSpanBuffers.Operations;

namespace Google.FlatSpanBuffers
{
    /// <summary>
    /// All tables in the generated code derive from this struct, and add their own accessors.
    /// </summary>
    public struct Table
    {
        public readonly int bb_pos;
        public readonly ByteBuffer bb;

        // Initializes the table view over a buffer and absolute table position.
        public Table(int _i, ByteBuffer _bb)
        {
            bb = _bb;
            bb_pos = _i;
        }

        // Look up a field in the vtable, return an offset into the object, or 0 if the field is not
        // present.
        public int __offset(int vtableOffset)
            => TableOperations.GetOffset(vtableOffset, bb_pos, bb);

        // Retrieve the relative offset stored at "offset"
        public int __indirect(int offset)
            => TableOperations.GetIndirect(offset, bb);

        // Create a .NET String from UTF-8 data stored inside the flatbuffer.
        public string __string(int offset)
            => TableOperations.GetString(offset, bb);

        // Get an encoded utf8 string as a Span of bytes.
        public Span<byte> __string_utf8_bytes(int offset)
            => TableOperations.GetStringBytes(offset, bb);

        // Get the length of a vector whose offset is stored at "offset" in this object.
        public int __vector_len(int offset)
            => TableOperations.GetVectorLength(offset, bb_pos, bb);

        // Get the start of data of a vector whose offset is stored at "offset" in this object.
        public int __vector(int offset)
            => TableOperations.GetVectorStart(offset, bb_pos, bb);

        // Get the data of a vector whose offset is stored at "offset" in this object as a
        // Span<T>. Returns an empty span if the vector is not present in the ByteBuffer.
        public Span<T> __vector_as_span<T>(int offset)
            where T : unmanaged
            => TableOperations.GetVectorAsSpan<T, ByteBuffer>(offset, bb_pos, bb);

        // Initialize any Table-derived type to point to the union at the given offset.
        public T __union<T>(int offset) where T : struct, IFlatbufferObject
            => TableOperations.GetUnion<T>(offset, bb);

        public static bool __has_identifier(ByteBuffer bb, string ident)
            => TableOperations.HasIdentifier(bb, ident);
    }

    /// <summary>
    /// All tables in the generated code derive from this struct, and add their own accessors.
    /// </summary>
    public ref struct TableSpan
    {
        public readonly int bb_pos;
        public readonly ByteSpanBuffer bb;

        // Initializes the table view over a buffer and absolute table position.
        public TableSpan(int _i, ByteSpanBuffer _bb)
        {
            bb = _bb;
            bb_pos = _i;
        }

        // Look up a field in the vtable, return an offset into the object, or 0 if the field is not
        // present.
        public int __offset(int vtableOffset)
            => TableOperations.GetOffset(vtableOffset, bb_pos, bb);

        // Retrieve the relative offset stored at "offset"
        public int __indirect(int offset)
            => TableOperations.GetIndirect(offset, bb);

        // Create a .NET String from UTF-8 data stored inside the flatbuffer.
        public string __string(int offset)
            => TableOperations.GetString(offset, bb);

        // Get an encoded utf8 string as a Span of bytes.
        public Span<byte> __string_utf8_bytes(int offset)
            => TableOperations.GetStringBytes(offset, bb);

        // Get the length of a vector whose offset is stored at "offset" in this object.
        public int __vector_len(int offset)
            => TableOperations.GetVectorLength(offset, bb_pos, bb);

        // Get the start of data of a vector whose offset is stored at "offset" in this object.
        public int __vector(int offset)
            => TableOperations.GetVectorStart(offset, bb_pos, bb);

        // Get the data of a vector whose offset is stored at "offset" in this object as a
        // Span<T>. Returns an empty span if the vector is not present in the ByteBuffer.
        public Span<T> __vector_as_span<T>(int offset)
            where T : unmanaged
            => TableOperations.GetVectorAsSpan<T, ByteSpanBuffer>(offset, bb_pos, bb);

        // Initialize any Table-derived type to point to the union at the given offset.
        public T __union<T>(int offset) where T : struct, IFlatbufferSpanObject, allows ref struct
            => TableOperations.GetUnionSpan<T>(offset, bb);

        public static bool __has_identifier(ByteSpanBuffer bb, string ident)
            => TableOperations.HasIdentifier(bb, ident);
    }
}
