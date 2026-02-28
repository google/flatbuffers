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

using System;

namespace Google.FlatSpanBuffers.Tests
{
    [FlatBuffersTestClass]
    public class FlatBufferBuilderTests
    {
        private FlatBufferBuilder CreateBuffer(bool forceDefaults = true)
        {
            var fbb = new FlatBufferBuilder(16) {ForceDefaults = forceDefaults};
            fbb.StartTable(1);
            return fbb;
        }

        private void AssertOffsetDelta(bool forceDefaults, int expectedDelta, Action<FlatBufferBuilder> add)
        {
            var fbb = CreateBuffer(forceDefaults);
            var startOffset = fbb.Offset;
            add(fbb);
            Assert.AreEqual(expectedDelta, fbb.Offset - startOffset);
        }

        [FlatBuffersTestMethod]
        public void FlatBufferBuilder_WithForceDefaults_DefaultValues_AdvanceOffsetBySize()
        {
            var cases = new (int Size, Action<FlatBufferBuilder> Add)[]
            {
                (sizeof(bool), fbb => fbb.Add<bool>(0, false, false)),
                (sizeof(sbyte), fbb => fbb.Add<sbyte>(0, 0, 0)),
                (sizeof(byte), fbb => fbb.Add<byte>(0, 0, 0)),
                (sizeof(short), fbb => fbb.Add<short>(0, 0, 0)),
                (sizeof(ushort), fbb => fbb.Add<ushort>(0, 0, 0)),
                (sizeof(int), fbb => fbb.Add<int>(0, 0, 0)),
                (sizeof(uint), fbb => fbb.Add<uint>(0, 0, 0)),
                (sizeof(long), fbb => fbb.Add<long>(0, 0, 0)),
                (sizeof(ulong), fbb => fbb.Add<ulong>(0, 0, 0)),
                (sizeof(float), fbb => fbb.Add<float>(0, 0, 0)),
                (sizeof(double), fbb => fbb.Add<double>(0, 0, 0)),
            };

            foreach (var testCase in cases)
            {
                AssertOffsetDelta(forceDefaults: true, expectedDelta: testCase.Size, add: testCase.Add);
            }
        }

        [FlatBuffersTestMethod]
        public void FlatBufferBuilder_WithoutForceDefaults_DefaultValues_DoNotAdvanceOffset()
        {
            var cases = new Action<FlatBufferBuilder>[]
            {
                fbb => fbb.Add<bool>(0, false, false),
                fbb => fbb.Add<sbyte>(0, 0, 0),
                fbb => fbb.Add<byte>(0, 0, 0),
                fbb => fbb.Add<short>(0, 0, 0),
                fbb => fbb.Add<ushort>(0, 0, 0),
                fbb => fbb.Add<int>(0, 0, 0),
                fbb => fbb.Add<uint>(0, 0, 0),
                fbb => fbb.Add<long>(0, 0, 0),
                fbb => fbb.Add<ulong>(0, 0, 0),
                fbb => fbb.Add<float>(0, 0, 0),
                fbb => fbb.Add<double>(0, 0, 0),
            };

            foreach (var add in cases)
            {
                AssertOffsetDelta(forceDefaults: false, expectedDelta: 0, add: add);
            }
        }

        [FlatBuffersTestMethod]
        public void FlatBufferBuilder_WithoutForceDefaults_NullableNull_DoesNotAdvanceOffset()
        {
            var cases = new Action<FlatBufferBuilder>[]
            {
                fbb => fbb.Add<bool>(0, (bool?)null),
                fbb => fbb.Add<sbyte>(0, (sbyte?)null),
                fbb => fbb.Add<byte>(0, (byte?)null),
                fbb => fbb.Add<short>(0, (short?)null),
                fbb => fbb.Add<ushort>(0, (ushort?)null),
                fbb => fbb.Add<int>(0, (int?)null),
                fbb => fbb.Add<uint>(0, (uint?)null),
                fbb => fbb.Add<long>(0, (long?)null),
                fbb => fbb.Add<ulong>(0, (ulong?)null),
                fbb => fbb.Add<float>(0, (float?)null),
                fbb => fbb.Add<double>(0, (double?)null),
            };

            foreach (var add in cases)
            {
                AssertOffsetDelta(forceDefaults: false, expectedDelta: 0, add: add);
            }
        }

        [FlatBuffersTestMethod]
        public void FlatBufferBuilder_WithoutForceDefaults_NullableNonNull_AdvancesOffsetBySize()
        {
            var cases = new (int Size, Action<FlatBufferBuilder> Add)[]
            {
                (sizeof(bool), fbb => fbb.Add<bool>(0, (bool?)true)),
                (sizeof(sbyte), fbb => fbb.Add<sbyte>(0, (sbyte?)1)),
                (sizeof(byte), fbb => fbb.Add<byte>(0, (byte?)1)),
                (sizeof(short), fbb => fbb.Add<short>(0, (short?)1)),
                (sizeof(ushort), fbb => fbb.Add<ushort>(0, (ushort?)1)),
                (sizeof(int), fbb => fbb.Add<int>(0, (int?)1)),
                (sizeof(uint), fbb => fbb.Add<uint>(0, (uint?)1)),
                (sizeof(long), fbb => fbb.Add<long>(0, (long?)1)),
                (sizeof(ulong), fbb => fbb.Add<ulong>(0, (ulong?)1)),
                (sizeof(float), fbb => fbb.Add<float>(0, (float?)1.0F)),
                (sizeof(double), fbb => fbb.Add<double>(0, (double?)1.0)),
            };

            foreach (var testCase in cases)
            {
                AssertOffsetDelta(forceDefaults: false, expectedDelta: testCase.Size, add: testCase.Add);
            }
        }

        [FlatBuffersTestMethod]
        public void FlatBufferBuilder_AddSpan_Array_AdvancesOffsetByElementSizeTimesLength()
        {
            var cases = new (int ExpectedDelta, Action<FlatBufferBuilder> Add)[]
            {
                (sizeof(float) * 9, fbb => fbb.AddSpan<float>(new[] { 1.25F, -2.5F, 3.75F, -4.0F, 5.5F, -6.25F, 7.0F, -8.875F, 9.125F })),
                (sizeof(bool) * 9, fbb => fbb.AddSpan<bool>(new[] { true, false, true, true, false, false, true, false, true })),
                (sizeof(double) * 9, fbb => fbb.AddSpan<double>(new[] { 1.0, -2.0, 3.5, -4.5, 5.25, -6.75, 7.125, -8.875, 9.5 })),
            };

            foreach (var testCase in cases)
            {
                AssertOffsetDelta(forceDefaults: false, expectedDelta: testCase.ExpectedDelta, add: testCase.Add);
            }
        }

        [FlatBuffersTestMethod]
        public void FlatBufferBuilder_AddSpan_StackallocSpan_AdvancesOffsetByElementSizeTimesLength()
        {
            const int len = 9;

            AssertOffsetDelta(forceDefaults: false, expectedDelta: sizeof(float) * len, add: fbb =>
            {
                Span<float> data = stackalloc float[] { 1.25F, -2.5F, 3.75F, -4.0F, 5.5F, -6.25F, 7.0F, -8.875F, 9.125F };
                fbb.AddSpan<float>(data);
            });

            AssertOffsetDelta(forceDefaults: false, expectedDelta: sizeof(bool) * len, add: fbb =>
            {
                Span<bool> data = stackalloc bool[] { true, false, true, true, false, false, true, false, true };
                fbb.AddSpan<bool>(data);
            });

            AssertOffsetDelta(forceDefaults: false, expectedDelta: sizeof(double) * len, add: fbb =>
            {
                Span<double> data = stackalloc double[] { 1.0, -2.0, 3.5, -4.5, 5.25, -6.75, 7.125, -8.875, 9.5 };
                fbb.AddSpan<double>(data);
            });
        }

        [FlatBuffersTestMethod]
        public void FlatBufferBuilder_AddSpan_Empty_DoesNotAdvanceOffset()
        {
            AssertOffsetDelta(forceDefaults: false, expectedDelta: 0, add: fbb => fbb.AddSpan<float>(Array.Empty<float>()));
            AssertOffsetDelta(forceDefaults: false, expectedDelta: 0, add: fbb => fbb.AddSpan<float>(Span<float>.Empty));
        }

        [FlatBuffersTestMethod]
        public void FlatBufferBuilder_AddSpan_Vector_RoundTripsValuesInOrder()
        {
            var expected = new[] { 1.25F, -2.5F, 3.75F, -4.0F, 5.5F };

            var fbb = new FlatBufferBuilder(64);
            fbb.StartVector(sizeof(float), expected.Length, sizeof(float));
            fbb.AddSpan<float>(expected);
            var vectorOffset = fbb.EndVector();

            fbb.StartTable(1);
            fbb.AddOffset(0, vectorOffset.Value, 0);
            var rootTable = fbb.EndTable();
            fbb.Finish(rootTable);

            var bb = new ByteBuffer(fbb.SizedReadOnlySpan().ToArray());
            int rootOffset = bb.Get<int>(bb.Position);
            int rootTablePos = bb.Position + rootOffset;
            int vtableOffset = rootTablePos - bb.Get<int>(rootTablePos);
            int fieldOffset = bb.Get<short>(vtableOffset + 4);

            Assert.IsFalse(fieldOffset == 0);

            int fieldPos = rootTablePos + fieldOffset;
            int vectorStart = fieldPos + bb.Get<int>(fieldPos);
            int count = bb.Get<int>(vectorStart);
            int dataStart = vectorStart + sizeof(int);

            Assert.AreEqual(expected.Length, count);

            for (int i = 0; i < expected.Length; i++)
            {
                float actual = bb.Get<float>(dataStart + (i * sizeof(float)));
                Assert.AreEqual(expected[i], actual);
            }
        }

        [FlatBuffersTestMethod]
        public void FlatBufferBuilder_Add_null_String()
        {
            var fbb = new FlatBufferBuilder(16);
            string s = null;
            Assert.AreEqual(fbb.CreateSharedString(s).Value, 0);
            Assert.AreEqual(fbb.CreateString(s).Value, 0);
        }

        [FlatBuffersTestMethod]
        public void FlatBufferBuilder_Empty_Builder()
        {
            var fbb = new FlatBufferBuilder(16);
            var str = "Hello";
            var flatbuffer = "Flatbuffers!";
            var strOffset = fbb.CreateSharedString(str);
            var flatbufferOffset = fbb.CreateSharedString(flatbuffer);
            fbb.Clear();
            var flatbufferOffset2 = fbb.CreateSharedString(flatbuffer);
            var strOffset2 = fbb.CreateSharedString(str);
            Assert.IsFalse(strOffset.Value == strOffset2.Value);
            Assert.IsFalse(flatbufferOffset.Value == flatbufferOffset2.Value);
        }

        [FlatBuffersTestMethod]
        public void FlatBufferBuilder_VTable_GrowsWhenTableHasMoreFieldsThanInitialSize()
        {
            var fbb = new FlatBufferBuilder(
                new ByteBuffer(256),
                vtableSpace: new int[2],
                vtableOffsetSpace: new int[16]);
            fbb.ForceDefaults = true;

            const int numFields = 8;
            fbb.StartTable(numFields);

            for (int i = 0; i < numFields; i++)
            {
                fbb.Add<int>(i, i + 100, 0);
            }

            int tableOffset = fbb.EndTable();
            fbb.Finish(tableOffset);

            var bytes = fbb.SizedReadOnlySpan();
            Assert.IsTrue(bytes.Length > 0);

            var bb = new ByteBuffer(bytes.ToArray());
            int rootOffset = bb.Get<int>(bb.Position);
            int tableStart = bb.Position + rootOffset;
            int vtableOffset = tableStart - bb.Get<int>(tableStart);

            short vtableSize = bb.Get<short>(vtableOffset);
            int expectedVtableSize = (2 + numFields) * sizeof(short);
            Assert.AreEqual(expectedVtableSize, vtableSize);
        }

        [FlatBuffersTestMethod]
        public void FlatBufferBuilder_VTables_GrowsWhenMoreTablesThanInitialSize()
        {
            var fbb = new FlatBufferBuilder(
                new ByteBuffer(1024),
                vtableSpace: new int[16],
                vtableOffsetSpace: new int[2]);
            fbb.ForceDefaults = true;

            const int numTables = 8;
            Span<int> tableOffsets = stackalloc int[numTables];

            for (int t = 0; t < numTables; t++)
            {
                // Create tables with different field counts to ensure unique vtables
                int fieldCount = t + 1;
                fbb.StartTable(fieldCount);

                for (int f = 0; f < fieldCount; f++)
                {
                    fbb.Add<int>(f, (t * 100) + f, 0);
                }

                tableOffsets[t] = fbb.EndTable();
            }

            fbb.StartVector(sizeof(int), numTables, sizeof(int));
            fbb.AddOffsetSpan(tableOffsets);
            var tableVector = fbb.EndVector();

            // Create a wrapper table containing all the sub-tables
            fbb.StartTable(1);
            fbb.AddOffset(0, tableVector.Value, 0);
            var rootTable = fbb.EndTable();
            fbb.Finish(rootTable);

            var bytes = fbb.SizedReadOnlySpan();
            Assert.IsTrue(bytes.Length > 0);

            var bb = new ByteBuffer(bytes.ToArray());
            int rootOffset = bb.Get<int>(bb.Position);
            int rootTablePos = bb.Position + rootOffset;
            var tablePositions = ReadTablePositionsFromVector(bb, rootTablePos, 0);
            Assert.AreEqual(numTables, tablePositions.Length);

            var uniqueVtables = new System.Collections.Generic.HashSet<int>();
            for (int i = 0; i < tablePositions.Length; i++)
            {
                int vtableOffset = tablePositions[i] - bb.Get<int>(tablePositions[i]);
                uniqueVtables.Add(vtableOffset);
            }

            Assert.AreEqual(numTables, uniqueVtables.Count);
        }

        [FlatBuffersTestMethod]
        public void FlatBufferBuilder_VTableAndVTables_BothGrowDynamically()
        {
            var fbb = new FlatBufferBuilder(
                new ByteBuffer(2048),
                vtableSpace: new int[1],
                vtableOffsetSpace: new int[1]);
            fbb.ForceDefaults = true;

            // Create multiple tables each with many fields
            const int numTables = 5;
            const int numFieldsPerTable = 10;
            Span<int> tableOffsets = stackalloc int[numTables];

            for (int t = 0; t < numTables; t++)
            {
                // Start table with more fields than initial vtable capacity
                fbb.StartTable(numFieldsPerTable);

                // Add different values per table to ensure unique vtables
                for (int f = 0; f < numFieldsPerTable; f++)
                {
                    // Use different default values per table to force unique vtables
                    if (f < t + 1)
                        fbb.Add<int>(f, (t * 1000) + f, 0);
                }

                tableOffsets[t] = fbb.EndTable();
            }

            fbb.StartVector(sizeof(int), numTables, sizeof(int));
            fbb.AddOffsetSpan(tableOffsets);
            var tableVector = fbb.EndVector();

            fbb.StartTable(1);
            fbb.AddOffset(0, tableVector.Value, 0);
            var rootTable = fbb.EndTable();
            fbb.Finish(rootTable);

            var bytes = fbb.SizedReadOnlySpan();
            Assert.IsTrue(bytes.Length > 0);

            var bb = new ByteBuffer(bytes.ToArray());
            int rootOffset = bb.Get<int>(bb.Position);
            int rootTablePos = bb.Position + rootOffset;
            var tablePositions = ReadTablePositionsFromVector(bb, rootTablePos, 0);
            Assert.AreEqual(numTables, tablePositions.Length);

            var uniqueVtables = new System.Collections.Generic.HashSet<int>();
            for (int i = 0; i < tablePositions.Length; i++)
            {
                int vtableOffset = tablePositions[i] - bb.Get<int>(tablePositions[i]);
                uniqueVtables.Add(vtableOffset);
            }

            Assert.AreEqual(numTables, uniqueVtables.Count);

            int sampleTablePos = tablePositions[tablePositions.Length - 1];
            int sampleVtableOffset = sampleTablePos - bb.Get<int>(sampleTablePos);
            short vtableSize = bb.Get<short>(sampleVtableOffset);
            int expectedVtableSize = (2 + numTables) * sizeof(short);
            Assert.AreEqual(expectedVtableSize, vtableSize);
        }

        private static int[] ReadTablePositionsFromVector(ByteBuffer bb, int rootTablePos, int fieldIndex)
        {
            int vtableOffset = rootTablePos - bb.Get<int>(rootTablePos);
            int fieldOffset = bb.Get<short>(vtableOffset + 4 + (fieldIndex * sizeof(short)));
            if (fieldOffset == 0)
                return Array.Empty<int>();

            int fieldPos = rootTablePos + fieldOffset;
            int vectorStart = fieldPos + bb.Get<int>(fieldPos);
            int count = bb.Get<int>(vectorStart);
            int dataStart = vectorStart + sizeof(int);

            var positions = new int[count];
            for (int i = 0; i < count; i++)
            {
                int elemOffset = bb.Get<int>(dataStart + (i * sizeof(int)));
                positions[i] = dataStart + (i * sizeof(int)) + elemOffset;
            }

            return positions;
        }
    }
}
