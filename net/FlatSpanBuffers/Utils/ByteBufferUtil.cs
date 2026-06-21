/*
 * Copyright 2017 Google Inc. All rights reserved.
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

namespace Google.FlatSpanBuffers.Utils
{
	/// <summary>
	/// Class that collects utility functions around `ByteBuffer`.
	/// </summary>
	public static class ByteBufferUtil
	{
		// Extract the size prefix from a `ByteBuffer`.
		public static int GetSizePrefix<TBuffer>(this TBuffer bb)
			where TBuffer : struct, IByteBuffer
		{
			return bb.Get<int>(bb.Position);
		}

		// Advance the position of the buffer past the size prefix.
		public static void RemoveSizePrefix<TBuffer>(this ref TBuffer bb)
			where TBuffer : struct, IByteBuffer
		{
			bb.Position += FlatBufferConstants.SizePrefixLength;
		}
		
		public static ByteSpanBuffer ToSizedByteSpanBuffer(this ByteBuffer bb)
        {
            return new ByteSpanBuffer(bb.ToSizedSpan());
        }
	}
}
