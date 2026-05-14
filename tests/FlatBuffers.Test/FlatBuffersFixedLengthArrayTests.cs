/*
 * Copyright 2025 Google Inc. All rights reserved.
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
using System.Linq;
using MyGame.Example;

namespace Google.FlatBuffers.Test
{
    [FlatBuffersTestClass]
    public class FlatBuffersFixedLengthArrayTests
    {
        [FlatBuffersTestMethod]
        public void FixedLengthArray_LengthConstantsMatchSchema_ReturnTrue()
        {
            const int nestedALength = NestedStruct.ALength;
            const int nestedCLength = NestedStruct.CLength;
            const int nestedDLength = NestedStruct.DLength;
            const int arrayBLength = ArrayStruct.BLength;
            const int arrayFLength = ArrayStruct.FLength;

            Assert.AreEqual(2, nestedALength);
            Assert.AreEqual(2, nestedCLength);
            Assert.AreEqual(2, nestedDLength);
            Assert.AreEqual(15, arrayBLength);
            Assert.AreEqual(2, arrayFLength);
        }

#if ENABLE_SPAN_T
        [FlatBuffersTestMethod]
        public void FixedLengthArray_GetBytesSpanLengthIsCorrect_ReturnTrue()
        {
            var builder = new FlatBufferBuilder(1024);
            var ints = new int[] { 1, 2 };
            var enumB = TestEnum.A;
            var enums = new TestEnum[] { TestEnum.B, TestEnum.C };
            var longs = new long[] { 10L, 20L };
            
            var structOffset = NestedStruct.CreateNestedStruct(builder, ints, enumB, enums, longs);
            builder.Finish(structOffset.Value);
            
            var bb = builder.DataBuffer;
            var nestedStruct = new NestedStruct();
            nestedStruct.__assign(bb.Length - builder.Offset, bb);

            Span<int> intSpan = nestedStruct.GetABytes();
            Span<TestEnum> enumSpan = nestedStruct.GetCBytes();
            Span<long> longSpan = nestedStruct.GetDBytes();

            Assert.AreEqual(intSpan.Length, NestedStruct.ALength);
            Assert.AreEqual(enumSpan.Length, NestedStruct.CLength);
            Assert.AreEqual(longSpan.Length, NestedStruct.DLength);          
        }
#endif

#if !ENABLE_SPAN_T
        [FlatBuffersTestMethod]
        public void FixedLengthArray_GetBytesArraySegmentLengthIsCorrect_ReturnTrue()
        {
            var builder = new FlatBufferBuilder(1024);
            var ints = new int[] { 1, 2 };
            var enumB = TestEnum.A;
            var enums = new TestEnum[] { TestEnum.B, TestEnum.C };
            var longs = new long[] { 10L, 20L };

            var structOffset = NestedStruct.CreateNestedStruct(builder, ints, enumB, enums, longs);
            builder.Finish(structOffset.Value);

            var buffer = builder.DataBuffer;
            var nestedStruct = new NestedStruct();
            nestedStruct.__assign(buffer.Length - builder.Offset, buffer);

            Assert.IsTrue(nestedStruct.GetABytes().HasValue);
            Assert.IsTrue(nestedStruct.GetCBytes().HasValue);
            Assert.IsTrue(nestedStruct.GetDBytes().HasValue);

            ArraySegment<byte> intSegment = nestedStruct.GetABytes().Value;
            ArraySegment<byte> enumSegment = nestedStruct.GetCBytes().Value;
            ArraySegment<byte> longSegment = nestedStruct.GetDBytes().Value;

            Assert.AreEqual(intSegment.Count, NestedStruct.ALength * sizeof(int));
            Assert.AreEqual(enumSegment.Count, NestedStruct.CLength * sizeof(sbyte));
            Assert.AreEqual(longSegment.Count, NestedStruct.DLength * sizeof(long));
        }
#endif

#if ENABLE_SPAN_T
        [FlatBuffersTestMethod]
        public void FixedLengthArray_GetBytesSpanEquality_ReturnTrue()
        {
            var builder = new FlatBufferBuilder(1024);

            var floatA = 3.14f;
            var intArray = Enumerable.Range(1, 15).ToArray();
            var byteC = (sbyte)42;
            var intE = 999;
            var longArray = new long[] { 5000L, 6000L };

            var nestedInts = new int[2, 2] { { 10, 20 }, { 30, 40 } };
            var nestedEnumB = new TestEnum[] { TestEnum.A, TestEnum.B };
            var nestedEnums = new TestEnum[2, 2] { { TestEnum.A, TestEnum.B }, { TestEnum.C, TestEnum.A } };
            var nestedLongs = new long[2, 2] { { 100L, 200L }, { 300L, 400L } };

            var structOffset = ArrayStruct.CreateArrayStruct(builder, floatA, intArray, byteC,
                nestedInts, nestedEnumB, nestedEnums, nestedLongs, intE, longArray);

            ArrayTable.StartArrayTable(builder);
            ArrayTable.AddA(builder, structOffset);
            var rootTable = ArrayTable.EndArrayTable(builder);
            builder.Finish(rootTable.Value);

            var finishedBytes = builder.SizedByteArray();
            ByteBuffer bb = new ByteBuffer(finishedBytes);
            ArrayTable arrayTable = ArrayTable.GetRootAsArrayTable(bb);
            ArrayStruct arrayStruct = arrayTable.A.Value;

            Assert.AreEqual(byteC, arrayStruct.C);
            Assert.AreEqual(intE, arrayStruct.E);

            Assert.IsTrue(arrayStruct.GetBBytes().SequenceEqual(intArray));
            Assert.IsTrue(arrayStruct.GetFBytes().SequenceEqual(longArray));
            
            // Test nested struct arrays
            for (int i = 0; i < 2; i++)
            {
                var nestedStruct = arrayStruct.D(i);
                
                var nestedIntSpan = nestedStruct.GetABytes();
                var expectedNestedInts = new int[] { nestedInts[i, 0], nestedInts[i, 1] };
                Assert.IsTrue(nestedIntSpan.SequenceEqual(expectedNestedInts));
                
                Assert.AreEqual(nestedEnumB[i], nestedStruct.B);
                
                var nestedEnumSpan = nestedStruct.GetCBytes();
                var expectedNestedEnums = new TestEnum[] { nestedEnums[i, 0], nestedEnums[i, 1] };
                Assert.IsTrue(nestedEnumSpan.SequenceEqual(expectedNestedEnums));
                
                var nestedLongSpan = nestedStruct.GetDBytes();
                var expectedNestedLongs = new long[] { nestedLongs[i, 0], nestedLongs[i, 1] };
                Assert.IsTrue(nestedLongSpan.SequenceEqual(expectedNestedLongs));
            }
        }
#endif  

#if !ENABLE_SPAN_T
        [FlatBuffersTestMethod] 
        public void FixedLengthArray_GetBytesArraySegmentEquality_ReturnTrue()
        {
            var builder = new FlatBufferBuilder(1024);
            
            var floatA = 3.14f;
            var intArray = Enumerable.Range(1, 15).ToArray();
            var byteC = (sbyte)42;
            var intE = 999;
            var longArray = new long[] { 5000L, 6000L };
            
            var nestedInts = new int[2, 2] { { 10, 20 }, { 30, 40 } };
            var nestedEnumB = new TestEnum[] { TestEnum.A, TestEnum.B };
            var nestedEnums = new TestEnum[2, 2] { { TestEnum.A, TestEnum.B }, { TestEnum.C, TestEnum.A } };
            var nestedLongs = new long[2, 2] { { 100L, 200L }, { 300L, 400L } };
            
            var structOffset = ArrayStruct.CreateArrayStruct(builder, floatA, intArray, byteC, 
                nestedInts, nestedEnumB, nestedEnums, nestedLongs, intE, longArray);

            ArrayTable.StartArrayTable(builder);
            ArrayTable.AddA(builder, structOffset);
            var rootTable = ArrayTable.EndArrayTable(builder);
            builder.Finish(rootTable.Value);

            var finishedBytes = builder.SizedByteArray();
            ByteBuffer bb = new ByteBuffer(finishedBytes);
            ArrayTable arrayTable = ArrayTable.GetRootAsArrayTable(bb);
            ArrayStruct arrayStruct = arrayTable.A.Value;

            // Test that we can read basic scalars correctly
            Assert.AreEqual(byteC, arrayStruct.C);
            Assert.AreEqual(intE, arrayStruct.E);
            
            Assert.IsTrue(arrayStruct.GetBBytes().HasValue);
            var intSegment = arrayStruct.GetBBytes().Value;
            for (int i = 0, offset = 0; i < intArray.Length; i++, offset += sizeof(int))
            {
                var segmentValue = BitConverter.ToInt32(intSegment.Array,
                    intSegment.Offset + offset);
                Assert.AreEqual(intArray[i], segmentValue);
            }
            
            Assert.IsTrue(arrayStruct.GetFBytes().HasValue);
            var longSegment = arrayStruct.GetFBytes().Value;
            for (int i = 0, offset = 0; i < longArray.Length; i++, offset += sizeof(long))
            {
                var segmentValue = BitConverter.ToInt64(longSegment.Array,
                    longSegment.Offset + offset);
                Assert.AreEqual(longArray[i], segmentValue);
            }
            
            // Test nested struct arrays
            for (int i = 0; i < 2; i++)
            {
                var nestedStruct = arrayStruct.D(i);
                
                Assert.IsTrue(nestedStruct.GetABytes().HasValue);
                var nestedIntSegment = nestedStruct.GetABytes().Value;
                var expectedNestedInts = new int[] { nestedInts[i, 0], nestedInts[i, 1] };
                for (int ii = 0, offset = 0; ii < NestedStruct.ALength; ii++, offset += sizeof(int))
                {
                    var segmentValue = BitConverter.ToInt32(nestedIntSegment.Array,
                        nestedIntSegment.Offset + offset);
                    Assert.AreEqual(expectedNestedInts[ii], segmentValue);
                }

                Assert.AreEqual(nestedEnumB[i], nestedStruct.B);
                
                Assert.IsTrue(nestedStruct.GetCBytes().HasValue);
                var nestedEnumSegment = nestedStruct.GetCBytes().Value;
                var expectedNestedEnums = new TestEnum[] { nestedEnums[i, 0], nestedEnums[i, 1] };
                for (int ii = 0, offset = 0; ii < NestedStruct.CLength; ii++, offset += sizeof(sbyte))
                {
                    var segmentValue = (TestEnum)nestedEnumSegment.Array[nestedEnumSegment.Offset + offset];
                    Assert.AreEqual(expectedNestedEnums[ii], segmentValue);
                }
                
                Assert.IsTrue(nestedStruct.GetDBytes().HasValue);
                var nestedLongSegment = nestedStruct.GetDBytes().Value;
                var expectedNestedLongs = new long[] { nestedLongs[i, 0], nestedLongs[i, 1] };
                for (int ii = 0, offset = 0; ii < NestedStruct.DLength; ii++, offset += sizeof(long))
                {
                    var segmentValue = BitConverter.ToInt64(nestedLongSegment.Array,
                        nestedLongSegment.Offset + offset);
                    Assert.AreEqual(expectedNestedLongs[ii], segmentValue);
                }
            }
        }
#endif
    }
}
