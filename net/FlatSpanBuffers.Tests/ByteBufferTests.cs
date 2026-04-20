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

namespace Google.FlatSpanBuffers.Tests
{
   // Copied from: tests/FlatBuffers.Test/ByteBufferTests.cs
   // Adapted for Google.FlatSpanBuffers namespace and API
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

        [FlatBuffersTestMethod]
        public void ByteBuffer_PutByteCannotPutAtOffsetPastLength()
        {
            var uut = new ByteBuffer(1);
            Assert.Throws<IndexOutOfRangeException>(() => uut.PutByte(1, 99));
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_PutShortPopulatesBufferCorrectly()
        {
            var buffer = new byte[2];
            var uut = new ByteBuffer(buffer);
            uut.Put<short>(0, (short)1);

            // Ensure Endianness was written correctly
            Assert.AreEqual((byte)1, buffer[0]);
            Assert.AreEqual((byte)0, buffer[1]);
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_PutShortCannotPutAtOffsetPastLength()
        {
            var uut = new ByteBuffer(2);
            Assert.Throws<ArgumentOutOfRangeException>(() => uut.Put<short>(2, 99));
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_PutShortChecksLength()
        {
            var uut = new ByteBuffer(1);
            Assert.Throws<ArgumentOutOfRangeException>(() => uut.Put<short>(0, 99));
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_PutShortChecksLengthAndOffset()
        {
            var uut = new ByteBuffer(2);
            Assert.Throws<ArgumentOutOfRangeException>(() => uut.Put<short>(1, 99));
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_PutIntPopulatesBufferCorrectly()
        {
            var buffer = new byte[4];
            var uut = new ByteBuffer(buffer);
            uut.Put<int>(0, 0x0A0B0C0D);

            // Ensure Endianness was written correctly
            Assert.AreEqual((byte)0x0D, buffer[0]);
            Assert.AreEqual((byte)0x0C, buffer[1]);
            Assert.AreEqual((byte)0x0B, buffer[2]);
            Assert.AreEqual((byte)0x0A, buffer[3]);
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_PutIntCannotPutAtOffsetPastLength()
        {
            var uut = new ByteBuffer(4);
            Assert.Throws<ArgumentOutOfRangeException>(() => uut.Put<int>(2, 0x0A0B0C0D));
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_PutIntChecksLength()
        {
            var uut = new ByteBuffer(1);
            Assert.Throws<ArgumentOutOfRangeException>(() => uut.Put<int>(0, 0x0A0B0C0D));
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_PutIntChecksLengthAndOffset()
        {
            var uut = new ByteBuffer(4);
            Assert.Throws<ArgumentOutOfRangeException>(() => uut.Put<int>(2, 0x0A0B0C0D));
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_PutLongPopulatesBufferCorrectly()
        {
            var buffer = new byte[8];
            var uut = new ByteBuffer(buffer);
            uut.Put<long>(0, 0x010203040A0B0C0D);

            // Ensure Endianness was written correctly
            Assert.AreEqual((byte)0x0D, buffer[0]);
            Assert.AreEqual((byte)0x0C, buffer[1]);
            Assert.AreEqual((byte)0x0B, buffer[2]);
            Assert.AreEqual((byte)0x0A, buffer[3]);
            Assert.AreEqual((byte)0x04, buffer[4]);
            Assert.AreEqual((byte)0x03, buffer[5]);
            Assert.AreEqual((byte)0x02, buffer[6]);
            Assert.AreEqual((byte)0x01, buffer[7]);
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_PutLongCannotPutAtOffsetPastLength()
        {
            var uut = new ByteBuffer(8);
            Assert.Throws<ArgumentOutOfRangeException>(() => uut.Put<long>(2, 0x010203040A0B0C0D));
        }

       [FlatBuffersTestMethod]
        public void ByteBuffer_PutLongChecksLength()
        {
            var uut = new ByteBuffer(1);
            Assert.Throws<ArgumentOutOfRangeException>(() => uut.Put<long>(0, 0x010203040A0B0C0D));
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_PutLongChecksLengthAndOffset()
        {
            var uut = new ByteBuffer(8);
            Assert.Throws<ArgumentOutOfRangeException>(() => uut.Put<long>(2, 0x010203040A0B0C0D));
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_GetByteReturnsCorrectData()
        {
            var buffer = new byte[1];
            buffer[0] = 99;
            var uut = new ByteBuffer(buffer);
            Assert.AreEqual((byte)99, uut.Get(0));
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_GetByteChecksOffset()
        {
            var uut = new ByteBuffer(1);
            Assert.Throws<IndexOutOfRangeException>(() => uut.Get(1));
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_GetShortReturnsCorrectData()
        {
            var buffer = new byte[2];
            buffer[0] = 1;
            buffer[1] = 0;
            var uut = new ByteBuffer(buffer);
            Assert.AreEqual((short)1, uut.Get<short>(0));
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_GetShortChecksOffset()
        {
            var uut = new ByteBuffer(2);
            Assert.Throws<ArgumentOutOfRangeException>(() => uut.Get<short>(2));
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_GetShortChecksLength()
        {
            var uut = new ByteBuffer(2);
            Assert.Throws<ArgumentOutOfRangeException>(() => uut.Get<short>(1));
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_GetIntReturnsCorrectData()
        {
            var buffer = new byte[4];
            buffer[0] = 0x0D;
            buffer[1] = 0x0C;
            buffer[2] = 0x0B;
            buffer[3] = 0x0A;
            var uut = new ByteBuffer(buffer);
            Assert.AreEqual(0x0A0B0C0D, uut.Get<int>(0));
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_GetIntChecksOffset()
        {
            var uut = new ByteBuffer(4);
            Assert.Throws<ArgumentOutOfRangeException>(() => uut.Get<int>(4));
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_GetIntChecksLength()
        {
            var uut = new ByteBuffer(2);
            Assert.Throws<ArgumentOutOfRangeException>(() => uut.Get<int>(0));
        }

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
            Assert.AreEqual(0x010203040A0B0C0D, uut.Get<long>(0));
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_GetLongChecksOffset()
        {
            var uut = new ByteBuffer(8);
            Assert.Throws<ArgumentOutOfRangeException>(() => uut.Get<long>(8));
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_GetLongChecksLength()
        {
            var uut = new ByteBuffer(7);
            Assert.Throws<ArgumentOutOfRangeException>(() => uut.Get<long>(0));
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

            var fullArray = uut.ToSpan(0, buffer.Length);
            Assert.SpanEqual<byte>(buffer, fullArray);
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

            var sizedArray = uut.ToSizedSpan();
            Assert.SpanEqual<byte>(buffer, sizedArray);
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
            Assert.AreEqual(0x0A0B0C0D, uut.Get<int>(0));

            uut.Position = 2;
            var dup = new ByteBuffer(buffer, uut.Position, buffer.Length);
            Assert.AreEqual(0x0A0B, dup.Get<short>(2));

            dup.Position = 1;
            var dup2 = new ByteBuffer(buffer, dup.Position, buffer.Length);
            Assert.AreEqual(0x0A, dup2.Get(3));
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_To_Array_Float()
        {
            const int len = 9;

            var fData = new float[len];
            fData[0] = 1.0079F;
            fData[1] = 4.0026F;
            fData[2] = 6.941F;
            fData[3] = 9.0122F;
            fData[4] = 10.811F;
            fData[5] = 12.0107F;
            fData[6] = 14.0067F;
            fData[7] = 15.9994F;
            fData[8] = 18.9984F;

            var buffer = new byte[sizeof(float) * fData.Length];
            Buffer.BlockCopy(fData, 0, buffer, 0, buffer.Length);

            var uut = new ByteBuffer(buffer);

            var bbSpan = uut.GetSpan<float>(0, len);
            Assert.SpanEqual<float>(fData, bbSpan);

            var bbSpan2 = uut.GetSpan<float>(sizeof(float), len - 1);
            Assert.AreEqual(bbSpan2.Length, len - 1);
            for (int i = 1; i < len - 1; i++)
            {
                Assert.AreEqual(fData[i], bbSpan2[i - 1]);
            }

            var bbSpan3 = uut.GetSpan<float>(sizeof(float) * 2, len - 4);
            Assert.AreEqual(bbSpan3.Length, len - 4);
            for (int i = 2; i < len - 4; i++)
            {
                Assert.AreEqual(fData[i], bbSpan3[i - 2]);
            }
        }

        private static void ByteBuffer_Put_Span_Helper<T>(ReadOnlySpan<T> data, int typeSize)
            where T : unmanaged, IEquatable<T>
        {
            // Create the Byte Buffer
            var uut = new ByteBuffer(1024);

            // Put the data into the buffer and make sure the offset is
            // calculated correctly
            int nOffset = 1024 - typeSize * data.Length;
            uut.PutSpan(nOffset, data);

            // Get the full span back out and ensure they are the same
            var bbSpan = uut.GetSpan<T>(nOffset, data.Length);
            Assert.SpanEqual(data, bbSpan);
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_Put_Array_Float()
        {
            const int len = 9;

            // Construct the data array
            Span<float> data = stackalloc float[len];
            data[0] = 1.0079F;
            data[1] = 4.0026F;
            data[2] = 6.941F;
            data[3] = 9.0122F;
            data[4] = 10.811F;
            data[5] = 12.0107F;
            data[6] = 14.0067F;
            data[7] = 15.9994F;
            data[8] = 18.9984F;

            ByteBuffer_Put_Span_Helper<float>(data, sizeof(float));
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_Put_Array_Double()
        {
            const int len = 9;

            // Construct the data array
            Span<double> data = stackalloc double[len];
            data[0] = 1.0079;
            data[1] = 4.0026;
            data[2] = 6.941;
            data[3] = 9.0122;
            data[4] = 10.811;
            data[5] = 12.0107;
            data[6] = 14.0067;
            data[7] = 15.9994;
            data[8] = 18.9984;

            ByteBuffer_Put_Span_Helper<double>(data, sizeof(double));
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_Put_Array_Int()
        {
            const int len = 9;

            // Construct the data array
            Span<int> data = stackalloc int[len];
            data[0] = 1;
            data[1] = 4;
            data[2] = 6;
            data[3] = 9;
            data[4] = 10;
            data[5] = 12;
            data[6] = 14;
            data[7] = 15;
            data[8] = 18;

            ByteBuffer_Put_Span_Helper<int>(data, sizeof(int));
        }


        [FlatBuffersTestMethod]
        public void ByteBuffer_Put_Array_UInt()
        {
            const int len = 9;

            // Construct the data array
            Span<uint> data = stackalloc uint[len];
            data[0] = 1;
            data[1] = 4;
            data[2] = 6;
            data[3] = 9;
            data[4] = 10;
            data[5] = 12;
            data[6] = 14;
            data[7] = 15;
            data[8] = 18;

            ByteBuffer_Put_Span_Helper<uint>(data, sizeof(uint));
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_Put_Array_Bool()
        {
            const int len = 9;

            // Construct the data array
            Span<bool> data = stackalloc bool[len];
            data[0] = true;
            data[1] = true;
            data[2] = false;
            data[3] = true;
            data[4] = false;
            data[5] = true;
            data[6] = true;
            data[7] = true;
            data[8] = false;

            ByteBuffer_Put_Span_Helper<bool>(data, sizeof(bool));
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_Put_Array_Long()
        {
            const int len = 9;

            // Construct the data array
            Span<long> data = stackalloc long[len];
            data[0] = 1;
            data[1] = 4;
            data[2] = 6;
            data[3] = 9;
            data[4] = 10;
            data[5] = 12;
            data[6] = 14;
            data[7] = 15;
            data[8] = 18;

            ByteBuffer_Put_Span_Helper<long>(data, sizeof(long));
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_Put_Array_Byte()
        {
            const int len = 9;

            // Construct the data array
            Span<byte> data = stackalloc byte[len];
            data[0] = 1;
            data[1] = 4;
            data[2] = 6;
            data[3] = 9;
            data[4] = 10;
            data[5] = 12;
            data[6] = 14;
            data[7] = 15;
            data[8] = 18;

            ByteBuffer_Put_Span_Helper<byte>(data, sizeof(byte));
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_Put_Array_SByte()
        {
            const int len = 9;

            // Construct the data array
            Span<sbyte> data = stackalloc sbyte[len];
            data[0] = 1;
            data[1] = 4;
            data[2] = 6;
            data[3] = 9;
            data[4] = 10;
            data[5] = 12;
            data[6] = 14;
            data[7] = 15;
            data[8] = 18;

            ByteBuffer_Put_Span_Helper<sbyte>(data, sizeof(sbyte));
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_Get_Double()
        {
            var uut = new ByteBuffer(1024);
            double value = 3.14159265;
            uut.Put<double>(900, value);
            double getValue = uut.Get<double>(900);
            Assert.AreEqual(value, getValue);
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_Get_Float()
        {
            var uut = new ByteBuffer(1024);
            float value = 3.14159265F;
            uut.Put<float>(900, value);
            float getValue = uut.Get<float>(900);
            Assert.AreEqual(value, getValue);
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_ToSizedSpan_MatchesBuffer()
        {
            var buffer = new byte[4];
            buffer[0] = 0x0D;
            buffer[1] = 0x0C;
            buffer[2] = 0x0B;
            buffer[3] = 0x0A;
            var uut = new ByteBuffer(buffer);
            var span = uut.ToSizedSpan();
            Assert.SpanEqual<byte>(buffer, span);
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_ToSpan_MatchesBuffer()
        {
            var buffer = new byte[4];
            buffer[0] = 0x0D;
            buffer[1] = 0x0C;
            buffer[2] = 0x0B;
            buffer[3] = 0x0A;
            var uut = new ByteBuffer(buffer);
            var span = uut.ToSpan(0, 4);
            Assert.SpanEqual<byte>(buffer, span);
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_SlicedToSizedSpan_MatchesBuffer()
        {
            var buffer = new byte[8];
            buffer[0] = 0x0D;
            buffer[1] = 0x0C;
            buffer[2] = 0x0B;
            buffer[3] = 0x0A;
            buffer[4] = 0x09;
            buffer[5] = 0x08;
            buffer[6] = 0x07;
            buffer[7] = 0x06;

            var uut = new ByteBuffer(buffer, 2, 6);
            var span = uut.ToSizedSpan();
            Assert.SpanEqual<byte>(buffer.AsSpan(2, 4), span);
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_SlicedToSpan_MatchesBuffer()
        {
            var buffer = new byte[4];
            buffer[0] = 0x0D;
            buffer[1] = 0x0C;
            buffer[2] = 0x0B;
            buffer[3] = 0x0A;
            var uut = new ByteBuffer(buffer);
            var span = uut.ToSpan(1, 2);
            Assert.SpanEqual<byte>(buffer.AsSpan(1, 2), span);

            span[0] = 0xAA;
            Assert.AreEqual((byte)0xAA, buffer[1]);
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_GetSpan_Float()
        {
            const int len = 9;
            var fData = new float[len];
            fData[0] = 1.0079F;
            fData[1] = 4.0026F;
            fData[2] = 6.941F;
            fData[3] = 9.0122F;
            fData[4] = 10.811F;
            fData[5] = 12.0107F;
            fData[6] = 14.0067F;
            fData[7] = 15.9994F;
            fData[8] = 18.9984F;

            var buffer = new byte[sizeof(float) * fData.Length];
            Buffer.BlockCopy(fData, 0, buffer, 0, buffer.Length);
            var uut = new ByteBuffer(buffer);

            // Get the full span back out and ensure they are the same
            var bbSpan = uut.GetSpan<float>(0, len);
            Assert.SpanEqual<float>(fData, bbSpan);

            // Get a portion of the span back out and ensure the subrange matches
            var bbSpan2 = uut.GetSpan<float>(sizeof(float), len - 1);
            Assert.AreEqual(bbSpan2.Length, len - 1);
            for (int i = 1; i < len - 1; i++)
            {
                Assert.AreEqual(fData[i], bbSpan2[i - 1]);
            }

            // Get a sub portion of the span back out and ensure the subrange matches
            var bbSpan3 = uut.GetSpan<float>(sizeof(float) * 2, len - 4);
            Assert.AreEqual(bbSpan3.Length, len - 4);
            for (int i = 2; i < len - 4; i++)
            {
                Assert.AreEqual(fData[i], bbSpan3[i - 2]);
            }
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_PutSpan_Float()
        {
            const int len = 9;
            Span<float> data = stackalloc float[len];
            data[0] = 1.0079F;
            data[1] = 4.0026F;
            data[2] = 6.941F;
            data[3] = 9.0122F;
            data[4] = 10.811F;
            data[5] = 12.0107F;
            data[6] = 14.0067F;
            data[7] = 15.9994F;
            data[8] = 18.9984F;

            var uut = new ByteBuffer(1024);
            uut.PutSpan<float>(0, data);

            // Get the full span back out and ensure they are the same
            var bbSpan = uut.GetSpan<float>(0, len);
            Assert.SpanEqual<float>(data, bbSpan);
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_PutSpan_Double()
        {
            const int len = 9;
            Span<double> data = stackalloc double[len];
            data[0] = 1.0079;
            data[1] = 4.0026;
            data[2] = 6.941;
            data[3] = 9.0122;
            data[4] = 10.811;
            data[5] = 12.0107;
            data[6] = 14.0067;
            data[7] = 15.9994;
            data[8] = 18.9984;

            var uut = new ByteBuffer(1024);
            uut.PutSpan<double>(0, data);

            // Get the full span back out and ensure they are the same
            var bbSpan = uut.GetSpan<double>(0, len);
            Assert.SpanEqual<double>(data, bbSpan);
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_PutSpan_Int()
        {
            const int len = 9;
            Span<int> data = stackalloc int[len];
            data[0] = 1;
            data[1] = 4;
            data[2] = 6;
            data[3] = 9;
            data[4] = 10;
            data[5] = 12;
            data[6] = 14;
            data[7] = 15;
            data[8] = 18;

            var uut = new ByteBuffer(1024);
            uut.PutSpan<int>(0, data);

            // Get the full span back out and ensure they are the same
            var bbSpan = uut.GetSpan<int>(0, len);
            Assert.SpanEqual<int>(data, bbSpan);
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_PutSpan_UInt()
        {
            const int len = 9;
            Span<uint> data = stackalloc uint[len];
            data[0] = 1;
            data[1] = 4;
            data[2] = 6;
            data[3] = 9;
            data[4] = 10;
            data[5] = 12;
            data[6] = 14;
            data[7] = 15;
            data[8] = 18;

            var uut = new ByteBuffer(1024);
            uut.PutSpan<uint>(0, data);

            // Get the full span back out and ensure they are the same
            var bbSpan = uut.GetSpan<uint>(0, len);
            Assert.SpanEqual<uint>(data, bbSpan);
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_PutSpan_Long()
        {
            const int len = 9;
            Span<long> data = stackalloc long[len];
            data[0] = 1;
            data[1] = 4;
            data[2] = 6;
            data[3] = 9;
            data[4] = 10;
            data[5] = 12;
            data[6] = 14;
            data[7] = 15;
            data[8] = 18;

            var uut = new ByteBuffer(1024);
            uut.PutSpan<long>(0, data);

            // Get the full span back out and ensure they are the same
            var bbSpan = uut.GetSpan<long>(0, len);
            Assert.SpanEqual<long>(data, bbSpan);
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_PutSpan_Byte()
        {
            const int len = 9;
            Span<byte> data = stackalloc byte[len];
            data[0] = 1;
            data[1] = 4;
            data[2] = 6;
            data[3] = 9;
            data[4] = 10;
            data[5] = 12;
            data[6] = 14;
            data[7] = 15;
            data[8] = 18;

            var uut = new ByteBuffer(1024);
            uut.PutSpan<byte>(0, data);

            // Get the full span back out and ensure they are the same
            var bbSpan = uut.GetSpan<byte>(0, len);
            Assert.SpanEqual<byte>(data, bbSpan);
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_PutSpan_SByte()
        {
            const int len = 9;
            Span<sbyte> data = stackalloc sbyte[len];
            data[0] = 1;
            data[1] = 4;
            data[2] = 6;
            data[3] = 9;
            data[4] = 10;
            data[5] = 12;
            data[6] = 14;
            data[7] = 15;
            data[8] = 18;

            var uut = new ByteBuffer(1024);
            uut.PutSpan<sbyte>(0, data);

            // Get the full span back out and ensure they are the same
            var bbSpan = uut.GetSpan<sbyte>(0, len);
            Assert.SpanEqual<sbyte>(data, bbSpan);
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_Position_SetsCorrectly()
        {
            var buffer = new byte[100];
            var uut = new ByteBuffer(buffer);

            Assert.AreEqual(0, uut.Position);

            uut.Position = 50;
            Assert.AreEqual(50, uut.Position);
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_Reset_SetsPositionToZero()
        {
            var buffer = new byte[100];
            var uut = new ByteBuffer(buffer);

            uut.Position = 50;
            Assert.AreEqual(50, uut.Position);

            uut.Reset();
            Assert.AreEqual(0, uut.Position);
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_PartialBuffer_HasCorrectLength()
        {
            var buffer = new byte[1000];
            // Create a ByteBuffer that only uses part of the buffer
            var uut = new ByteBuffer(buffer, 100, 500);

            Assert.AreEqual(500, uut.Length);
            Assert.AreEqual(100, uut.Position);
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_PutAndGetSbyte()
        {
            var buffer = new byte[10];
            var uut = new ByteBuffer(buffer);

            uut.PutSbyte(0, -42);
            Assert.AreEqual((sbyte)-42, uut.GetSbyte(0));

            uut.PutSbyte(1, 127);
            Assert.AreEqual((sbyte)127, uut.GetSbyte(1));
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_PadBytes()
        {
            var buffer = new byte[10];
            var uut = new ByteBuffer(buffer);

            buffer[2] = 99;
            buffer[3] = 99;
            buffer[4] = 99;
            buffer[5] = 99;
            buffer[6] = 99;
            uut.PadBytes(2, 5);

            Assert.AreEqual((byte)0, buffer[0]);
            Assert.AreEqual((byte)0, buffer[1]);
            Assert.AreEqual((byte)0, buffer[2]);
            Assert.AreEqual((byte)0, buffer[3]);
            Assert.AreEqual((byte)0, buffer[4]);
            Assert.AreEqual((byte)0, buffer[5]);
            Assert.AreEqual((byte)0, buffer[6]);
            Assert.AreEqual((byte)0, buffer[7]);
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_StringUTF8_RoundTrip()
        {
            var uut = new ByteBuffer(1024);
            var testString = "Hello, World!";

            uut.PutStringUTF8(0, testString.AsSpan());
            var readString = uut.GetStringUTF8(0, testString.Length);

            Assert.AreEqual(testString, readString);
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_StringUTF8_Unicode()
        {
            var uut = new ByteBuffer(1024);
            var testString = "Hello, UTF-8: \u4e16\u754c";

            var byteCount = System.Text.Encoding.UTF8.GetByteCount(testString);
            uut.PutStringUTF8(0, testString.AsSpan());

            var readString = uut.GetStringUTF8(0, byteCount);
            Assert.AreEqual(testString, readString);
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_ResetWithNewBuffer()
        {
            var buffer1 = new byte[100];
            var uut = new ByteBuffer(buffer1);

            Assert.AreEqual(100, uut.Length);

            var buffer2 = new byte[200];
            uut.Reset(buffer2, 10, 150);

            Assert.AreEqual(150, uut.Length);
            Assert.AreEqual(10, uut.Position);
        }
    }
}
