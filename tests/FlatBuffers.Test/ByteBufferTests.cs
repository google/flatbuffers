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

namespace FlatBuffers.Test
{
    [FlatBuffersTestClass]
    public class ByteBufferTests
    {

        [FlatBuffersTestMethod]
        public void ByteBuffer_Length_MatchesBufferLength()
        {
            var buffer = new byte[1000];
            var uut = new ByteBuffer(buffer);
            Assert.AreEqual(buffer.Length, uut.Length);
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_PutBytePopulatesBufferAtZeroOffset()
        {
            var buffer = new byte[1];
            var uut = new ByteBuffer(buffer);
            uut.PutByte(0, (byte)99);

            Assert.AreEqual((byte)99, buffer[0]);
        }

#if !BYTEBUFFER_NO_BOUNDS_CHECK
        [FlatBuffersTestMethod]
        public void ByteBuffer_PutByteCannotPutAtOffsetPastLength()
        {
            var uut = new ByteBuffer(1);
            Assert.Throws<ArgumentOutOfRangeException>(() => uut.PutByte(1, 99));
        }
#endif

        [FlatBuffersTestMethod]
        public void ByteBuffer_PutShortPopulatesBufferCorrectly()
        {
            var buffer = new byte[2];
            var uut = new ByteBuffer(buffer);
            uut.PutShort(0, (short)1);

            // Ensure Endianness was written correctly
            Assert.AreEqual((byte)1, buffer[0]);
            Assert.AreEqual((byte)0, buffer[1]);
        }

#if !BYTEBUFFER_NO_BOUNDS_CHECK
        [FlatBuffersTestMethod]
        public void ByteBuffer_PutShortCannotPutAtOffsetPastLength()
        {
            var uut = new ByteBuffer(2);
            Assert.Throws<ArgumentOutOfRangeException>(() => uut.PutShort(2, 99));
        }
#endif

#if !BYTEBUFFER_NO_BOUNDS_CHECK
        [FlatBuffersTestMethod]
        public void ByteBuffer_PutShortChecksLength()
        {
            var uut = new ByteBuffer(1);
            Assert.Throws<ArgumentOutOfRangeException>(() => uut.PutShort(0, 99));
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_PutShortChecksLengthAndOffset()
        {
            var uut = new ByteBuffer(2);
            Assert.Throws<ArgumentOutOfRangeException>(() => uut.PutShort(1, 99));
        }
#endif

        [FlatBuffersTestMethod]
        public void ByteBuffer_PutIntPopulatesBufferCorrectly()
        {
            var buffer = new byte[4];
            var uut = new ByteBuffer(buffer);
            uut.PutInt(0, 0x0A0B0C0D);

            // Ensure Endianness was written correctly
            Assert.AreEqual(0x0D, buffer[0]);
            Assert.AreEqual(0x0C, buffer[1]);
            Assert.AreEqual(0x0B, buffer[2]);
            Assert.AreEqual(0x0A, buffer[3]);
        }

 #if !BYTEBUFFER_NO_BOUNDS_CHECK
        [FlatBuffersTestMethod]
        public void ByteBuffer_PutIntCannotPutAtOffsetPastLength()
        {
            var uut = new ByteBuffer(4);
            Assert.Throws<ArgumentOutOfRangeException>(() => uut.PutInt(2, 0x0A0B0C0D));
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_PutIntChecksLength()
        {
            var uut = new ByteBuffer(1);
            Assert.Throws<ArgumentOutOfRangeException>(() => uut.PutInt(0, 0x0A0B0C0D));
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_PutIntChecksLengthAndOffset()
        {
            var uut = new ByteBuffer(4);
            Assert.Throws<ArgumentOutOfRangeException>(() => uut.PutInt(2, 0x0A0B0C0D));
        }
#endif

        [FlatBuffersTestMethod]
        public void ByteBuffer_PutLongPopulatesBufferCorrectly()
        {
            var buffer = new byte[8];
            var uut = new ByteBuffer(buffer);
            uut.PutLong(0, 0x010203040A0B0C0D);

            // Ensure Endianness was written correctly
            Assert.AreEqual(0x0D, buffer[0]);
            Assert.AreEqual(0x0C, buffer[1]);
            Assert.AreEqual(0x0B, buffer[2]);
            Assert.AreEqual(0x0A, buffer[3]);
            Assert.AreEqual(0x04, buffer[4]);
            Assert.AreEqual(0x03, buffer[5]);
            Assert.AreEqual(0x02, buffer[6]);
            Assert.AreEqual(0x01, buffer[7]);
        }

#if !BYTEBUFFER_NO_BOUNDS_CHECK
        [FlatBuffersTestMethod]
        public void ByteBuffer_PutLongCannotPutAtOffsetPastLength()
        {
            var uut = new ByteBuffer(8);
            Assert.Throws<ArgumentOutOfRangeException>(() => uut.PutLong(2, 0x010203040A0B0C0D));
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_PutLongChecksLength()
        {
            var uut = new ByteBuffer(1);
            Assert.Throws<ArgumentOutOfRangeException>(() => uut.PutLong(0, 0x010203040A0B0C0D));
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_PutLongChecksLengthAndOffset()
        {
            var uut = new ByteBuffer(8);
            Assert.Throws<ArgumentOutOfRangeException>(() => uut.PutLong(2, 0x010203040A0B0C0D));
        }
#endif

        [FlatBuffersTestMethod]
        public void ByteBuffer_GetByteReturnsCorrectData()
        {
            var buffer = new byte[1];
            buffer[0] = 99;
            var uut = new ByteBuffer(buffer);
            Assert.AreEqual((byte)99, uut.Get(0));
        }

#if !BYTEBUFFER_NO_BOUNDS_CHECK
        [FlatBuffersTestMethod]
        public void ByteBuffer_GetByteChecksOffset()
        {
            var uut = new ByteBuffer(1);
            Assert.Throws<ArgumentOutOfRangeException>(()=>uut.Get(1));
        }
#endif

        [FlatBuffersTestMethod]
        public void ByteBuffer_GetShortReturnsCorrectData()
        {
            var buffer = new byte[2];
            buffer[0] = 1;
            buffer[1] = 0;
            var uut = new ByteBuffer(buffer);
            Assert.AreEqual(1, uut.GetShort(0));
        }

#if !BYTEBUFFER_NO_BOUNDS_CHECK
        [FlatBuffersTestMethod]
        public void ByteBuffer_GetShortChecksOffset()
        {
            var uut = new ByteBuffer(2);
            Assert.Throws<ArgumentOutOfRangeException>(() => uut.GetShort(2));
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_GetShortChecksLength()
        {
            var uut = new ByteBuffer(2);
            Assert.Throws<ArgumentOutOfRangeException>(() => uut.GetShort(1));
        }
#endif

        [FlatBuffersTestMethod]
        public void ByteBuffer_GetIntReturnsCorrectData()
        {
            var buffer = new byte[4];
            buffer[0] = 0x0D;
            buffer[1] = 0x0C;
            buffer[2] = 0x0B;
            buffer[3] = 0x0A;
            var uut = new ByteBuffer(buffer);
            Assert.AreEqual(0x0A0B0C0D, uut.GetInt(0));
        }

#if !BYTEBUFFER_NO_BOUNDS_CHECK
        [FlatBuffersTestMethod]
        public void ByteBuffer_GetIntChecksOffset()
        {
            var uut = new ByteBuffer(4);
            Assert.Throws<ArgumentOutOfRangeException>(() => uut.GetInt(4));
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_GetIntChecksLength()
        {
            var uut = new ByteBuffer(2);
            Assert.Throws<ArgumentOutOfRangeException>(() => uut.GetInt(0));
        }
#endif

        [FlatBuffersTestMethod]
        public void ByteBuffer_GetLongReturnsCorrectData()
        {
            var buffer = new byte[8];
            buffer[0] = 0x0D;
            buffer[1] = 0x0C;
            buffer[2] = 0x0B;
            buffer[3] = 0x0A;
            buffer[4] = 0x04;
            buffer[5] = 0x03;
            buffer[6] = 0x02;
            buffer[7] = 0x01;
            var uut = new ByteBuffer(buffer);
            Assert.AreEqual(0x010203040A0B0C0D, uut.GetLong(0));
        }

#if !BYTEBUFFER_NO_BOUNDS_CHECK
        [FlatBuffersTestMethod]
        public void ByteBuffer_GetLongChecksOffset()
        {
            var uut = new ByteBuffer(8);
            Assert.Throws<ArgumentOutOfRangeException>(() => uut.GetLong(8));
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_GetLongChecksLength()
        {
            var uut = new ByteBuffer(7);
            Assert.Throws<ArgumentOutOfRangeException>(() => uut.GetLong(0));
        }
#endif

        [FlatBuffersTestMethod]
        public void ByteBuffer_ReverseBytesUshort()
        {
            const ushort original = (ushort)0x1234U;
            var reverse = ByteBuffer.ReverseBytes(original);
            Assert.AreEqual(0x3412U, reverse);

            var rereverse = ByteBuffer.ReverseBytes(reverse);
            Assert.AreEqual(original, rereverse);
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_ReverseBytesUint()
        {
            const uint original = 0x12345678;
            var reverse = ByteBuffer.ReverseBytes(original);
            Assert.AreEqual(0x78563412U, reverse);

            var rereverse = ByteBuffer.ReverseBytes(reverse);
            Assert.AreEqual(original, rereverse);
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_ReverseBytesUlong()
        {
            const ulong original = 0x1234567890ABCDEFUL;
            var reverse = ByteBuffer.ReverseBytes(original);
            Assert.AreEqual(0xEFCDAB9078563412UL, reverse);

            var rereverse = ByteBuffer.ReverseBytes(reverse);
            Assert.AreEqual(original, rereverse);
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_ToFullArray_MatchesBuffer()
        {
            var buffer = new byte[4];
            buffer[0] = 0x0D;
            buffer[1] = 0x0C;
            buffer[2] = 0x0B;
            buffer[3] = 0x0A;
            var uut = new ByteBuffer(buffer);
            Assert.ArrayEqual(buffer, uut.ToFullArray());
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_ToSizedArray_MatchesBuffer()
        {
            var buffer = new byte[4];
            buffer[0] = 0x0D;
            buffer[1] = 0x0C;
            buffer[2] = 0x0B;
            buffer[3] = 0x0A;
            var uut = new ByteBuffer(buffer);
            Assert.ArrayEqual(buffer, uut.ToFullArray());
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_Duplicate_MatchesBuffer()
        {
            var buffer = new byte[4];
            buffer[0] = 0x0D;
            buffer[1] = 0x0C;
            buffer[2] = 0x0B;
            buffer[3] = 0x0A;
            var uut = new ByteBuffer(buffer);
            Assert.AreEqual(0x0A0B0C0D, uut.GetInt(0));

            // Advance by two bytes
            uut.Position = 2; uut = uut.Duplicate();
            Assert.AreEqual(0x0A0B, uut.GetShort(2));

            // Advance by one more byte
            uut.Position = 1; uut = uut.Duplicate();
            Assert.AreEqual(0x0A, uut.Get(3));
        }
    }
}
