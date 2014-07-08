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

namespace FlatBuffersNet
{
    /// <summary>
    /// Responsible for building up and accessing a flatbuffer formatted byte array (via ByteBuffer)
    /// This code is a pretty much 1:1 port of the original
    /// </summary>
    public class FlatBufferBuilder
    {
        private int _space;
        private ByteBuffer _bb;
        private int _minAlign = 1;

        int[] vtable;        // The vtable for the current table, null otherwise.
        int object_start;    // Starting offset of the current struct/table.
        int[] vtables = new int[16];  // List of offsets of all vtables.
        int num_vtables = 0;          // Number of entries in `vtables` in use.
        int vector_num_elems = 0;     // For the current vector being built.

        public FlatBufferBuilder(int initialSize)
        {
            _space = initialSize;
            _bb = new ByteBuffer(new byte[initialSize]);
        }


        public int Offset { get { return _bb.Length - _space; } }

        public void Pad(int size)
        {
            for (var i = 0; i < size; i++)
            {
                _bb.PutByte(--_space, 0);
            }
        }

        // Doubles the size of the ByteBuffer, and copies the old data towards the
        // end of the new buffer (since we build the buffer backwards).
        void GrowBuffer()
        {
            byte[] old_buf = _bb.Data;
            int old_buf_size = old_buf.Length;
            if ((old_buf_size & 0xC0000000) != 0)
                throw new Exception("FlatBuffers: cannot grow buffer beyond 2 gigabytes.");

            int new_buf_size = old_buf_size << 1;
            var new_buf = new byte[new_buf_size];
           
            Buffer.BlockCopy(old_buf, 0, new_buf, new_buf_size - old_buf_size, old_buf_size);

            _bb = new ByteBuffer(new_buf);
        }

        // Prepare to write an element of `size` after `additional_bytes`
        // have been written, e.g. if you write a string, you need to align such
        // the int length field is aligned to SIZEOF_INT, and the string data follows it
        // directly.
        // If all you need to do is align, `additional_bytes` will be 0.
        public void Prep(int size, int additionalBytes)
        {
            // Track the biggest thing we've ever aligned to.
            if (size > _minAlign) 
                _minAlign = size;
            // Find the amount of alignment needed such that `size` is properly
            // aligned after `additional_bytes`
            int alignSize = ((~((int)_bb.Length - _space + additionalBytes)) + 1) & (size - 1);
            // Reallocate the buffer if needed.
            while (_space < alignSize + size + additionalBytes)
            {
                var oldBufSize = (int)_bb.Length;
                GrowBuffer();
                _space += (int)_bb.Length - oldBufSize;

            }
            Pad(alignSize);
        }

        public void PutByte(byte x)
        {
            _bb.PutByte(_space -= sizeof(byte), x);
        }

        public void PutShort(short x)
        {
            _bb.PutShort(_space -= sizeof(short), x);
        }

        public void PutInt32(int x)
        {
            _bb.PutInt(_space -= sizeof(int), x);
        }

        public void PutInt64(long x)
        {
            _bb.PutLong(_space -= sizeof(long), x);
        }

        public void PutFloat(float x)
        {
            _bb.PutFloat(_space -= sizeof(float), x);
        }

        public void PutDouble(double x)
        {
            _bb.PutDouble(_space -= sizeof(double), x);
        }

        // Adds a scalar to the buffer, properly aligned, and the buffer grown
        // if needed.
        public void AddByte(byte x) { Prep(sizeof(byte), 0); PutByte(x); }
        public void AddShort(short x) { Prep(sizeof(short), 0); PutShort(x); }
        public void AddInt(int x) { Prep(sizeof(int), 0); PutInt32(x); }
        public void AddLong(long x) { Prep(sizeof(long), 0); PutInt64(x); }
        public void AddFloat(float x) { Prep(sizeof(float), 0); PutFloat(x); }
        public void AddDouble(double x) { Prep(sizeof(double), 0); PutDouble(x); }



        // Adds on offset, relative to where it will be written.
        public void AddOffset(int off) 
        {
            Prep(sizeof(int), 0);  // Ensure alignment is already done.
            if (off > Offset)
                throw new ArgumentException();
            
            off = Offset - off + sizeof(int);
            PutInt32(off);
        }

        public void StartVector(int elemSize, int count)
        {
            NotNested();
            vector_num_elems = count;
            Prep(sizeof(int), elemSize * count);
        }

        public int EndVector()
        {
            PutInt32(vector_num_elems);
            return Offset;
        }

        public void Nested(int obj)
        {
            // Structs are always stored inline, so need to be created right
            // where they are used. You'll get this assert if you created it
            // elsewhere.
            if (obj != Offset)
                throw new Exception("FlatBuffers: struct must be serialized inline.");
        }

        public void NotNested()
        {
            // You should not be creating any other objects or strings/vectors
            // while an object is being constructed
            if (vtable != null)
                throw new Exception("FlatBuffers: object serialization must not be nested.");
        }

        public void StartObject(int numfields)
        {
            NotNested();
            vtable = new int[numfields];
            object_start = Offset;
        }


        // Set the current vtable at `voffset` to the current location in the buffer.
        public void Slot(int voffset)
        {
            vtable[voffset] = Offset;
        }

        // Add a scalar to a table at `o` into its vtable, with value `x` and default `d`
        public void AddByte(int o, byte x, int d) { if (x != d) { AddByte(x); Slot(o); } }
        public void AddShort(int o, short x, int d) { if (x != d) { AddShort(x); Slot(o); } }
        public void AddInt(int o, int x, int d) { if (x != d) { AddInt(x); Slot(o); } }
        public void AddLong(int o, long x, long d) { if (x != d) { AddLong(x); Slot(o); } }
        public void AddFloat(int o, float x, double d) { if (x != d) { AddFloat(x); Slot(o); } }
        public void AddDouble(int o, double x, double d) { if (x != d) { AddDouble(x); Slot(o); } }
        public void AddOffset(int o, int x, int d) { if (x != d) { AddOffset(x); Slot(o); } }

        public int CreateString(string s)
        {
            NotNested();
            byte[] utf8 = Encoding.UTF8.GetBytes(s);
            AddByte((byte)0);
            StartVector(1, utf8.Length);
            Buffer.BlockCopy(utf8, 0, _bb.Data, _space -= utf8.Length, utf8.Length);
            return EndVector();
        }

        // Structs are stored inline, so nothing additional is being added. `d` is always 0.
        public void AddStruct(int voffset, int x, int d)
        {
            if (x != d)
            {
                Nested(x);
                Slot(voffset);
            }
        }

        public int EndObject() 
        {

            if (vtable == null)
                throw new InvalidOperationException("Flatbuffers: calling endObject without a startObject"); // 

            AddInt((int)0);
            int vtableloc = Offset;
            // Write out the current vtable.
            for (int i = vtable.Length - 1; i >= 0 ; i--) {
                // Offset relative to the start of the table.
                short off = (short)(vtable[i] != 0 ? vtableloc - vtable[i] : 0);
                AddShort(off);
            }

            const int standard_fields = 2; // The fields below:
            AddShort((short)(vtableloc - object_start));
            AddShort((short)((vtable.Length + standard_fields) * sizeof(short)));

            // Search for an existing vtable that matches the current one.
            int existing_vtable = 0;
            outer_loop:
            for (int i = 0; i < num_vtables; i++) {
                int vt1 = _bb.Length - vtables[i];
                int vt2 = _space;
                short len = _bb.GetShort(vt1);
                if (len == _bb.GetShort(vt2)) {
                    for (int j = sizeof(short); j < len; j += sizeof(short)) {
                        if (_bb.GetShort(vt1 + j) != _bb.GetShort(vt2 + j)) {
                            goto endLoop;
                        }
                    }
                    existing_vtable = vtables[i];
                    break;
                }

            endLoop: { }
            }

            if (existing_vtable != 0) {
                // Found a match:
                // Remove the current vtable.
                _space = _bb.Length - vtableloc;
                // Point table to existing vtable.
                _bb.PutInt(_space, existing_vtable - vtableloc);
            } else {
                // No match:
                // Add the location of the current vtable to the list of vtables.
                if (num_vtables == vtables.Length)
                {
                    // Arrays.CopyOf(vtables num_vtables * 2);
                    var newvtables = new int[ num_vtables * 2];
                    Array.Copy(vtables, newvtables, vtables.Length);

                    vtables = newvtables;
                };
                vtables[num_vtables++] = Offset;
                // Point table to current vtable.
                _bb.PutInt(_bb.Length - vtableloc, Offset - vtableloc);
            }

            vtable = null;
            return vtableloc;
        }

        public void Finish(int root_table)
        {
            Prep(_minAlign, sizeof(int));
            AddOffset(root_table);
        }

        public ByteBuffer Data { get { return _bb; }}

        // The FlatBuffer data doesn't start at offset 0 in the ByteBuffer:
        public int DataStart
        {
            get { return _space; }
        }


        // Utility function for copying a byte array that starts at 0.
        public byte[] SizedByteArray()
        {
            var newArray = new byte[_bb.Data.Length];
            Buffer.BlockCopy(_bb.Data, DataStart, newArray, 0, _bb.Data.Length);
            return newArray;
        }

    }
}
