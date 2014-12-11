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

//#define UNSAFE_BYTEBUFFER  // uncomment this line to use faster ByteBuffer

using System;
using System.Linq;

namespace FlatBuffers
{
    /// <summary>
    /// Class to mimic Java's ByteBuffer which is used heavily in Flatbuffers.
    /// If your execution environment allows unsafe code, you should enable 
    /// unsafe code in your project and #define UNSAFE_BYTEBUFFER to use a 
    /// MUCH faster version of ByteBuffer.
    /// </summary>
    public class ByteBuffer
    {
        private readonly byte[] _buffer;
        private int _pos;  // Must track start of the buffer.

        public int Length { get { return _buffer.Length; } }

        public byte[] Data { get { return _buffer; } }

        public ByteBuffer(byte[] buffer)
        {
            _buffer = buffer;
            _pos = 0;
        }

        public int position() { return _pos; }

#if !UNSAFE_BYTEBUFFER
        // Helper functions for the safe (but slower) version.
        protected void WriteLittleEndian(int offset, byte[] data)
        {
            if (!BitConverter.IsLittleEndian)
            {
                Array.Reverse(data, 0, data.Length);
            }
            Buffer.BlockCopy(data, 0, _buffer, offset, data.Length);
            _pos = offset;
        }

        protected byte[] ReadLittleEndian(int offset, int count)
        {
            AssertOffsetAndLength(offset, count);
            var tmp = new byte[count];
            Buffer.BlockCopy(_buffer, offset, tmp, 0, count);
            return (BitConverter.IsLittleEndian)
              ? tmp
              : tmp.Reverse().ToArray();
        }
#endif // UNSAFE_BYTEBUFFER

        private void AssertOffsetAndLength(int offset, int length)
        {
            if (offset < 0 ||
                offset >= _buffer.Length ||
                offset + length > _buffer.Length)
                throw new ArgumentOutOfRangeException();
        }

        public void PutSbyte(int offset, sbyte value)
        {
            AssertOffsetAndLength(offset, sizeof(sbyte));
            _buffer[offset] = (byte)value;
            _pos = offset;
        }

        public void PutByte(int offset, byte value)
        {
            AssertOffsetAndLength(offset, sizeof(byte));
            _buffer[offset] = value;
            _pos = offset;
        }

#if UNSAFE_BYTEBUFFER
        // Unsafe but more efficient versions of Put*.
        public unsafe void PutShort(int offset, short value)
        {
            AssertOffsetAndLength(offset, sizeof(short));
            fixed (byte* ptr = _buffer)
            {
                if (BitConverter.IsLittleEndian)
                {
                    *(short*)(ptr + offset) = value;
                }
                else
                {
                    ushort uvalue = (ushort)(value);
                    *(ushort*)(ptr + offset) = (ushort)(((uvalue & 0x0FU) << 8) | ((uvalue & 0xF0U) << 0));
                }
            }
            _pos = offset;
        }

        public unsafe void PutUshort(int offset, ushort value)
        {
            AssertOffsetAndLength(offset, sizeof(ushort));
            fixed (byte* ptr = _buffer)
            {
                if (BitConverter.IsLittleEndian)
                {
                    *(ushort*)(ptr + offset) = value;
                }
                else
                {
                    *(ushort*)(ptr + offset) = (ushort)(((value & 0x0FU) << 8) | ((value & 0xF0U) << 0));
                }
            }
            _pos = offset;
        }

        public unsafe void PutInt(int offset, int value)
        {
            AssertOffsetAndLength(offset, sizeof(int));
            fixed (byte* ptr = _buffer)
            {
                if (BitConverter.IsLittleEndian)
                {
                    *(int*)(ptr + offset) = value;
                }
                else
                {
                    uint uvalue = (uint)(value);
                    *(uint*)(ptr + offset) = ((uvalue & 0x000FU) << 24) | ((uvalue & 0x00F0U) << 16) | ((uvalue & 0x0F00U) << 8) | ((uvalue & 0xF000U) << 0);
                }
            }
            _pos = offset;
        }

        public unsafe void PutUint(int offset, uint value)
        {
            AssertOffsetAndLength(offset, sizeof(uint));
            fixed (byte* ptr = _buffer)
            {
                if (BitConverter.IsLittleEndian)
                {
                    *(uint*)(ptr + offset) = value;
                }
                else
                {
                    *(uint*)(ptr + offset) = ((value & 0x000FU) << 24) | ((value & 0x00F0U) << 16) | ((value & 0x0F00U) << 8) | ((value & 0xF000U) << 0);
                }
            }
            _pos = offset;
        }

        public unsafe void PutLong(int offset, long value)
        {
            AssertOffsetAndLength(offset, sizeof(long));
            
            fixed (byte* ptr = _buffer)
            {
                if (BitConverter.IsLittleEndian)
                {
                    *(long*)(ptr + offset) = value;
                }
                else
                {
                    ulong uvalue = (ulong)(value);
                    *(ulong*)(ptr + offset) = (((uvalue & 0x0000000FUL) << 56) | ((uvalue & 0x000000F0UL) << 48) | ((uvalue & 0x00000F00UL) << 40) | ((uvalue & 0x0000F000UL) << 32) |
                                               ((uvalue & 0x000F0000UL) << 24) | ((uvalue & 0x00F00000UL) << 16) | ((uvalue & 0x0F000000UL) << 8) | ((uvalue & 0xF0000000UL) << 0));
                }
            }
            _pos = offset;
        }

        public unsafe void PutUlong(int offset, ulong value)
        {
            AssertOffsetAndLength(offset, sizeof(ulong));

            fixed (byte* ptr = _buffer)
            {
                if (BitConverter.IsLittleEndian)
                {
                    *(ulong*)(ptr + offset) = value;
                }
                else
                {
                    *(ulong*)(ptr + offset) = (((value & 0x0000000FUL) << 56) | ((value & 0x000000F0UL) << 48) | ((value & 0x00000F00UL) << 40) | ((value & 0x0000F000UL) << 32) |
                                               ((value & 0x000F0000UL) << 24) | ((value & 0x00F00000UL) << 16) | ((value & 0x0F000000UL) << 8) | ((value & 0xF0000000UL) << 0));
                }
            }
            _pos = offset;
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
                    uint uvalue = *(uint*)(&value);
                    *(uint*)(ptr + offset) = ((uvalue & 0x000FU) << 24) | ((uvalue & 0x00F0U) << 16) | ((uvalue & 0x0F00U) << 8) | ((uvalue & 0xF000U) << 0);
                }
            }
            _pos = offset;
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
                    ulong uvalue = *(ulong*)(ptr + offset);
                    *(ulong*)(ptr + offset) = ((uvalue & 0x0000000FUL) << 56) | ((uvalue & 0x000000F0UL) << 48) | ((uvalue & 0x00000F00UL) << 40) | ((uvalue & 0x0000F000UL) << 32) |
                                              ((uvalue & 0x000F0000UL) << 24) | ((uvalue & 0x00F00000UL) << 16) | ((uvalue & 0x0F000000UL) << 8) | ((uvalue & 0xF0000000UL) << 0);
                }
            }
            _pos = offset;
        }
#else // !UNSAFE_BYTEBUFFER
        // Slower versions of Put* for when unsafe code is not allowed.
        public void PutShort(int offset, short value)
        {
            AssertOffsetAndLength(offset, sizeof(short));
            WriteLittleEndian(offset, BitConverter.GetBytes(value));
        }

        public void PutUshort(int offset, ushort value)
        {
            AssertOffsetAndLength(offset, sizeof(ushort));
            WriteLittleEndian(offset, BitConverter.GetBytes(value));
        }

        public void PutInt(int offset, int value)
        {
            AssertOffsetAndLength(offset, sizeof(int));
            WriteLittleEndian(offset, BitConverter.GetBytes(value));
        }

        public void PutUint(int offset, uint value)
        {
            AssertOffsetAndLength(offset, sizeof(uint));
            WriteLittleEndian(offset, BitConverter.GetBytes(value));
        }

        public void PutLong(int offset, long value)
        {
            AssertOffsetAndLength(offset, sizeof(long));
            WriteLittleEndian(offset, BitConverter.GetBytes(value));
        }

        public void PutUlong(int offset, ulong value)
        {
            AssertOffsetAndLength(offset, sizeof(ulong));
            WriteLittleEndian(offset, BitConverter.GetBytes(value));
        }

        public void PutFloat(int offset, float value)
        {
            AssertOffsetAndLength(offset, sizeof(float));
            WriteLittleEndian(offset, BitConverter.GetBytes(value));
        }

        public void PutDouble(int offset, double value)
        {
            AssertOffsetAndLength(offset, sizeof(double));
            WriteLittleEndian(offset, BitConverter.GetBytes(value));
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

#if UNSAFE_BYTEBUFFER
        // Unsafe but more efficient versions of Get*.
        public unsafe short GetShort(int offset)
        {
            AssertOffsetAndLength(offset, sizeof(short));
            fixed (byte* ptr = _buffer)
            {
                if (BitConverter.IsLittleEndian)
                {
                    return *(short*)(ptr + offset);
                }
                else
                {
                    ushort uvalue = *(ushort*)(ptr + offset);
                    return (short)(((uvalue & 0x0FU) << 8) | ((uvalue & 0xF0U) << 0));
                }
            }
        }

        public unsafe ushort GetUshort(int offset)
        {
            AssertOffsetAndLength(offset, sizeof(ushort));
            fixed (byte* ptr = _buffer)
            {
                if (BitConverter.IsLittleEndian)
                {
                    return *(ushort*)(ptr + offset);
                }
                else
                {
                    ushort uvalue = *(ushort*)(ptr + offset);
                    return (ushort)(((uvalue & 0x0FU) << 8) | ((uvalue & 0xF0U) << 0));
                }
            }
        }

        public unsafe int GetInt(int offset)
        {
            AssertOffsetAndLength(offset, sizeof(int));
            fixed (byte* ptr = _buffer)
            {
                if (BitConverter.IsLittleEndian) { 
                    return *(int*)(ptr + offset);
                }
                else
                {
                    uint uvalue = *(uint*)(ptr + offset);
                    return (int)(((uvalue & 0x000FU) << 24) | ((uvalue & 0x00F0U) << 16) | ((uvalue & 0x0F00U) << 8) | ((uvalue & 0xF000U) << 0));
                }
            }
        }

        public unsafe uint GetUint(int offset)
        {
            AssertOffsetAndLength(offset, sizeof(uint));
            fixed (byte* ptr = _buffer)
            {
                if (BitConverter.IsLittleEndian)
                {
                    return *(uint*)(ptr + offset);
                }
                else
                {
                    uint uvalue = *(uint*)(ptr + offset);
                    return ((uvalue & 0x000FU) << 24) | ((uvalue & 0x00F0U) << 16) | ((uvalue & 0x0F00U) << 8) | ((uvalue & 0xF000U) << 0);
                }
            }
        }

        public unsafe long GetLong(int offset)
        {
            AssertOffsetAndLength(offset, sizeof(long));
            fixed (byte* ptr = _buffer)
            {
                if (BitConverter.IsLittleEndian)
                {
                    return *(long*)(ptr + offset);
                }
                else
                {
                    ulong uvalue = (ulong)(ptr + offset);
                    return (long)(((uvalue & 0x0000000FUL) << 56) | ((uvalue & 0x000000F0UL) << 48) | ((uvalue & 0x00000F00UL) << 40) | ((uvalue & 0x0000F000UL) << 32) |
                                  ((uvalue & 0x000F0000UL) << 24) | ((uvalue & 0x00F00000UL) << 16) | ((uvalue & 0x0F000000UL) << 8) | ((uvalue & 0xF0000000UL) << 0));

                }
            }
        }

        public unsafe ulong GetUlong(int offset)
        {
            AssertOffsetAndLength(offset, sizeof(ulong));
            fixed (byte* ptr = _buffer)
            {
                if (BitConverter.IsLittleEndian)
                {
                    return *(ulong*)(ptr + offset);
                }
                else
                {
                    ulong uvalue = (ulong)(ptr + offset);
                    return ((uvalue & 0x0000000FUL) << 56) | ((uvalue & 0x000000F0UL) << 48) | ((uvalue & 0x00000F00UL) << 40) | ((uvalue & 0x0000F000UL) << 32) |
                           ((uvalue & 0x000F0000UL) << 24) | ((uvalue & 0x00F00000UL) << 16) | ((uvalue & 0x0F000000UL) << 8) | ((uvalue & 0xF0000000UL) << 0);
                }
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
                    uint uvalue = *(uint*)(ptr + offset);
                    uvalue = ((uvalue & 0x000FU) << 24) | ((uvalue & 0x00F0U) << 16) | ((uvalue & 0x0F00U) << 8) | ((uvalue & 0xF000U) << 0);
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
                    ulong uvalue = *(ulong*)(ptr + offset);
                    uvalue = ((uvalue & 0x0000000FUL) << 56) | ((uvalue & 0x000000F0UL) << 48) | ((uvalue & 0x00000F00UL) << 40) | ((uvalue & 0x0000F000UL) << 32) |
                             ((uvalue & 0x000F0000UL) << 24) | ((uvalue & 0x00F00000UL) << 16) | ((uvalue & 0x0F000000UL) << 8) | ((uvalue & 0xF0000000UL) << 0);
                    return *(double*)(&uvalue);
                }
            }
        }
#else // !UNSAFE_BYTEBUFFER
        // Slower versions of Get* for when unsafe code is not allowed.
        public short GetShort(int index)
        {
            var tmp = ReadLittleEndian(index, sizeof(short));
            var value = BitConverter.ToInt16(tmp, 0);
            return value;
        }

        public ushort GetUshort(int index)
        {
            var tmp = ReadLittleEndian(index, sizeof(ushort));
            var value = BitConverter.ToUInt16(tmp, 0);
            return value;
        }

        public int GetInt(int index)
        {
            var tmp = ReadLittleEndian(index, sizeof(int));
            var value = BitConverter.ToInt32(tmp, 0);
            return value;
        }

        public uint GetUint(int index)
        {
            var tmp = ReadLittleEndian(index, sizeof(uint));
            var value = BitConverter.ToUInt32(tmp, 0);
            return value;
        }

        public long GetLong(int index)
        {
            var tmp = ReadLittleEndian(index, sizeof(long));
            var value = BitConverter.ToInt64(tmp, 0);
            return value;
        }

        public ulong GetUlong(int index)
        {
            var tmp = ReadLittleEndian(index, sizeof(ulong));
            var value = BitConverter.ToUInt64(tmp, 0);
            return value;
        }

        public float GetFloat(int index)
        {
            var tmp = ReadLittleEndian(index, sizeof(float));
            var value = BitConverter.ToSingle(tmp, 0);
            return value;
        }

        public double GetDouble(int index)
        {
            var tmp = ReadLittleEndian(index, sizeof(double));
            var value = BitConverter.ToDouble(tmp, 0);
            return value;
        }
#endif // UNSAFE_BYTEBUFFER
    }
}
