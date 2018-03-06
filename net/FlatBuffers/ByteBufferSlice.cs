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
using System.IO;

namespace FlatBuffers
{
    /// <summary>
    /// SubClass of ByteBuffer that adds slice behavior.
    /// </summary>
    public class ByteBufferSlice : ByteBuffer
    {
        private readonly int _off; // Parent's position if sliced

        public override int Length { get { return _buffer.Length - _off; } }

        public ByteBufferSlice(byte[] buffer, int pos, int off) : base(buffer, pos)
        {
            _off = off;
        }

        public override ByteBuffer Slice()
        {
            return new ByteBufferSlice(_buffer, 0, _off + Position);
        }

        public override void GrowFront(int newSize)
        {
            // We cannot grow slices, only the original buffers.
            // All current slices of this buffer will become invalid.
            throw new Exception("ByteBuffer: cannot grow slices.");
        }

        public override byte[] ToArray(int pos, int len)
        {
            return base.ToArray(_off + pos, len);
        }

        public override ArraySegment<byte> ToArraySegment(int pos, int len)
        {
            return base.ToArraySegment(_off + pos, len);
        }

        public override MemoryStream ToMemoryStream(int pos, int len)
        {
            return base.ToMemoryStream(_off + pos, len);
        }

        public override void PutSbyte(int offset, sbyte value)
        {
            base.PutSbyte(_off + offset, value);
        }

        public override void PutByte(int offset, byte value)
        {
            base.PutByte(_off + offset, value);
        }

        public override void PutByte(int offset, byte value, int count)
        {
            base.PutByte(_off + offset, value, count);
        }

        public override void PutStringUTF8(int offset, string value)
        {
            base.PutStringUTF8(_off + offset, value);
        }

#if UNSAFE_BYTEBUFFER
        public override unsafe void PutUshort(int offset, ushort value)
        {
            base.PutUshort(_off + offset, value);
        }

        public override unsafe void PutUint(int offset, uint value)
        {
            base.PutUint(_off + offset, value);
        }

        public override unsafe void PutUlong(int offset, ulong value)
        {
            base.PutUlong(_off + offset, value);
        }

        public override unsafe void PutFloat(int offset, float value)
        {
            base.PutFloat(_off + offset, value);
        }

        public override unsafe void PutDouble(int offset, double value)
        {
            base.PutDouble(_off + offset, value);
        }
#else // !UNSAFE_BYTEBUFFER
        public override void PutShort(int offset, short value)
        {
            base.PutShort(_off + offset, value);
        }

        public override void PutUshort(int offset, ushort value)
        {
            base.PutUshort(_off + offset, value);
        }

        public override void PutInt(int offset, int value)
        {
            base.PutInt(_off + offset, value);
        }

        public override void PutUint(int offset, uint value)
        {
            base.PutUint(_off + offset, value);
        }

        public override void PutLong(int offset, long value)
        {
            base.PutLong(_off + offset, value);
        }

        public override void PutUlong(int offset, ulong value)
        {
            base.PutUlong(_off + offset, value);
        }

        public override void PutFloat(int offset, float value)
        {
            base.PutFloat(_off + offset, value);
        }

        public override void PutDouble(int offset, double value)
        {
            base.PutDouble(_off + offset, value);
        }
#endif // UNSAFE_BYTEBUFFER

        public override sbyte GetSbyte(int index)
        {
            return base.GetSbyte(_off + index);
        }

        public override byte Get(int index)
        {
            return base.Get(_off + index);
        }

        public override string GetStringUTF8(int startPos, int len)
        {
            return base.GetStringUTF8(_off + startPos, len);
        }

#if UNSAFE_BYTEBUFFER
        public override unsafe ushort GetUshort(int offset)
        {
            return base.GetUshort(_off + offset);
        }

        public override unsafe uint GetUint(int offset)
        {
            return base.GetUint(_off + offset);
        }

        public override unsafe ulong GetUlong(int offset)
        {
            return base.GetUlong(_off + offset);
        }

        public override unsafe float GetFloat(int offset)
        {
            return base.GetFloat(_off + offset);
        }

        public override unsafe double GetDouble(int offset)
        {
            return base.GetDouble(_off + offset);
        }
#else // !UNSAFE_BYTEBUFFER
        public override short GetShort(int index)
        {
            return base.GetShort(_off + index);
        }

        public override ushort GetUshort(int index)
        {
            return base.GetUshort(_off + index);
        }

        public override int GetInt(int index)
        {
            return base.GetInt(_off + index);
        }

        public override uint GetUint(int index)
        {
            return base.GetUint(_off + index);
        }

        public override long GetLong(int index)
        {
           return base.GetLong(_off + index);
        }

        public override ulong GetUlong(int index)
        {
            return base.GetUlong(_off + index);
        }

        public override float GetFloat(int index)
        {
            return base.GetFloat(_off + index);
        }

        public override double GetDouble(int index)
        {
            return base.GetDouble(_off + index);
        }
#endif // UNSAFE_BYTEBUFFER
    }
}
