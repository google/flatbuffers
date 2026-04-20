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

namespace Google.FlatSpanBuffers
{
    /// <summary>
    /// Interface to expose select ByteBuffer operations to generic classes/functions.
    /// Intent is NOT to use this to store a ByteBuffer as these are structs (avoid boxing)
    /// </summary>
    public interface IByteBuffer
    {
        int Position { get; set; }
        int Length { get; }
        public void Reset();
        ReadOnlySpan<byte> ToReadOnlySpan(int start, int length);
        T Get<T>(int offset) where T : unmanaged;
        Span<T> GetSpan<T>(int offset, int length) where T : unmanaged;
        ReadOnlySpan<T> GetReadOnlySpan<T>(int offset, int length) where T : unmanaged;
        string GetStringUTF8(int startPos, int len);
        void Put<T>(int offset, T value) where T : unmanaged;
        void PadBytes(int offset, int count);
        void PutSpan<T>(int offset, scoped ReadOnlySpan<T> value) where T : unmanaged;
        public void PutStringUTF8(int offset, scoped ReadOnlySpan<char> value);
    }
}
