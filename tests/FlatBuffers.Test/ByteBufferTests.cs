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
    public class ByteBufferTests
    {

        public void ByteBuffer_Length_MatchesBufferLength()
        {
            var buffer = new byte[1000];
            var uut = new ByteBuffer(buffer);
            Assert.AreEqual(buffer.Length, uut.Length);
        }

        public void ByteBuffer_PutBytePopulatesBufferAtZeroOffset()
        {
            var buffer = new byte[1];
            var uut = new ByteBuffer(buffer);
            uut.PutByte(0, (byte)99);

            Assert.AreEqual((byte)99, buffer[0]);
        }

        public void ByteBuffer_PutByteCannotPutAtOffsetPastLength()
        {
            var buffer = new byte[1];
            var uut = new ByteBuffer(buffer);
            Assert.Throws<ArgumentOutOfRangeException>(() => uut.PutByte(1, 99));
        }

        public void ByteBuffer_PutShortPopulatesBufferCorrectly()
        {
            var buffer = new byte[2];
            var uut = new ByteBuffer(buffer);
            uut.PutShort(0, (short)1);

            // Ensure Endianness was written correctly
            Assert.AreEqual((byte)1, buffer[0]);
            Assert.AreEqual((byte)0, buffer[1]);
        }

        public void ByteBuffer_PutShortCannotPutAtOffsetPastLength()
        {
            var buffer = new byte[2];
            var uut = new ByteBuffer(buffer);
            Assert.Throws<ArgumentOutOfRangeException>(() => uut.PutShort(2, 99));
        }

        public void ByteBuffer_PutShortChecksLength()
        {
            var buffer = new byte[1];
            var uut = new ByteBuffer(buffer);
            Assert.Throws<ArgumentOutOfRangeException>(() => uut.PutShort(0, 99));
        }

        public void ByteBuffer_PutShortChecksLengthAndOffset()
        {
            var buffer = new byte[2];
            var uut = new ByteBuffer(buffer);
            Assert.Throws<ArgumentOutOfRangeException>(() => uut.PutShort(1, 99));
        }

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

        public void ByteBuffer_PutIntCannotPutAtOffsetPastLength()
        {
            var buffer = new byte[4];
            var uut = new ByteBuffer(buffer);
            Assert.Throws<ArgumentOutOfRangeException>(() => uut.PutInt(2, 0x0A0B0C0D));
        }

        public void ByteBuffer_PutIntChecksLength()
        {
            var buffer = new byte[1];
            var uut = new ByteBuffer(buffer);
            Assert.Throws<ArgumentOutOfRangeException>(() => uut.PutInt(0, 0x0A0B0C0D));
        }

        public void ByteBuffer_PutIntChecksLengthAndOffset()
        {
            var buffer = new byte[4];
            var uut = new ByteBuffer(buffer);
            Assert.Throws<ArgumentOutOfRangeException>(() => uut.PutInt(2, 0x0A0B0C0D));
        }

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

        public void ByteBuffer_PutLongCannotPutAtOffsetPastLength()
        {
            var buffer = new byte[8];
            var uut = new ByteBuffer(buffer);
            Assert.Throws<ArgumentOutOfRangeException>(() => uut.PutLong(2, 0x010203040A0B0C0D));
        }

        public void ByteBuffer_PutLongChecksLength()
        {
            var buffer = new byte[1];
            var uut = new ByteBuffer(buffer);
            Assert.Throws<ArgumentOutOfRangeException>(() => uut.PutLong(0, 0x010203040A0B0C0D));
        }

        public void ByteBuffer_PutLongChecksLengthAndOffset()
        {
            var buffer = new byte[8];
            var uut = new ByteBuffer(buffer);
            Assert.Throws<ArgumentOutOfRangeException>(() => uut.PutLong(2, 0x010203040A0B0C0D));
        }

        public void ByteBuffer_GetByteReturnsCorrectData()
        {
            var buffer = new byte[1];
            buffer[0] = 99;
            var uut = new ByteBuffer(buffer);
            Assert.AreEqual((byte)99, uut.Get(0));
        }

        public void ByteBuffer_GetByteChecksOffset()
        {
            var buffer = new byte[1];
            var uut = new ByteBuffer(buffer);
            Assert.Throws<ArgumentOutOfRangeException>(()=>uut.Get(1));
        }

        public void ByteBuffer_GetShortReturnsCorrectData()
        {
            var buffer = new byte[2];
            buffer[0] = 1;
            buffer[1] = 0;
            var uut = new ByteBuffer(buffer);
            Assert.AreEqual(1, uut.GetShort(0));
        }

        public void ByteBuffer_GetShortChecksOffset()
        {
            var buffer = new byte[2];
            var uut = new ByteBuffer(buffer);
            Assert.Throws<ArgumentOutOfRangeException>(() => uut.GetShort(2));
        }

        public void ByteBuffer_GetShortChecksLength()
        {
            var buffer = new byte[2];
            var uut = new ByteBuffer(buffer);
            Assert.Throws<ArgumentOutOfRangeException>(() => uut.GetShort(1));
        }

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

        public void ByteBuffer_GetIntChecksOffset()
        {
            var buffer = new byte[4];
            var uut = new ByteBuffer(buffer);
            Assert.Throws<ArgumentOutOfRangeException>(() => uut.GetInt(4));
        }

        public void ByteBuffer_GetIntChecksLength()
        {
            var buffer = new byte[2];
            var uut = new ByteBuffer(buffer);
            Assert.Throws<ArgumentOutOfRangeException>(() => uut.GetInt(0));
        }

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

        public void ByteBuffer_GetLongChecksOffset()
        {
            var buffer = new byte[8];
            var uut = new ByteBuffer(buffer);
            Assert.Throws<ArgumentOutOfRangeException>(() => uut.GetLong(8));
        }

        public void ByteBuffer_GetLongChecksLength()
        {
            var buffer = new byte[7];
            var uut = new ByteBuffer(buffer);
            Assert.Throws<ArgumentOutOfRangeException>(() => uut.GetLong(0));
        }

    }
}
