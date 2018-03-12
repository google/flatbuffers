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

// There are 2 #defines that have an impact on performance of this ByteBuffer implementation
//
//      UNSAFE_BYTEBUFFER 
//          This will use unsafe code to manipulate the underlying byte array. This
//          can yield a reasonable performance increase.
//
//      BYTEBUFFER_NO_BOUNDS_CHECK
//          This will disable the bounds check asserts to the byte array. This can
//          yield a small performance gain in normal code..
//
// Using UNSAFE_BYTEBUFFER and BYTEBUFFER_NO_BOUNDS_CHECK together can yield a
// performance gain of ~15% for some operations, however doing so is potentially 
// dangerous. Do so at your own risk!
//

using System;
using System.IO;
using System.Text;

namespace FlatBuffers
{
    /// <summary>
    /// Class to mimic Java's ByteBuffer which is used heavily in Flatbuffers.
    /// </summary>
    public class ByteBuffer
    {
        protected byte[] _buffer;
        private int _pos;  // Must track start of the buffer.

        public int Length { get { return _buffer.Length; } }

        public ByteBuffer(int size) : this(new byte[size]) { }

        public ByteBuffer(byte[] buffer) : this(buffer, 0) { }

        public ByteBuffer(byte[] buffer, int pos)
        {
            _buffer = buffer;
            _pos = pos;
        }

        public int Position {
            get { return _pos; }
            set { _pos = value; }
        }

        public void Reset()
        {
            _pos = 0;
        }

        // Create a new ByteBuffer on the same underlying data.
        // The new ByteBuffer's position will be same as this buffer's.
        public ByteBuffer Duplicate()
        {
            return new ByteBuffer(_buffer, Position);
        }

        // Increases the size of the ByteBuffer, and copies the old data towards
        // the end of the new buffer.
        public void GrowFront(int newSize)
        {
            if ((Length & 0xC0000000) != 0)
                throw new Exception(
                    "ByteBuffer: cannot grow buffer beyond 2 gigabytes.");

            if (newSize < Length)
                throw new Exception("ByteBuffer: cannot truncate buffer.");

            byte[] newBuffer = new byte[newSize];
            Buffer.BlockCopy(_buffer, 0, newBuffer, newSize - Length,
                             Length);
            _buffer = newBuffer;
        }

        public byte[] ToArray(int pos, int len)
        {
            byte[] arr = new byte[len];
            Buffer.BlockCopy(_buffer, pos, arr, 0, len);
            return arr;
        }

        public byte[] ToSizedArray()
        {
            return ToArray(Position, Length - Position);
        }

        public byte[] ToFullArray()
        {
            return ToArray(0, Length);
        }

        public ArraySegment<byte> ToArraySegment(int pos, int len)
        {
            return new ArraySegment<byte>(_buffer, pos, len);
        }

        public MemoryStream ToMemoryStream(int pos, int len)
        {
            return new MemoryStream(_buffer, pos, len);
        }

#if !UNSAFE_BYTEBUFFER
        // Pre-allocated helper arrays for convertion.
        private float[] floathelper = new[] { 0.0f };
        private int[] inthelper = new[] { 0 };
        private double[] doublehelper = new[] { 0.0 };
        private ulong[] ulonghelper = new[] { 0UL };
#endif // !UNSAFE_BYTEBUFFER

        // Helper functions for the unsafe version.
        static public ushort ReverseBytes(ushort input)
        {
            return (ushort)(((input & 0x00FFU) << 8) |
                            ((input & 0xFF00U) >> 8));
        }
        static public uint ReverseBytes(uint input)
        {
            return ((input & 0x000000FFU) << 24) |
                   ((input & 0x0000FF00U) <<  8) |
                   ((input & 0x00FF0000U) >>  8) |
                   ((input & 0xFF000000U) >> 24);
        }
        static public ulong ReverseBytes(ulong input)
        {
            return (((input & 0x00000000000000FFUL) << 56) |
                    ((input & 0x000000000000FF00UL) << 40) |
                    ((input & 0x0000000000FF0000UL) << 24) |
                    ((input & 0x00000000FF000000UL) <<  8) |
                    ((input & 0x000000FF00000000UL) >>  8) |
                    ((input & 0x0000FF0000000000UL) >> 24) |
                    ((input & 0x00FF000000000000UL) >> 40) |
                    ((input & 0xFF00000000000000UL) >> 56));
        }

#if !UNSAFE_BYTEBUFFER
        // Helper functions for the safe (but slower) version.
        protected void WriteLittleEndian(int offset, int count, ulong data)
        {
            if (BitConverter.IsLittleEndian)
            {
                for (int i = 0; i < count; i++)
                {
                    _buffer[offset + i] = (byte)(data >> i * 8);
                }
            }
            else
            {
                for (int i = 0; i < count; i++)
                {
                    _buffer[offset + count - 1 - i] = (byte)(data >> i * 8);
                }
            }
        }

        protected ulong ReadLittleEndian(int offset, int count)
        {
            AssertOffsetAndLength(offset, count);
            ulong r = 0;
            if (BitConverter.IsLittleEndian)
            {
                for (int i = 0; i < count; i++)
                {
                  r |= (ulong)_buffer[offset + i] << i * 8;
                }
            }
            else
            {
              for (int i = 0; i < count; i++)
              {
                r |= (ulong)_buffer[offset + count - 1 - i] << i * 8;
              }
            }
            return r;
        }
#endif // !UNSAFE_BYTEBUFFER

        private void AssertOffsetAndLength(int offset, int length)
        {
            #if !BYTEBUFFER_NO_BOUNDS_CHECK
            if (offset < 0 ||
                offset > _buffer.Length - length)
                throw new ArgumentOutOfRangeException();
            #endif
        }

        public void PutSbyte(int offset, sbyte value)
        {
            AssertOffsetAndLength(offset, sizeof(sbyte));
            _buffer[offset] = (byte)value;
        }

        public void PutByte(int offset, byte value)
        {
            AssertOffsetAndLength(offset, sizeof(byte));
            _buffer[offset] = value;
        }

        public void PutByte(int offset, byte value, int count)
        {
            AssertOffsetAndLength(offset, sizeof(byte) * count);
            for (var i = 0; i < count; ++i)
                _buffer[offset + i] = value;
        }

        // this method exists in order to conform with Java ByteBuffer standards
        public void Put(int offset, byte value)
        {
            PutByte(offset, value);
        }

        public void PutStringUTF8(int offset, string value)
        {
            AssertOffsetAndLength(offset, value.Length);
            Encoding.UTF8.GetBytes(value, 0, value.Length,
                _buffer, offset);
        }

#if UNSAFE_BYTEBUFFER
        // Unsafe but more efficient versions of Put*.
        public void PutShort(int offset, short value)
        {
            PutUshort(offset, (ushort)value);
        }

        public unsafe void PutUshort(int offset, ushort value)
        {
            AssertOffsetAndLength(offset, sizeof(ushort));
            fixed (byte* ptr = _buffer)
            {
                *(ushort*)(ptr + offset) = BitConverter.IsLittleEndian
                    ? value
                    : ReverseBytes(value);
            }
        }

        public void PutInt(int offset, int value)
        {
            PutUint(offset, (uint)value);
        }

        public unsafe void PutUint(int offset, uint value)
        {
            AssertOffsetAndLength(offset, sizeof(uint));
            fixed (byte* ptr = _buffer)
            {
                *(uint*)(ptr + offset) = BitConverter.IsLittleEndian
                    ? value
                    : ReverseBytes(value);
            }
        }

        public unsafe void PutLong(int offset, long value)
        {
            PutUlong(offset, (ulong)value);
        }

        public unsafe void PutUlong(int offset, ulong value)
        {
            AssertOffsetAndLength(offset, sizeof(ulong));
            fixed (byte* ptr = _buffer)
            {
                *(ulong*)(ptr + offset) = BitConverter.IsLittleEndian
                    ? value
                    : ReverseBytes(value);
            }
        }

        public unsafe void PutFloat(int offset, float value)
        {
            AssertOffsetAndLength(offset, sizeof(float));
            fixed (byte* ptr = _buffer)
            {
                if (BitConverter.IsLittleEndian)
                {
                    *(float*)(ptr + offset) = value;
                }
                else
                {
                    *(uint*)(ptr + offset) = ReverseBytes(*(uint*)(&value));
                }
            }
        }

        public unsafe void PutDouble(int offset, double value)
        {
            AssertOffsetAndLength(offset, sizeof(double));
            fixed (byte* ptr = _buffer)
            {
                if (BitConverter.IsLittleEndian)
                {
                    *(double*)(ptr + offset) = value;

                }
                else
                {
                    *(ulong*)(ptr + offset) = ReverseBytes(*(ulong*)(ptr + offset));
                }
            }
        }
#else // !UNSAFE_BYTEBUFFER
        // Slower versions of Put* for when unsafe code is not allowed.
        public void PutShort(int offset, short value)
        {
            AssertOffsetAndLength(offset, sizeof(short));
            WriteLittleEndian(offset, sizeof(short), (ulong)value);
        }

        public void PutUshort(int offset, ushort value)
        {
            AssertOffsetAndLength(offset, sizeof(ushort));
            WriteLittleEndian(offset, sizeof(ushort), (ulong)value);
        }

        public void PutInt(int offset, int value)
        {
            AssertOffsetAndLength(offset, sizeof(int));
            WriteLittleEndian(offset, sizeof(int), (ulong)value);
        }

        public void PutUint(int offset, uint value)
        {
            AssertOffsetAndLength(offset, sizeof(uint));
            WriteLittleEndian(offset, sizeof(uint), (ulong)value);
        }

        public void PutLong(int offset, long value)
        {
            AssertOffsetAndLength(offset, sizeof(long));
            WriteLittleEndian(offset, sizeof(long), (ulong)value);
        }

        public void PutUlong(int offset, ulong value)
        {
            AssertOffsetAndLength(offset, sizeof(ulong));
            WriteLittleEndian(offset, sizeof(ulong), value);
        }

        public void PutFloat(int offset, float value)
        {
            AssertOffsetAndLength(offset, sizeof(float));
            floathelper[0] = value;
            Buffer.BlockCopy(floathelper, 0, inthelper, 0, sizeof(float));
            WriteLittleEndian(offset, sizeof(float), (ulong)inthelper[0]);
        }

        public void PutDouble(int offset, double value)
        {
            AssertOffsetAndLength(offset, sizeof(double));
            doublehelper[0] = value;
            Buffer.BlockCopy(doublehelper, 0, ulonghelper, 0, sizeof(double));
            WriteLittleEndian(offset, sizeof(double), ulonghelper[0]);
        }

#endif // UNSAFE_BYTEBUFFER

        public sbyte GetSbyte(int index)
        {
            AssertOffsetAndLength(index, sizeof(sbyte));
            return (sbyte)_buffer[index];
        }

        public byte Get(int index)
        {
            AssertOffsetAndLength(index, sizeof(byte));
            return _buffer[index];
        }

        public string GetStringUTF8(int startPos, int len)
        {
            return Encoding.UTF8.GetString(_buffer, startPos, len);
        }

#if UNSAFE_BYTEBUFFER
        // Unsafe but more efficient versions of Get*.
        public short GetShort(int offset)
        {
            return (short)GetUshort(offset);
        }

        public unsafe ushort GetUshort(int offset)
        {
            AssertOffsetAndLength(offset, sizeof(ushort));
            fixed (byte* ptr = _buffer)
            {
                return BitConverter.IsLittleEndian
                    ? *(ushort*)(ptr + offset)
                    : ReverseBytes(*(ushort*)(ptr + offset));
            }
        }

        public int GetInt(int offset)
        {
            return (int)GetUint(offset);
        }

        public unsafe uint GetUint(int offset)
        {
            AssertOffsetAndLength(offset, sizeof(uint));
            fixed (byte* ptr = _buffer)
            {
                return BitConverter.IsLittleEndian
                    ? *(uint*)(ptr + offset)
                    : ReverseBytes(*(uint*)(ptr + offset));
            }
        }

        public long GetLong(int offset)
        {
            return (long)GetUlong(offset);
        }

        public unsafe ulong GetUlong(int offset)
        {
            AssertOffsetAndLength(offset, sizeof(ulong));
            fixed (byte* ptr = _buffer)
            {
                return BitConverter.IsLittleEndian
                    ? *(ulong*)(ptr + offset)
                    : ReverseBytes(*(ulong*)(ptr + offset));
            }
        }

        public unsafe float GetFloat(int offset)
        {
            AssertOffsetAndLength(offset, sizeof(float));
            fixed (byte* ptr = _buffer)
            {
                if (BitConverter.IsLittleEndian)
                {
                    return *(float*)(ptr + offset);
                }
                else
                {
                    uint uvalue = ReverseBytes(*(uint*)(ptr + offset));
                    return *(float*)(&uvalue);
                }
            }
        }

        public unsafe double GetDouble(int offset)
        {
            AssertOffsetAndLength(offset, sizeof(double));
            fixed (byte* ptr = _buffer)
            {
                if (BitConverter.IsLittleEndian)
                {
                    return *(double*)(ptr + offset);
                }
                else
                {
                    ulong uvalue = ReverseBytes(*(ulong*)(ptr + offset));
                    return *(double*)(&uvalue);
                }
            }
        }
#else // !UNSAFE_BYTEBUFFER
        // Slower versions of Get* for when unsafe code is not allowed.
        public short GetShort(int index)
        {
            return (short)ReadLittleEndian(index, sizeof(short));
        }

        public ushort GetUshort(int index)
        {
            return (ushort)ReadLittleEndian(index, sizeof(ushort));
        }

        public int GetInt(int index)
        {
            return (int)ReadLittleEndian(index, sizeof(int));
        }

        public uint GetUint(int index)
        {
            return (uint)ReadLittleEndian(index, sizeof(uint));
        }

        public long GetLong(int index)
        {
           return (long)ReadLittleEndian(index, sizeof(long));
        }

        public ulong GetUlong(int index)
        {
            return ReadLittleEndian(index, sizeof(ulong));
        }

        public float GetFloat(int index)
        {
            int i = (int)ReadLittleEndian(index, sizeof(float));
            inthelper[0] = i;
            Buffer.BlockCopy(inthelper, 0, floathelper, 0, sizeof(float));
            return floathelper[0];
        }

        public double GetDouble(int index)
        {
            ulong i = ReadLittleEndian(index, sizeof(double));
            // There's Int64BitsToDouble but it uses unsafe code internally.
            ulonghelper[0] = i;
            Buffer.BlockCopy(ulonghelper, 0, doublehelper, 0, sizeof(double));
            return doublehelper[0];
        }
#endif // UNSAFE_BYTEBUFFER
    }
}
