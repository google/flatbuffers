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
using Google.FlatSpanBuffers.Operations;

namespace Google.FlatSpanBuffers
{
    public ref struct ByteSpanBuffer : IByteBuffer
    {
        private Span<byte> _buffer;
        private int _pos;

        public ByteSpanBuffer(Span<byte> buffer)
        {
            _buffer = buffer;
            _pos = 0;
        }

        public ByteSpanBuffer(ByteBuffer bb)
        {
            _buffer = bb.ToSizedSpan();
            _pos = 0;
        }

        // Position is the start of the encoded flatbuffer.
        public int Position { get => _pos; set => _pos = value; }
        public readonly int Length => _buffer.Length;

        public void Reset() => _pos = 0;

        public Span<byte> ToSpan(int pos, int len)
            => _buffer.Slice(pos, len);
        public ReadOnlySpan<byte> ToReadOnlySpan(int pos, int len)
            => _buffer.Slice(pos, len);
        public Span<byte> ToSizedSpan()
            => _buffer.Slice(Position, Length - Position);
        public ReadOnlySpan<byte> ToSizedReadOnlySpan()
            => _buffer.Slice(Position, Length - Position);
        public void PutByte(int offset, byte value)
            => _buffer[offset] = value;
        public void PutSbyte(int offset, sbyte value)
            => _buffer[offset] = (byte)value;
        public void PadBytes(int offset, int count)
            => BufferOperations.PadZeros(_buffer.Slice(offset, count));
        public void Put<T>(int offset, T value) where T : unmanaged
            => BufferOperations.Write(_buffer, offset, value);
        public void PutSpan<T>(int offset, scoped ReadOnlySpan<T> value) where T : unmanaged
            => BufferOperations.WriteSpan(_buffer, offset, value);
        public void PutStringUTF8(int offset, scoped ReadOnlySpan<char> value)
            => Encoding.UTF8.GetBytes(value, _buffer.Slice(offset));
        public byte Get(int index)
            => _buffer[index];
        public sbyte GetSbyte(int index)
            => (sbyte)_buffer[index];
        public T Get<T>(int offset) where T : unmanaged
            => BufferOperations.Read<T>(_buffer, offset);
        public Span<T> GetSpan<T>(int offset, int length) where T : unmanaged
           => BufferOperations.ReadSpan<T>(_buffer, offset, length);
        public ReadOnlySpan<T> GetReadOnlySpan<T>(int offset, int length) where T : unmanaged
          => BufferOperations.ReadSpan<T>(_buffer, offset, length);
        public string GetStringUTF8(int startPos, int len)
            => Encoding.UTF8.GetString(_buffer.Slice(startPos, len));
    }
}
