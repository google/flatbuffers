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
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using Google.FlatSpanBuffers.Operations;

namespace Google.FlatSpanBuffers
{
    public struct ByteBuffer : IByteBuffer
    {
        private byte[] _buffer;
        private int _pos;
        private int _length;

        public ByteBuffer(int size)
            : this(new byte[size]) { }

        public ByteBuffer(byte[] buffer)
            : this(buffer, 0) { }

        public ByteBuffer(byte[] buffer, int pos)
            : this(buffer, pos, buffer.Length) { }

        // Providing a length is helpful for tracking the 
        // actual length of an encoded FlatBuffer off the wire. 
        public ByteBuffer(byte[] buffer, int pos, int length)
        {
            if (buffer == null)
                throw new ArgumentNullException(nameof(buffer));

            _buffer = buffer;
            _pos = pos;
            _length = length;
        }

        // Position is the start of the encoded flatbuffer.
        public int Position { get => _pos; set => _pos = value; }
        public int Length => _length;
        public byte[] Buffer => _buffer;

        public void Reset() => _pos = 0;
        public void Reset(byte[] buffer, int pos, int length)
        {
            _buffer = buffer;
            _pos = pos;
            _length = length;
        }

        public Span<byte> ToSpan(int pos, int len)
            => _buffer.AsSpan(pos, len);
        public ReadOnlySpan<byte> ToReadOnlySpan(int pos, int len)
            => _buffer.AsSpan(pos, len);
        public Span<byte> ToSizedSpan()
            => _buffer.AsSpan(Position, Length - Position);
        public ReadOnlySpan<byte> ToSizedReadOnlySpan()
            => _buffer.AsSpan(Position, Length - Position);
        public void PutByte(int offset, byte value)
            => _buffer[offset] = value;
        public void PutSbyte(int offset, sbyte value)
            => _buffer[offset] = (byte)value;
        public void PadBytes(int offset, int count)
            => BufferOperations.PadZeros(_buffer.AsSpan(offset, count));
        public void Put<T>(int offset, T value) where T : unmanaged
            => BufferOperations.Write(_buffer, offset, value);
        public void PutSpan<T>(int offset, scoped ReadOnlySpan<T> value) where T : unmanaged
            => BufferOperations.WriteSpan(_buffer, offset, value);
        public void PutStringUTF8(int offset, scoped ReadOnlySpan<char> value)
            => Encoding.UTF8.GetBytes(value, _buffer.AsSpan(offset));
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
            => Encoding.UTF8.GetString(_buffer.AsSpan(startPos, len));
    }
}
