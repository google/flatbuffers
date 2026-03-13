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
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Google.FlatSpanBuffers.Operations
{
    // This class consolidates valid type checks and buffer read/write operations w/ big endian handling.
    // Unsafe calls are used carefully here. WriteUnaligned and ReadUnaligned are 'ok' because we do bounds checks with span slices. 
    public static class BufferOperations
    {
        // The intent of this call is to validate type validity for the buffer operations.
        // Expectation is that this call will be omitted by the JIT if the type is valid.
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        private static void ValidateType<T>()
            where T : unmanaged
        {
            // The unmanaged constraint includes the types flatbuffers supports, including enums.
            // However, some undesirable types can pass this check (unmanaged structs).
            // Use size as an additional validity check to keep most of the unsupported types out.
            int size = Unsafe.SizeOf<T>();
            if (size != 1 && size != 2 && size != 4 && size != 8)
                ThrowNotSupported();
        }

        [MethodImpl(MethodImplOptions.NoInlining)]
        private static void ThrowNotSupported()
        {
            throw new NotSupportedException("Type not supported by buffer operations");
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static int SizeOf<T>()
            where T : unmanaged
        {
            ValidateType<T>();
            return Unsafe.SizeOf<T>();
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static T Read<T>(ReadOnlySpan<byte> buffer, int offset)
            where T : unmanaged
        {
            ValidateType<T>();
            var src = buffer.Slice(offset, Unsafe.SizeOf<T>());
            T value = Unsafe.ReadUnaligned<T>(ref MemoryMarshal.GetReference(src));

            if (!BitConverter.IsLittleEndian)
            {
                Span<byte> valueBytes = MemoryMarshal.AsBytes(
                    MemoryMarshal.CreateSpan(ref value, 1));
                valueBytes.Reverse();
            }

            return value;
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static void Write<T>(Span<byte> buffer, int offset, T value)
            where T : unmanaged
        {
            ValidateType<T>();
            var dest = buffer.Slice(offset, Unsafe.SizeOf<T>());
            Unsafe.WriteUnaligned(ref MemoryMarshal.GetReference(dest), value);

            if (!BitConverter.IsLittleEndian)
            {
                dest.Reverse();
            }
        }

        public static Span<T> ReadSpan<T>(Span<byte> buffer, int offset, int length)
            where T : unmanaged
        {
            ValidateType<T>();
            var byteLength = Unsafe.SizeOf<T>() * length;
            var dataRead = buffer.Slice(offset, byteLength);

            if (!BitConverter.IsLittleEndian)
            {
                int size = Unsafe.SizeOf<T>();
                if (size > 1)
                {
                    Span<byte> reversed = new byte[dataRead.Length];
                    dataRead.CopyTo(reversed);

                    for (int i = 0; i < reversed.Length; i += size)
                    {
                        reversed.Slice(i, size).Reverse();
                    }

                    dataRead = reversed;
                }
            }

            return MemoryMarshal.Cast<byte, T>(dataRead);
        }

        public static void WriteSpan<T>(Span<byte> buffer, int offset, scoped ReadOnlySpan<T> data)
            where T : unmanaged
        {
            ValidateType<T>();
            if (data.Length == 0)
                return;

            ReadOnlySpan<byte> dataBytes = MemoryMarshal.AsBytes(data);
            Span<byte> dest = buffer.Slice(offset, dataBytes.Length);
            dataBytes.CopyTo(dest);

            if (!BitConverter.IsLittleEndian)
            {
                int size = Unsafe.SizeOf<T>();
                if (size > 1)
                {
                    for (int i = 0; i < dest.Length; i += size)
                    {
                        dest.Slice(i, size).Reverse();
                    }
                }
            }
        }

        // Created optimized version of 'Fill(0)' because FlatBuffers pads only a few bytes at a time. 
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static void PadZeros(Span<byte> buffer)
        {
            // The Length keeps us safe with WriteUnaligned.
            switch (buffer.Length)
            {
                case 0: return;
                case 1: buffer[0] = 0; return;
                case 2: Unsafe.WriteUnaligned<short>(ref buffer[0], 0); return;
                case 3: Unsafe.WriteUnaligned<short>(ref buffer[0], 0); buffer[2] = 0; return;
                case 4: Unsafe.WriteUnaligned<int>(ref buffer[0], 0); return;
                case 5: Unsafe.WriteUnaligned<int>(ref buffer[0], 0); buffer[4] = 0; return;
                case 6: Unsafe.WriteUnaligned<int>(ref buffer[0], 0); Unsafe.WriteUnaligned<short>(ref buffer[4], 0); return;
                case 7: Unsafe.WriteUnaligned<int>(ref buffer[0], 0); Unsafe.WriteUnaligned<short>(ref buffer[4], 0); buffer[6] = 0; return;
                default:
                    buffer.Fill(0);
                    return;
            }
        }
    }
}
