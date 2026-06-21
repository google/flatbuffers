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
using MyGame.Example;
using SpanMonster = MyGame.Example.StackBuffer.Monster;

namespace Google.FlatSpanBuffers.Tests
{
    [FlatBuffersTestClass]
    public class VectorEnumeratorTests
    {
        private static ByteBuffer BuildMonsterBuffer()
        {
            var fbb = new FlatBufferBuilder(512);

            var name0 = fbb.CreateString("Barney");
            var name1 = fbb.CreateString("Frodo");
            var name2 = fbb.CreateString("Wilma");

            Span<Offset<Monster>> monOffsets = stackalloc Offset<Monster>[3];
            Monster.StartMonster(fbb);
            Monster.AddName(fbb, name0);
            monOffsets[0] = Monster.EndMonster(fbb);
            Monster.StartMonster(fbb);
            Monster.AddName(fbb, name1);
            monOffsets[1] = Monster.EndMonster(fbb);
            Monster.StartMonster(fbb);
            Monster.AddName(fbb, name2);
            monOffsets[2] = Monster.EndMonster(fbb);
            var tableVec = Monster.CreateSortedVectorOfMonster(fbb, monOffsets);

            Monster.StartTest4Vector(fbb, 2);
            Test.CreateTest(fbb, (short)10, (sbyte)20);
            Test.CreateTest(fbb, (short)30, (sbyte)40);
            var structVec = fbb.EndVector();

            Span<StringOffset> strings = stackalloc StringOffset[2];
            strings[0] = fbb.CreateString("test1");
            strings[1] = fbb.CreateString("test2");
            var stringVec = Monster.CreateTestarrayofstringVectorBlock(fbb, strings);

            Monster.StartMonster(fbb);
            Monster.AddTestarrayoftables(fbb, tableVec);
            Monster.AddTest4(fbb, structVec);
            Monster.AddTestarrayofstring(fbb, stringVec);
            var mon = Monster.EndMonster(fbb);
            Monster.FinishMonsterBuffer(fbb, mon);

            return fbb.DataBuffer;
        }

        [FlatBuffersTestMethod]
        public void StringVector_ForEach()
        {
            var bb = BuildMonsterBuffer();
            string[] expected = { "test1", "test2" };

            var monster = Monster.GetRootAsMonster(bb);
            var testArrVec = monster.Testarrayofstring.Value;
            int i = 0;
            foreach (var s in testArrVec)
            {
                Assert.AreEqual(s, expected[i++]);
            }

            var spanMonster = SpanMonster.GetRootAsMonster(new ByteSpanBuffer(bb));
            var testArrSpanVec = spanMonster.Testarrayofstring.Value;
            i = 0;
            foreach (var s in testArrSpanVec)
            {
                Assert.AreEqual(s, expected[i++]);
            }
        }

        [FlatBuffersTestMethod]
        public void StructVector_ForEach()
        {
            var bb = BuildMonsterBuffer();
            (short A, sbyte B)[] expected = { (30, 40), (10, 20) };

            var monster =  Monster.GetRootAsMonster(bb);
            var structVec = monster.Test4.Value;
            int i = 0;
            foreach (var t in structVec)
            {
                Assert.AreEqual(expected[i].A, t.A);
                Assert.AreEqual(expected[i++].B, t.B);
            }

            var spanMonster = SpanMonster.GetRootAsMonster(new ByteSpanBuffer(bb));
            var structSpanVec = spanMonster.Test4.Value;
            i = 0;
            foreach (var t in structSpanVec)
            {
                Assert.AreEqual(expected[i].A, t.A);
                Assert.AreEqual(expected[i++].B, t.B);
            }
        }

        [FlatBuffersTestMethod]
        public void TableVector_ForEach()
        {
            var bb = BuildMonsterBuffer();
            string[] expected = { "Barney", "Frodo", "Wilma" };

            var tableVec = Monster.GetRootAsMonster(bb).Testarrayoftables.Value;
            int i = 0;
            foreach (var m in tableVec)
            {
                Assert.AreEqual(expected[i++], m.Name);
            }

            var tableSpanVec = SpanMonster.GetRootAsMonster(new ByteSpanBuffer(bb)).Testarrayoftables.Value;
            i = 0;
            foreach (var m in tableSpanVec)
            {
                Assert.AreEqual(expected[i++], m.Name);
            }
        }
    }
}
