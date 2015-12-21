using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace FlatBuffers 
{
    public struct TablePos 
    {
        public TablePos(int bb_pos, ByteBuffer bb) 
        {
            this.bb_pos = bb_pos;
            this.bb = bb;
        }

        public readonly int bb_pos;
        public readonly ByteBuffer bb;

        // Look up a field in the vtable, return an offset into the object, or 0 if the field is not
        // present.
        public int __offset(int vtableOffset)
        {
            int vtable = bb_pos - bb.GetInt(bb_pos);
            return vtableOffset < bb.GetShort(vtable) ? (int)bb.GetShort(vtable + vtableOffset) : 0;
        }

        // Retrieve the relative offset stored at "offset"
        public int __indirect(int offset)
        {
            return offset + bb.GetInt(offset);
        }

        // Create a .NET String from UTF-8 data stored inside the flatbuffer.
        public string __string(int offset)
        {
            offset += bb.GetInt(offset);
            var len = bb.GetInt(offset);
            var startPos = offset + sizeof(int);
            return Encoding.UTF8.GetString(bb.Data, startPos , len);
        }

        // Get the length of a vector whose offset is stored at "offset" in this object.
        public int __vector_len(int offset)
        {
            offset += bb_pos;
            offset += bb.GetInt(offset);
            return bb.GetInt(offset);
        }

        // Get the start of data of a vector whose offset is stored at "offset" in this object.
        public int __vector(int offset)
        {
            offset += bb_pos;
            return offset + bb.GetInt(offset) + sizeof(int);  // data starts after the length
        }

        // Get the data of a vector whoses offset is stored at "offset" in this object as an
        // ArraySegment&lt;byte&gt;. If the vector is not present in the ByteBuffer,
        // then a null value will be returned.
        public ArraySegment<byte>? __vector_as_arraysegment(int offset) {
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
        public TablePos __union(int offset)
        {
            offset += bb_pos;
            return new TablePos(offset + bb.GetInt(offset), bb);
        }

        public static bool __has_identifier(ByteBuffer bb, string ident)
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
