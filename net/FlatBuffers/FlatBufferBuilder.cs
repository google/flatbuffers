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
    /// Responsible for building up and accessing a flatbuffer formatted byte
    /// array (via ByteBuffer)
    /// </summary>
    public class FlatBufferBuilder
    {
        private int _space;
        private ByteBuffer _bb;
        private int _minAlign = 1;

        // The vtable for the current table (if _vtableSize >= 0)
        private int[] _vtable = new int[16];
        // The size of the vtable. -1 indicates no vtable
        private int _vtableSize = -1;
        // Starting offset of the current struct/table.
        private int _objectStart;
        // List of offsets of all vtables.
        private int[] _vtables = new int[16];
        // Number of entries in `vtables` in use.
        private int _numVtables = 0;
        // For the current vector being built.
        private int _vectorNumElems = 0;

        public FlatBufferBuilder(int initialSize)
        {
            if (initialSize <= 0)
                throw new ArgumentOutOfRangeException("initialSize",
                    initialSize, "Must be greater than zero");
            _space = initialSize;
            _bb = new ByteBuffer(new byte[initialSize]);
        }

        public void Clear()
        {
            _space = _bb.Length;
            _bb.Reset();
            _minAlign = 1;
            while (_vtableSize > 0) _vtable[--_vtableSize] = 0;
            _vtableSize = -1;
            _objectStart = 0;
            _numVtables = 0;
            _vectorNumElems = 0;
        }

        public int Offset { get { return _bb.Length - _space; } }

        public void Pad(int size)
        {
             _bb.PutByte(_space -= size, 0, size);
        }

        // Doubles the size of the ByteBuffer, and copies the old data towards
        // the end of the new buffer (since we build the buffer backwards).
        void GrowBuffer()
        {
            var oldBuf = _bb.Data;
            var oldBufSize = oldBuf.Length;
            if ((oldBufSize & 0xC0000000) != 0)
                throw new Exception(
                    "FlatBuffers: cannot grow buffer beyond 2 gigabytes.");

            var newBufSize = oldBufSize << 1;
            var newBuf = new byte[newBufSize];

            Buffer.BlockCopy(oldBuf, 0, newBuf, newBufSize - oldBufSize,
                             oldBufSize);
            _bb = new ByteBuffer(newBuf, newBufSize);
        }

        // Prepare to write an element of `size` after `additional_bytes`
        // have been written, e.g. if you write a string, you need to align
        // such the int length field is aligned to SIZEOF_INT, and the string
        // data follows it directly.
        // If all you need to do is align, `additional_bytes` will be 0.
        public void Prep(int size, int additionalBytes)
        {
            // Track the biggest thing we've ever aligned to.
            if (size > _minAlign)
                _minAlign = size;
            // Find the amount of alignment needed such that `size` is properly
            // aligned after `additional_bytes`
            var alignSize =
                ((~((int)_bb.Length - _space + additionalBytes)) + 1) &
                (size - 1);
            // Reallocate the buffer if needed.
            while (_space < alignSize + size + additionalBytes)
            {
                var oldBufSize = (int)_bb.Length;
                GrowBuffer();
                _space += (int)_bb.Length - oldBufSize;

            }
            if (alignSize > 0)
                Pad(alignSize);
        }

        public void PutBool(bool x)
        {
          _bb.PutByte(_space -= sizeof(byte), (byte)(x ? 1 : 0));
        }

        public void PutSbyte(sbyte x)
        {
          _bb.PutSbyte(_space -= sizeof(sbyte), x);
        }

        public void PutByte(byte x)
        {
            _bb.PutByte(_space -= sizeof(byte), x);
        }

        public void PutShort(short x)
        {
            _bb.PutShort(_space -= sizeof(short), x);
        }

        public void PutUshort(ushort x)
        {
          _bb.PutUshort(_space -= sizeof(ushort), x);
        }

        public void PutInt(int x)
        {
            _bb.PutInt(_space -= sizeof(int), x);
        }

        public void PutUint(uint x)
        {
          _bb.PutUint(_space -= sizeof(uint), x);
        }

        public void PutLong(long x)
        {
            _bb.PutLong(_space -= sizeof(long), x);
        }

        public void PutUlong(ulong x)
        {
          _bb.PutUlong(_space -= sizeof(ulong), x);
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
        public void AddBool(bool x) { Prep(sizeof(byte), 0); PutBool(x); }
        public void AddSbyte(sbyte x) { Prep(sizeof(sbyte), 0); PutSbyte(x); }
        public void AddByte(byte x) { Prep(sizeof(byte), 0); PutByte(x); }
        public void AddShort(short x) { Prep(sizeof(short), 0); PutShort(x); }
        public void AddUshort(ushort x) { Prep(sizeof(ushort), 0); PutUshort(x); }
        public void AddInt(int x) { Prep(sizeof(int), 0); PutInt(x); }
        public void AddUint(uint x) { Prep(sizeof(uint), 0); PutUint(x); }
        public void AddLong(long x) { Prep(sizeof(long), 0); PutLong(x); }
        public void AddUlong(ulong x) { Prep(sizeof(ulong), 0); PutUlong(x); }
        public void AddFloat(float x) { Prep(sizeof(float), 0); PutFloat(x); }
        public void AddDouble(double x) { Prep(sizeof(double), 0);
                                          PutDouble(x); }



        // Adds on offset, relative to where it will be written.
        public void AddOffset(int off)
        {
            Prep(sizeof(int), 0);  // Ensure alignment is already done.
            if (off > Offset)
                throw new ArgumentException();

            off = Offset - off + sizeof(int);
            PutInt(off);
        }

        public void StartVector(int elemSize, int count, int alignment)
        {
            NotNested();
            _vectorNumElems = count;
            Prep(sizeof(int), elemSize * count);
            Prep(alignment, elemSize * count); // Just in case alignment > int.
        }

        public VectorOffset EndVector()
        {
            PutInt(_vectorNumElems);
            return new VectorOffset(Offset);
        }

        public void Nested(int obj)
        {
            // Structs are always stored inline, so need to be created right
            // where they are used. You'll get this assert if you created it
            // elsewhere.
            if (obj != Offset)
                throw new Exception(
                    "FlatBuffers: struct must be serialized inline.");
        }

        public void NotNested()
        {
            // You should not be creating any other objects or strings/vectors
            // while an object is being constructed
            if (_vtableSize >= 0)
                throw new Exception(
                    "FlatBuffers: object serialization must not be nested.");
        }

        public void StartObject(int numfields)
        {
            if (numfields < 0)
                throw new ArgumentOutOfRangeException("Flatbuffers: invalid numfields");

            NotNested();

            if (_vtable.Length < numfields)
                _vtable = new int[numfields];

            _vtableSize = numfields;
            _objectStart = Offset;
        }


        // Set the current vtable at `voffset` to the current location in the
        // buffer.
        public void Slot(int voffset)
        {
            if (voffset >= _vtableSize)
                throw new IndexOutOfRangeException("Flatbuffers: invalid voffset");

            _vtable[voffset] = Offset;
        }

        // Add a scalar to a table at `o` into its vtable, with value `x` and default `d`
        public void AddBool(int o, bool x, bool d) { if (x != d) { AddBool(x); Slot(o); } }
        public void AddSbyte(int o, sbyte x, sbyte d) { if (x != d) { AddSbyte(x); Slot(o); } }
        public void AddByte(int o, byte x, byte d) { if (x != d) { AddByte(x); Slot(o); } }
        public void AddShort(int o, short x, int d) { if (x != d) { AddShort(x); Slot(o); } }
        public void AddUshort(int o, ushort x, ushort d) { if (x != d) { AddUshort(x); Slot(o); } }
        public void AddInt(int o, int x, int d) { if (x != d) { AddInt(x); Slot(o); } }
        public void AddUint(int o, uint x, uint d) { if (x != d) { AddUint(x); Slot(o); } }
        public void AddLong(int o, long x, long d) { if (x != d) { AddLong(x); Slot(o); } }
        public void AddUlong(int o, ulong x, ulong d) { if (x != d) { AddUlong(x); Slot(o); } }
        public void AddFloat(int o, float x, double d) { if (x != d) { AddFloat(x); Slot(o); } }
        public void AddDouble(int o, double x, double d) { if (x != d) { AddDouble(x); Slot(o); } }
        public void AddOffset(int o, int x, int d) { if (x != d) { AddOffset(x); Slot(o); } }

        public StringOffset CreateString(string s)
        {
            NotNested();            
            AddByte(0);
            var utf8StringLen = Encoding.UTF8.GetByteCount(s);
            StartVector(1, utf8StringLen, 1);
            Encoding.UTF8.GetBytes(s, 0, s.Length, _bb.Data, _space -= utf8StringLen);
            return new StringOffset(EndVector().Value);
        }

        // Structs are stored inline, so nothing additional is being added.
        // `d` is always 0.
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
            if (_vtableSize < 0)
                throw new InvalidOperationException(
                  "Flatbuffers: calling endObject without a startObject");

            AddInt((int)0);
            var vtableloc = Offset;
            // Write out the current vtable.
            for (int i = _vtableSize - 1; i >= 0 ; i--) {
                // Offset relative to the start of the table.
                short off = (short)(_vtable[i] != 0
                                        ? vtableloc - _vtable[i]
                                        : 0);
                AddShort(off);

                // clear out written entry
                _vtable[i] = 0;
            }

            const int standardFields = 2; // The fields below:
            AddShort((short)(vtableloc - _objectStart));
            AddShort((short)((_vtableSize + standardFields) *
                             sizeof(short)));

            // Search for an existing vtable that matches the current one.
            int existingVtable = 0;
            for (int i = 0; i < _numVtables; i++) {
                int vt1 = _bb.Length - _vtables[i];
                int vt2 = _space;
                short len = _bb.GetShort(vt1);
                if (len == _bb.GetShort(vt2)) {
                    for (int j = sizeof(short); j < len; j += sizeof(short)) {
                        if (_bb.GetShort(vt1 + j) != _bb.GetShort(vt2 + j)) {
                            goto endLoop;
                        }
                    }
                    existingVtable = _vtables[i];
                    break;
                }

            endLoop: { }
            }

            if (existingVtable != 0) {
                // Found a match:
                // Remove the current vtable.
                _space = _bb.Length - vtableloc;
                // Point table to existing vtable.
                _bb.PutInt(_space, existingVtable - vtableloc);
            } else {
                // No match:
                // Add the location of the current vtable to the list of
                // vtables.
                if (_numVtables == _vtables.Length)
                {
                    // Arrays.CopyOf(vtables num_vtables * 2);
                    var newvtables = new int[ _numVtables * 2];
                    Array.Copy(_vtables, newvtables, _vtables.Length);

                    _vtables = newvtables;
                };
                _vtables[_numVtables++] = Offset;
                // Point table to current vtable.
                _bb.PutInt(_bb.Length - vtableloc, Offset - vtableloc);
            }

            _vtableSize = -1;
            return vtableloc;
        }

        // This checks a required field has been set in a given table that has
        // just been constructed.
        public void Required(int table, int field)
        {
          int table_start = _bb.Length - table;
          int vtable_start = table_start - _bb.GetInt(table_start);
          bool ok = _bb.GetShort(vtable_start + field) != 0;
          // If this fails, the caller will show what field needs to be set.
          if (!ok)
            throw new InvalidOperationException("FlatBuffers: field " + field +
                                                " must be set");
        }

        public void Finish(int rootTable)
        {
            Prep(_minAlign, sizeof(int));
            AddOffset(rootTable);
            _bb.Position = _space;
        }

        public ByteBuffer DataBuffer { get { return _bb; } }

        // Utility function for copying a byte array that starts at 0.
        public byte[] SizedByteArray()
        {
            var newArray = new byte[_bb.Data.Length - _bb.Position];
            Buffer.BlockCopy(_bb.Data, _bb.Position, newArray, 0,
                             _bb.Data.Length - _bb.Position);
            return newArray;
        }

         public void Finish(int rootTable, string fileIdentifier)
         {
             Prep(_minAlign, sizeof(int) +
                             FlatBufferConstants.FileIdentifierLength);
             if (fileIdentifier.Length !=
                 FlatBufferConstants.FileIdentifierLength)
                 throw new ArgumentException(
                     "FlatBuffers: file identifier must be length " +
                     FlatBufferConstants.FileIdentifierLength,
                     "fileIdentifier");
             for (int i = FlatBufferConstants.FileIdentifierLength - 1; i >= 0;
                  i--)
             {
                AddByte((byte)fileIdentifier[i]);
             }
             Finish(rootTable);
        }


    }
}
