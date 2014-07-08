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

namespace FlatBuffersNet
{
    /// <summary>
    /// Class to mimick Java's ByteBuffer which is used heavily in Flatbuffers
    /// </summary>
    public class ByteBuffer
    {
        private readonly byte[] _buffer;

        public int Length { get { return _buffer.Length; } }

        public byte[] Data { get { return _buffer; } }

        public ByteBuffer(byte[] buffer)
        {
            _buffer = buffer;
        }

        protected void PutLittleEndian(int offset, byte[] data)
        {
            if (!BitConverter.IsLittleEndian)
            {
                data = data.Reverse().ToArray();
            }
            Buffer.BlockCopy(data, 0, _buffer, offset, data.Length);
        }

        protected byte[] ReadLittleEndian(int offset, int count)
        {
            AssertOffsetAndLength(offset, count);
            var tmp = new byte[count];
            Buffer.BlockCopy(_buffer, offset, tmp, 0, count);
            return (BitConverter.IsLittleEndian) ? tmp : tmp.Reverse().ToArray();
        }

        private void AssertOffsetAndLength(int offset, int length)
        {
            if (offset < 0 || offset >= _buffer.Length || offset + length > _buffer.Length)
                throw new ArgumentOutOfRangeException();
        }

        public void PutByte(int offset, byte value)
        {
            AssertOffsetAndLength(offset, sizeof(byte));
            _buffer[offset] = value;
        }

        public void PutShort(int offset, short value)
        {
            AssertOffsetAndLength(offset, sizeof(short));
            PutLittleEndian(offset, BitConverter.GetBytes(value));
        }

        public void PutInt(int offset, int value)
        {
            AssertOffsetAndLength(offset, sizeof(int));
            PutLittleEndian(offset, BitConverter.GetBytes(value));
        }

        public void PutLong(int offset, long value)
        {
            AssertOffsetAndLength(offset, sizeof(long));
            PutLittleEndian(offset, BitConverter.GetBytes(value));
        }

        public void PutFloat(int offset, float value)
        {
            AssertOffsetAndLength(offset, sizeof(float));
            PutLittleEndian(offset, BitConverter.GetBytes(value));
        }

        public void PutDouble(int offset, double value)
        {
            AssertOffsetAndLength(offset, sizeof(double));
            PutLittleEndian(offset, BitConverter.GetBytes(value));
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

        public int GetInt(int index)
        {
            var tmp = ReadLittleEndian(index, sizeof(int));
            var value = BitConverter.ToInt32(tmp, 0);
            return value;
        }

        public long GetLong(int index)
        {
            var tmp = ReadLittleEndian(index, sizeof(long));
            var value = BitConverter.ToInt64(tmp, 0);
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