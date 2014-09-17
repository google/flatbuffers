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
using System.Linq;

namespace FlatBuffers
{
    /// <summary>
    /// Class to mimick Java's ByteBuffer which is used heavily in Flatbuffers
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

        protected void WriteLittleEndian(int offset, byte[] data)
        {
            if (!BitConverter.IsLittleEndian)
            {
                data = data.Reverse().ToArray();
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
    }
}
