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

using System;
using System.Text;

namespace FlatBuffers
{
    /// <summary>
    /// All tables in the generated code derive from this class, and add their own accessors.
    /// </summary>
    public abstract class Table
    {
        protected int bb_pos;
        protected ByteBuffer bb;

        public ByteBuffer ByteBuffer { get { return bb; } }

        // Look up a field in the vtable, return an offset into the object, or 0 if the field is not
        // present.
        protected int __offset(int vtableOffset) { return __offset(vtableOffset, bb_pos - bb.GetInt(bb_pos), bb, false); }

        protected static int __offset(int vtableOffset, int vtable, ByteBuffer _bb, bool invoked_static)
        {
            if (!invoked_static) return vtableOffset < _bb.GetShort(vtable) ? (int)_bb.GetShort(vtable + vtableOffset) : 0;
            else return (int)_bb.GetShort(vtable + vtableOffset - _bb.GetInt(vtable)) + vtable;
        }

        // Retrieve the relative offset stored at "offset"
        protected int __indirect(int offset)
        {
            return offset + bb.GetInt(offset);
        }

        // Create a .NET String from UTF-8 data stored inside the flatbuffer.
        protected string __string(int offset) { return __string(offset, bb); }

        protected static string __string(int offset, ByteBuffer _bb)
        {
            offset += _bb.GetInt(offset);
            var len = _bb.GetInt(offset);
            var startPos = offset + sizeof(int);
            return Encoding.UTF8.GetString(_bb.Data, startPos, len);
        }

        // Get the length of a vector whose offset is stored at "offset" in this object.
        protected int __vector_len(int offset)
        {
            offset += bb_pos;
            offset += bb.GetInt(offset);
            return bb.GetInt(offset);
        }

        // Get the start of data of a vector whose offset is stored at "offset" in this object.
        protected int __vector(int offset)
        {
            offset += bb_pos;
            return offset + bb.GetInt(offset) + sizeof(int);  // data starts after the length
        }

        // Get the data of a vector whoses offset is stored at "offset" in this object as an
        // ArraySegment&lt;byte&gt;. If the vector is not present in the ByteBuffer,
        // then a null value will be returned.
        protected ArraySegment<byte>? __vector_as_arraysegment(int offset) {
            var o = this.__offset(offset);
            if (0 == o)
            {
                return null;
            }

            var pos = this.__vector(o);
            var len = this.__vector_len(o);
            return new ArraySegment<byte>(this.bb.Data, pos, len);
        }

        // Initialize any Table-derived type to point to the union at the given offset.
        protected TTable __union<TTable>(TTable t, int offset) where TTable : Table
        {
            offset += bb_pos;
            t.bb_pos = offset + bb.GetInt(offset);
            t.bb = bb;
            return t;
        }

        protected static bool __has_identifier(ByteBuffer bb, string ident)
        {
            if (ident.Length != FlatBufferConstants.FileIdentifierLength)
                throw new ArgumentException("FlatBuffers: file identifier must be length " + FlatBufferConstants.FileIdentifierLength, "ident");

            for (var i = 0; i < FlatBufferConstants.FileIdentifierLength; i++)
            {
                if (ident[i] != (char)bb.Get(bb.Position + sizeof(int) + i)) return false;
            }

            return true;
        }


    }
}
