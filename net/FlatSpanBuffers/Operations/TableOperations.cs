/*
 * Copyright 2026 Google Inc. All rights reserved.
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

namespace Google.FlatSpanBuffers.Operations
{
    public static class TableOperations
    {
        public static int GetOffset<TBuffer>(int vtableOffset, int bbPos, TBuffer buffer)
            where TBuffer : IByteBuffer, allows ref struct
        {
            int vtable = bbPos - buffer.Get<int>(bbPos);
            return vtableOffset < buffer.Get<short>(vtable) ? buffer.Get<short>(vtable + vtableOffset) : 0;
        }

        public static int GetIndirect<TBuffer>(int offset, TBuffer buffer)
            where TBuffer : IByteBuffer, allows ref struct
        {
            return offset + buffer.Get<int>(offset);
        }

        public static string GetString<TBuffer>(int offset, TBuffer buffer)
            where TBuffer : IByteBuffer, allows ref struct
        {
            int stringOffset = buffer.Get<int>(offset);
            if (stringOffset == 0)
                return null;

            offset += stringOffset;
            var len = buffer.Get<int>(offset);
            var startPos = offset + sizeof(int);
            return buffer.GetStringUTF8(startPos, len);
        }

        public static Span<byte> GetStringBytes<TBuffer>(int offset, TBuffer buffer)
            where TBuffer : IByteBuffer, allows ref struct
        {
            int stringOffset = buffer.Get<int>(offset);
            if (stringOffset == 0)
                return Span<byte>.Empty;

            offset += stringOffset;
            var len = buffer.Get<int>(offset);
            var startPos = offset + sizeof(int);
            return buffer.GetSpan<byte>(startPos, len);
        }

        public static int GetVectorLength<TBuffer>(int offset, int bbPos, TBuffer buffer)
            where TBuffer : IByteBuffer, allows ref struct
        {
            offset += bbPos;
            offset += buffer.Get<int>(offset);
            return buffer.Get<int>(offset);
        }

        public static int GetVectorStart<TBuffer>(int offset, int bbPos, TBuffer buffer)
            where TBuffer : IByteBuffer, allows ref struct
        {
            offset += bbPos;
            return offset + buffer.Get<int>(offset) + sizeof(int);  // data starts after the length
        }

        public static bool HasIdentifier<TBuffer>(TBuffer buffer, string ident)
            where TBuffer : IByteBuffer, allows ref struct
        {
            return HasIdentifier(buffer, ident, buffer.Position);
        }

        public static bool HasIdentifier<TBuffer>(TBuffer buffer, string ident, int startPos)
            where TBuffer : IByteBuffer, allows ref struct
        {
            if (ident.Length != FlatBufferConstants.FileIdentifierLength)
                throw new ArgumentException("FlatBuffers: file identifier must be length " + FlatBufferConstants.FileIdentifierLength, nameof(ident));

            var idSpan = buffer.GetReadOnlySpan<byte>(
                startPos + sizeof(int), FlatBufferConstants.FileIdentifierLength);
            for (var i = 0; i < idSpan.Length; i++)
            {
                if (ident[i] != (char)idSpan[i])
                    return false;
            }
            
            return true;
        }

        public static Span<T> GetVectorAsSpan<T, TBuffer>(int offset, int bbPos, TBuffer buffer)
            where T : unmanaged
            where TBuffer : IByteBuffer, allows ref struct
        {
            var o = GetOffset(offset, bbPos, buffer);
            if (0 == o)
            {
                return new Span<T>();
            }

            var pos = GetVectorStart(o, bbPos, buffer);
            var len = GetVectorLength(o, bbPos, buffer);
            return buffer.GetSpan<T>(pos, len);
        }

        public static T GetUnion<T>(int offset, ByteBuffer buffer)
            where T : struct, IFlatbufferObject
        {
            T t = new T();
            t.__init(GetIndirect(offset, buffer), buffer);
            return t;
        }

        public static T GetUnionSpan<T>(int offset, ByteSpanBuffer buffer)
            where T : struct, IFlatbufferSpanObject, allows ref struct
        {
            T t = new T();
            t.__init(GetIndirect(offset, buffer), buffer);
            return t;
        }
    }
}