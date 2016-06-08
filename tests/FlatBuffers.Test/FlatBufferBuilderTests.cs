/*
 * Copyright 2016 Google Inc. All rights reserved.
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

namespace FlatBuffers.Test
{
    [FlatBuffersTestClass]
    public class FlatBufferBuilderTests
    {
        private FlatBufferBuilder CreateBuffer(bool forceDefaults = true)
        {
            var fbb = new FlatBufferBuilder(16) {ForceDefaults = forceDefaults};
            fbb.StartObject(1);
            return fbb;
        }

        [FlatBuffersTestMethod]
        public void FlatBufferBuilder_WithForceDefaults_WhenAddBool_AndDefaultValue_OffsetIncreasesBySize()
        {
            var fbb = CreateBuffer();
            var storedOffset = fbb.Offset;
            fbb.AddBool(0, false, false);
            var endOffset = fbb.Offset;
            Assert.AreEqual(sizeof(bool), endOffset-storedOffset);
        }

        [FlatBuffersTestMethod]
        public void FlatBufferBuilder_WithForceDefaults_WhenAddSByte_AndDefaultValue_OffsetIncreasesBySize()
        {
            var fbb = CreateBuffer();
            var storedOffset = fbb.Offset;
            fbb.AddSbyte(0, 0, 0);
            var endOffset = fbb.Offset;
            Assert.AreEqual(sizeof(sbyte), endOffset - storedOffset);
        }

        [FlatBuffersTestMethod]
        public void FlatBufferBuilder_WithForceDefaults_WhenAddByte_AndDefaultValue_OffsetIncreasesBySize()
        {
            var fbb = CreateBuffer();
            var storedOffset = fbb.Offset;
            fbb.AddByte(0, 0, 0);
            var endOffset = fbb.Offset;
            Assert.AreEqual(sizeof(byte), endOffset - storedOffset);
        }

        [FlatBuffersTestMethod]
        public void FlatBufferBuilder_WithForceDefaults_WhenAddShort_AndDefaultValue_OffsetIncreasesBySize()
        {
            var fbb = CreateBuffer();
            var storedOffset = fbb.Offset;
            fbb.AddShort(0, 0, 0);
            var endOffset = fbb.Offset;
            Assert.AreEqual(sizeof(short), endOffset - storedOffset);
        }

        [FlatBuffersTestMethod]
        public void FlatBufferBuilder_WithForceDefaults_WhenAddUShort_AndDefaultValue_OffsetIncreasesBySize()
        {
            var fbb = CreateBuffer();
            var storedOffset = fbb.Offset;
            fbb.AddUshort(0, 0, 0);
            var endOffset = fbb.Offset;
            Assert.AreEqual(sizeof(ushort), endOffset - storedOffset);
        }

        [FlatBuffersTestMethod]
        public void FlatBufferBuilder_WithForceDefaults_WhenAddInt_AndDefaultValue_OffsetIncreasesBySize()
        {
            var fbb = CreateBuffer();
            var storedOffset = fbb.Offset;
            fbb.AddInt(0, 0, 0);
            var endOffset = fbb.Offset;
            Assert.AreEqual(sizeof(int), endOffset - storedOffset);
        }

        [FlatBuffersTestMethod]
        public void FlatBufferBuilder_WithForceDefaults_WhenAddUInt_AndDefaultValue_OffsetIncreasesBySize()
        {
            var fbb = CreateBuffer();
            var storedOffset = fbb.Offset;
            fbb.AddUint(0, 0, 0);
            var endOffset = fbb.Offset;
            Assert.AreEqual(sizeof(uint), endOffset - storedOffset);
        }

        [FlatBuffersTestMethod]
        public void FlatBufferBuilder_WithForceDefaults_WhenAddLong_AndDefaultValue_OffsetIncreasesBySize()
        {
            var fbb = CreateBuffer();
            var storedOffset = fbb.Offset;
            fbb.AddLong(0, 0, 0);
            var endOffset = fbb.Offset;
            Assert.AreEqual(sizeof(long), endOffset - storedOffset);
        }

        [FlatBuffersTestMethod]
        public void FlatBufferBuilder_WithForceDefaults_WhenAddULong_AndDefaultValue_OffsetIncreasesBySize()
        {
            var fbb = CreateBuffer();
            var storedOffset = fbb.Offset;
            fbb.AddUlong(0, 0, 0);
            var endOffset = fbb.Offset;
            Assert.AreEqual(sizeof(ulong), endOffset - storedOffset);
        }

        [FlatBuffersTestMethod]
        public void FlatBufferBuilder_WithForceDefaults_WhenAddFloat_AndDefaultValue_OffsetIncreasesBySize()
        {
            var fbb = CreateBuffer();
            var storedOffset = fbb.Offset;
            fbb.AddFloat(0, 0, 0);
            var endOffset = fbb.Offset;
            Assert.AreEqual(sizeof(float), endOffset - storedOffset);
        }

        [FlatBuffersTestMethod]
        public void FlatBufferBuilder_WithForceDefaults_WhenAddDouble_AndDefaultValue_OffsetIncreasesBySize()
        {
            var fbb = CreateBuffer();
            var storedOffset = fbb.Offset;
            fbb.AddDouble(0, 0, 0);
            var endOffset = fbb.Offset;
            Assert.AreEqual(sizeof(double), endOffset - storedOffset);
        }

        [FlatBuffersTestMethod]
        public void FlatBufferBuilder_WhenAddBool_AndDefaultValue_OffsetIsUnchanged()
        {
            var fbb = CreateBuffer(false);
            var storedOffset = fbb.Offset;
            fbb.AddBool(0, false, false);
            var endOffset = fbb.Offset;
            Assert.AreEqual(endOffset, storedOffset);
        }

        [FlatBuffersTestMethod]
        public void FlatBufferBuilder_WhenAddSByte_AndDefaultValue_OffsetIsUnchanged()
        {
            var fbb = CreateBuffer(false);
            var storedOffset = fbb.Offset;
            fbb.AddSbyte(0, 0, 0);
            var endOffset = fbb.Offset;
            Assert.AreEqual(endOffset, storedOffset);
        }

        [FlatBuffersTestMethod]
        public void FlatBufferBuilder_WhenAddByte_AndDefaultValue_OffsetIsUnchanged()
        {
            var fbb = CreateBuffer(false);
            var storedOffset = fbb.Offset;
            fbb.AddByte(0, 0, 0);
            var endOffset = fbb.Offset;
            Assert.AreEqual(endOffset, storedOffset);
        }

        [FlatBuffersTestMethod]
        public void FlatBufferBuilder_WhenAddShort_AndDefaultValue_OffsetIsUnchanged()
        {
            var fbb = CreateBuffer(false);
            var storedOffset = fbb.Offset;
            fbb.AddShort(0, 0, 0);
            var endOffset = fbb.Offset;
            Assert.AreEqual(endOffset, storedOffset);
        }

        [FlatBuffersTestMethod]
        public void FlatBufferBuilder_WhenAddUShort_AndDefaultValue_OffsetIsUnchanged()
        {
            var fbb = CreateBuffer(false);
            var storedOffset = fbb.Offset;
            fbb.AddUshort(0, 0, 0);
            var endOffset = fbb.Offset;
            Assert.AreEqual(endOffset, storedOffset);
        }

        [FlatBuffersTestMethod]
        public void FlatBufferBuilder_WhenAddInt_AndDefaultValue_OffsetIsUnchanged()
        {
            var fbb = CreateBuffer(false);
            var storedOffset = fbb.Offset;
            fbb.AddInt(0, 0, 0);
            var endOffset = fbb.Offset;
            Assert.AreEqual(endOffset, storedOffset);
        }

        [FlatBuffersTestMethod]
        public void FlatBufferBuilder_WhenAddUInt_AndDefaultValue_OffsetIsUnchanged()
        {
            var fbb = CreateBuffer(false);
            var storedOffset = fbb.Offset;
            fbb.AddUint(0, 0, 0);
            var endOffset = fbb.Offset;
            Assert.AreEqual(endOffset, storedOffset);
        }

        [FlatBuffersTestMethod]
        public void FlatBufferBuilder_WhenAddLong_AndDefaultValue_OffsetIsUnchanged()
        {
            var fbb = CreateBuffer(false);
            var storedOffset = fbb.Offset;
            fbb.AddLong(0, 0, 0);
            var endOffset = fbb.Offset;
            Assert.AreEqual(endOffset, storedOffset);
        }

        [FlatBuffersTestMethod]
        public void FlatBufferBuilder_WhenAddULong_AndDefaultValue_OffsetIsUnchanged()
        {
            var fbb = CreateBuffer(false);
            var storedOffset = fbb.Offset;
            fbb.AddUlong(0, 0, 0);
            var endOffset = fbb.Offset;
            Assert.AreEqual(endOffset, storedOffset);
        }

        [FlatBuffersTestMethod]
        public void FlatBufferBuilder_WhenAddFloat_AndDefaultValue_OffsetIsUnchanged()
        {
            var fbb = CreateBuffer(false);
            var storedOffset = fbb.Offset;
            fbb.AddFloat(0, 0, 0);
            var endOffset = fbb.Offset;
            Assert.AreEqual(endOffset, storedOffset);
        }

        [FlatBuffersTestMethod]
        public void FlatBufferBuilder_WhenAddDouble_AndDefaultValue_OffsetIsUnchanged()
        {
            var fbb = CreateBuffer(false);
            var storedOffset = fbb.Offset;
            fbb.AddDouble(0, 0, 0);
            var endOffset = fbb.Offset;
            Assert.AreEqual(endOffset, storedOffset);
        }
    }
}
