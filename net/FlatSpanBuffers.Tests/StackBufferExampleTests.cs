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
using System.Text;
using Google.FlatSpanBuffers.Utils;
using MyGame.Example;

using SpanMonster = MyGame.Example.StackBuffer.Monster;

namespace Google.FlatSpanBuffers.Tests
{
    [FlatBuffersTestClass]
    public class StackBufferExampleTests
    {
        private static FlatSpanBufferBuilder CreateSpanBuilderForMonster()
        {
            // MyGame.Example.Monster table has 62 fields.
            const int MonsterVtableSize = 64;
            var bb = new ByteSpanBuffer(new byte[1024]);
            FlatSpanBufferBuilder fbb = new FlatSpanBufferBuilder(bb, 
                vtableSpace: new int[MonsterVtableSize], vtableOffsetSpace: new int[16]);
            return fbb;
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_CanReadFlatBufferFromScratch()
        {
            var fbb = CreateSpanBuilderForMonster();

            var str = fbb.CreateString("MyMonster");
            var test1 = fbb.CreateString("test1");
            var test2 = fbb.CreateString("test2");

            Span<byte> inventoryData = stackalloc byte[] { 0, 1, 2, 3, 4 };
            var inv = SpanMonster.CreateInventoryVectorBlock(ref fbb, inventoryData);

            var fred = fbb.CreateString("Fred");
            SpanMonster.StartMonster(ref fbb);
            SpanMonster.AddName(ref fbb, fred);
            var mon2 = SpanMonster.EndMonster(ref fbb);

            SpanMonster.StartTest4Vector(ref fbb, 2);
            MyGame.Example.StackBuffer.Test.CreateTest(ref fbb, (short)10, (sbyte)20);
            MyGame.Example.StackBuffer.Test.CreateTest(ref fbb, (short)30, (sbyte)40);
            var test4 = fbb.EndVector();

            Span<StringOffset> testStrings = stackalloc StringOffset[] { test1, test2 };
            var testArrayOfString = SpanMonster.CreateTestarrayofstringVectorBlock(ref fbb, testStrings);

            Span<long> longsData = stackalloc long[] { 1, 100, 10000, 1000000, 100000000 };
            var longsVector = SpanMonster.CreateVectorOfLongsVector(ref fbb, longsData);

            Span<double> doublesData = stackalloc double[] { -1.7976931348623157e+308, 0, 1.7976931348623157e+308 };
            var doublesVector = SpanMonster.CreateVectorOfDoublesVector(ref fbb, doublesData);

            SpanMonster.StartMonster(ref fbb);
            SpanMonster.AddPos(ref fbb, MyGame.Example.StackBuffer.Vec3.CreateVec3(ref fbb, 1.0f, 2.0f, 3.0f, 3.0,
                                                     Color.Green, (short)5, (sbyte)6));
            SpanMonster.AddHp(ref fbb, (short)80);
            SpanMonster.AddName(ref fbb, str);
            SpanMonster.AddInventory(ref fbb, inv);
            SpanMonster.AddTestType(ref fbb, Any.Monster);
            SpanMonster.AddTest(ref fbb, mon2.Value);
            SpanMonster.AddTest4(ref fbb, test4);
            SpanMonster.AddTestarrayofstring(ref fbb, testArrayOfString);
            SpanMonster.AddTestbool(ref fbb, true);
            SpanMonster.AddVectorOfLongs(ref fbb, longsVector);
            SpanMonster.AddVectorOfDoubles(ref fbb, doublesVector);
            var mon = SpanMonster.EndMonster(ref fbb);

            SpanMonster.FinishMonsterBuffer(ref fbb, mon);

            TestBufferWithStackBuffer(fbb.DataBuffer);
        }

        private void TestBufferWithStackBuffer(ByteSpanBuffer bb)
        {
            bool test = SpanMonster.VerifyMonster(bb);
            Assert.AreEqual(true, test);

            SpanMonster monster = SpanMonster.GetRootAsMonster(bb);

            Assert.AreEqual(80, monster.Hp);
            Assert.AreEqual(150, monster.Mana);
            Assert.AreEqual("MyMonster", monster.Name);

            var pos = monster.Pos.Value;
            Assert.AreEqual(1.0f, pos.X, 6);
            Assert.AreEqual(2.0f, pos.Y, 6);
            Assert.AreEqual(3.0f, pos.Z, 6);

            Assert.AreEqual(3.0, pos.Test1, 6);
            Assert.AreEqual(Color.Green, pos.Test2);
            var t = pos.Test3;
            Assert.AreEqual((short)5, t.A);
            Assert.AreEqual((sbyte)6, t.B);

            Assert.AreEqual(Any.Monster, monster.TestType);

            var monster2 = monster.Test<SpanMonster>().Value;
            Assert.AreEqual("Fred", monster2.Name);

            Assert.AreEqual(5, monster.Inventory.Value.Length);
            var invsum = 0;
            for (var i = 0; i < monster.Inventory.Value.Length; i++)
            {
                invsum += monster.Inventory.Value[i];
            }
            Assert.AreEqual(10, invsum);

            var test4 = monster.Test4;
            Assert.IsTrue(test4.HasValue);
            var test4Vec = test4.Value;
            Assert.AreEqual(2, test4Vec.Length);
            var test0 = test4Vec[0];
            var test1 = test4Vec[1];

            Assert.AreEqual(100, test0.A + test0.B + test1.A + test1.B);

            var testarrayofstring = monster.Testarrayofstring;
            Assert.IsTrue(testarrayofstring.HasValue);
            var testarrayofstringVec = testarrayofstring.Value;
            Assert.AreEqual(2, testarrayofstringVec.Length);
            Assert.AreEqual("test1", testarrayofstringVec[0]);
            Assert.AreEqual("test2", testarrayofstringVec[1]);

            Assert.AreEqual(true, monster.Testbool);

            var nameBytes = monster.GetNameBytes();
            Assert.AreEqual("MyMonster", Encoding.UTF8.GetString(nameBytes.ToArray(), 0, nameBytes.Length));

            var longArray = monster.VectorOfLongs.Value;
            Assert.AreEqual(5, longArray.Length);
            Assert.AreEqual(100, longArray[1]);

            var doublesArray = monster.VectorOfDoubles.Value;
            Assert.AreEqual(3, doublesArray.Length);
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_TestVectorOfEnums()
        {
            const string monsterName = "TestVectorOfEnumsMonster";
            Span<Color> colorVec = stackalloc Color[] { Color.Red, Color.Green, Color.Blue };

            var fbb = CreateSpanBuilderForMonster();

            var str1 = fbb.CreateString(monsterName);
            var vec1 = SpanMonster.CreateVectorOfEnumsVector(ref fbb, colorVec);
            SpanMonster.StartMonster(ref fbb);
            SpanMonster.AddName(ref fbb, str1);
            SpanMonster.AddVectorOfEnums(ref fbb, vec1);
            var monster1 = SpanMonster.EndMonster(ref fbb);
            SpanMonster.FinishMonsterBuffer(ref fbb, monster1);

            var mons = SpanMonster.GetRootAsMonster(fbb.DataBuffer);
            var colors = mons.VectorOfEnums.Value;
            Assert.AreEqual(3, colors.Length);
            Assert.AreEqual(Color.Red, colors[0]);
            Assert.AreEqual(Color.Green, colors[1]);
            Assert.AreEqual(Color.Blue, colors[2]);
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_TestDefaultValues()
        {
            var fbb = CreateSpanBuilderForMonster();

            var nameOffset = fbb.CreateString("DefaultMonster");
            SpanMonster.StartMonster(ref fbb);
            SpanMonster.AddName(ref fbb, nameOffset);
            var monsterOffset = SpanMonster.EndMonster(ref fbb);
            SpanMonster.FinishMonsterBuffer(ref fbb, monsterOffset);

            var monster = SpanMonster.GetRootAsMonster(fbb.DataBuffer);

            // Check default values from schema
            Assert.AreEqual(150, monster.Mana);
            Assert.AreEqual(100, monster.Hp);
            Assert.AreEqual(Color.Blue, monster.Color);
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_TestStringAsSpan()
        {
            var fbb = CreateSpanBuilderForMonster();

            var name = "UnicodeMonster_\u00E4\u00F6\u00FC";
            var nameOffset = fbb.CreateString(name);
            SpanMonster.StartMonster(ref fbb);
            SpanMonster.AddName(ref fbb, nameOffset);
            var monsterOffset = SpanMonster.EndMonster(ref fbb);
            SpanMonster.FinishMonsterBuffer(ref fbb, monsterOffset);

            var monster = SpanMonster.GetRootAsMonster(fbb.DataBuffer);

            Assert.AreEqual(name, monster.Name);
            var nameBytes = monster.GetNameBytes();
            Assert.AreEqual(name, Encoding.UTF8.GetString(nameBytes.ToArray(), 0, nameBytes.Length));
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_TestInventoryAsSpan()
        {
            var fbb = CreateSpanBuilderForMonster();

            Span<byte> inventoryData = stackalloc byte[] { 10, 20, 30, 40, 50 };
            var inv = SpanMonster.CreateInventoryVectorBlock(ref fbb, inventoryData);

            var nameOffset = fbb.CreateString("InventoryMonster");
            SpanMonster.StartMonster(ref fbb);
            SpanMonster.AddName(ref fbb, nameOffset);
            SpanMonster.AddInventory(ref fbb, inv);
            var monsterOffset = SpanMonster.EndMonster(ref fbb);
            SpanMonster.FinishMonsterBuffer(ref fbb, monsterOffset);

            var monster = SpanMonster.GetRootAsMonster(fbb.DataBuffer);

            Assert.IsTrue(monster.Inventory.HasValue);
            var invSpan = monster.Inventory.Value;
            Assert.AreEqual(5, invSpan.Length);
            Assert.AreEqual(10, invSpan[0]);
            Assert.AreEqual(50, invSpan[4]);
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_TestUnion()
        {
            var fbb = CreateSpanBuilderForMonster();

            var nestedName = fbb.CreateString("NestedMonster");
            SpanMonster.StartMonster(ref fbb);
            SpanMonster.AddName(ref fbb, nestedName);
            SpanMonster.AddHp(ref fbb, 500);
            var nestedMonster = SpanMonster.EndMonster(ref fbb);

            var mainName = fbb.CreateString("MainMonster");
            SpanMonster.StartMonster(ref fbb);
            SpanMonster.AddName(ref fbb, mainName);
            SpanMonster.AddTestType(ref fbb, Any.Monster);
            SpanMonster.AddTest(ref fbb, nestedMonster.Value);
            var mainMonster = SpanMonster.EndMonster(ref fbb);
            SpanMonster.FinishMonsterBuffer(ref fbb, mainMonster);

            var monster = SpanMonster.GetRootAsMonster(fbb.DataBuffer);

            Assert.AreEqual(Any.Monster, monster.TestType);
            var nested = monster.Test<SpanMonster>().Value;
            Assert.AreEqual("NestedMonster", nested.Name);
            Assert.AreEqual(500, nested.Hp);
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_TestVectorOfStructs()
        {
            var fbb = CreateSpanBuilderForMonster();

            SpanMonster.StartTest4Vector(ref fbb, 3);
            MyGame.Example.StackBuffer.Test.CreateTest(ref fbb, (short)100, (sbyte)10);
            MyGame.Example.StackBuffer.Test.CreateTest(ref fbb, (short)200, (sbyte)20);
            MyGame.Example.StackBuffer.Test.CreateTest(ref fbb, (short)300, (sbyte)30);
            var test4 = fbb.EndVector();

            var nameOffset = fbb.CreateString("StructVectorMonster");
            SpanMonster.StartMonster(ref fbb);
            SpanMonster.AddName(ref fbb, nameOffset);
            SpanMonster.AddTest4(ref fbb, test4);
            var monsterOffset = SpanMonster.EndMonster(ref fbb);
            SpanMonster.FinishMonsterBuffer(ref fbb, monsterOffset);

            var monster = SpanMonster.GetRootAsMonster(fbb.DataBuffer);

            Assert.IsTrue(monster.Test4.HasValue);
            var test4Vec = monster.Test4.Value;
            Assert.AreEqual(3, test4Vec.Length);
            Assert.AreEqual((short)100, test4Vec[2].A);
            Assert.AreEqual((sbyte)10, test4Vec[2].B);
            Assert.AreEqual((short)300, test4Vec[0].A);
            Assert.AreEqual((sbyte)30, test4Vec[0].B);
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_TestEmptyBuffer()
        {
            var fbb = CreateSpanBuilderForMonster();

            var nameOffset = fbb.CreateString("EmptyMonster");
            SpanMonster.StartMonster(ref fbb);
            SpanMonster.AddName(ref fbb, nameOffset);
            var monsterOffset = SpanMonster.EndMonster(ref fbb);
            SpanMonster.FinishMonsterBuffer(ref fbb, monsterOffset);

            var monster = SpanMonster.GetRootAsMonster(fbb.DataBuffer);

            Assert.AreEqual("EmptyMonster", monster.Name);
            Assert.IsFalse(monster.Inventory.HasValue);
            Assert.IsFalse(monster.Test4.HasValue);
            Assert.IsFalse(monster.Testarrayoftables.HasValue);
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_TestVectorOfLongs()
        {
            var fbb = CreateSpanBuilderForMonster();

            Span<long> longsData = stackalloc long[] { long.MinValue, -1, 0, 1, long.MaxValue };
            var longsVector = SpanMonster.CreateVectorOfLongsVector(ref fbb, longsData);

            var nameOffset = fbb.CreateString("LongsMonster");
            SpanMonster.StartMonster(ref fbb);
            SpanMonster.AddName(ref fbb, nameOffset);
            SpanMonster.AddVectorOfLongs(ref fbb, longsVector);
            var monsterOffset = SpanMonster.EndMonster(ref fbb);
            SpanMonster.FinishMonsterBuffer(ref fbb, monsterOffset);

            var monster = SpanMonster.GetRootAsMonster(fbb.DataBuffer);

            Assert.IsTrue(monster.VectorOfLongs.HasValue);
            var longs = monster.VectorOfLongs.Value;
            Assert.AreEqual(5, longs.Length);
            Assert.AreEqual(long.MinValue, longs[0]);
            Assert.AreEqual(long.MaxValue, longs[4]);
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_TestVectorOfDoubles()
        {
            var fbb = CreateSpanBuilderForMonster();

            Span<double> doublesData = stackalloc double[] { double.MinValue, -1.5, 0, 1.5, double.MaxValue };
            var doublesVector = SpanMonster.CreateVectorOfDoublesVector(ref fbb, doublesData);

            var nameOffset = fbb.CreateString("DoublesMonster");
            SpanMonster.StartMonster(ref fbb);
            SpanMonster.AddName(ref fbb, nameOffset);
            SpanMonster.AddVectorOfDoubles(ref fbb, doublesVector);
            var monsterOffset = SpanMonster.EndMonster(ref fbb);
            SpanMonster.FinishMonsterBuffer(ref fbb, monsterOffset);

            var monster = SpanMonster.GetRootAsMonster(fbb.DataBuffer);

            Assert.IsTrue(monster.VectorOfDoubles.HasValue);
            var doubles = monster.VectorOfDoubles.Value;
            Assert.AreEqual(5, doubles.Length);
            Assert.AreEqual(double.MinValue, doubles[0], 6);
            Assert.AreEqual(double.MaxValue, doubles[4], 6);
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_TestArrayOfBools()
        {
            var fbb = CreateSpanBuilderForMonster();

            Span<bool> boolsData = stackalloc bool[] { true, false, true, false, true };
            var boolsVector = SpanMonster.CreateTestarrayofboolsVector(ref fbb, boolsData);

            var nameOffset = fbb.CreateString("BoolsMonster");
            SpanMonster.StartMonster(ref fbb);
            SpanMonster.AddName(ref fbb, nameOffset);
            SpanMonster.AddTestarrayofbools(ref fbb, boolsVector);
            var monsterOffset = SpanMonster.EndMonster(ref fbb);
            SpanMonster.FinishMonsterBuffer(ref fbb, monsterOffset);

            var monster = SpanMonster.GetRootAsMonster(fbb.DataBuffer);

            Assert.IsTrue(monster.Testarrayofbools.HasValue);
            var bools = monster.Testarrayofbools.Value;
            Assert.AreEqual(5, bools.Length);
            Assert.AreEqual(true, bools[0]);
            Assert.AreEqual(false, bools[1]);
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_TestEnemy()
        {
            var fbb = CreateSpanBuilderForMonster();

            // Create enemy monster
            var enemyName = fbb.CreateString("Enemy");
            SpanMonster.StartMonster(ref fbb);
            SpanMonster.AddName(ref fbb, enemyName);
            SpanMonster.AddHp(ref fbb, 50);
            var enemy = SpanMonster.EndMonster(ref fbb);

            // Create main monster with enemy
            var mainName = fbb.CreateString("MainWithEnemy");
            SpanMonster.StartMonster(ref fbb);
            SpanMonster.AddName(ref fbb, mainName);
            SpanMonster.AddEnemy(ref fbb, enemy);
            var main = SpanMonster.EndMonster(ref fbb);
            SpanMonster.FinishMonsterBuffer(ref fbb, main);

            var monster = SpanMonster.GetRootAsMonster(fbb.DataBuffer);

            Assert.IsTrue(monster.Enemy.HasValue);
            var enemyMonster = monster.Enemy.Value;
            Assert.AreEqual("Enemy", enemyMonster.Name);
            Assert.AreEqual(50, enemyMonster.Hp);
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_TestNanAndInfinityDefaults()
        {
            var fbb = CreateSpanBuilderForMonster();

            var nameOffset = fbb.CreateString("SpecialFloatsMonster");
            SpanMonster.StartMonster(ref fbb);
            SpanMonster.AddName(ref fbb, nameOffset);
            var monsterOffset = SpanMonster.EndMonster(ref fbb);
            SpanMonster.FinishMonsterBuffer(ref fbb, monsterOffset);

            var monster = SpanMonster.GetRootAsMonster(fbb.DataBuffer);

            // Test default values for nan and infinity fields
            Assert.IsTrue(float.IsNaN(monster.NanDefault));
            Assert.IsTrue(float.IsPositiveInfinity(monster.InfDefault));
            Assert.IsTrue(float.IsPositiveInfinity(monster.PositiveInfDefault));
            Assert.IsTrue(float.IsPositiveInfinity(monster.InfinityDefault));
            Assert.IsTrue(float.IsPositiveInfinity(monster.PositiveInfinityDefault));
            Assert.IsTrue(float.IsNegativeInfinity(monster.NegativeInfDefault));
            Assert.IsTrue(float.IsNegativeInfinity(monster.NegativeInfinityDefault));
            Assert.IsTrue(double.IsPositiveInfinity(monster.DoubleInfDefault));
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_TestVerifier_ValidBuffer()
        {
            var fbb = CreateSpanBuilderForMonster();

            var nameOffset = fbb.CreateString("VerifyMonster");
            SpanMonster.StartMonster(ref fbb);
            SpanMonster.AddName(ref fbb, nameOffset);
            SpanMonster.AddHp(ref fbb, 100);
            var monsterOffset = SpanMonster.EndMonster(ref fbb);
            SpanMonster.FinishMonsterBuffer(ref fbb, monsterOffset);

            Assert.IsTrue(SpanMonster.VerifyMonster(fbb.DataBuffer));

            Span<byte> corruptedData = stackalloc byte[8];
            var invalidBuffer = new ByteSpanBuffer(corruptedData);
            Assert.IsFalse(SpanMonster.VerifyMonster(invalidBuffer));
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_TestTableVector()
        {
            var fbb = CreateSpanBuilderForMonster();

            var name1 = fbb.CreateString("Monster1");
            var name2 = fbb.CreateString("Monster2");
            var name3 = fbb.CreateString("Monster3");

            SpanMonster.StartMonster(ref fbb);
            SpanMonster.AddName(ref fbb, name1);
            var mon1 = SpanMonster.EndMonster(ref fbb);

            SpanMonster.StartMonster(ref fbb);
            SpanMonster.AddName(ref fbb, name2);
            var mon2 = SpanMonster.EndMonster(ref fbb);

            SpanMonster.StartMonster(ref fbb);
            SpanMonster.AddName(ref fbb, name3);
            var mon3 = SpanMonster.EndMonster(ref fbb);

            Span<Offset<SpanMonster>> monstersSpan = stackalloc Offset<SpanMonster>[] { mon1, mon2, mon3 };
            var monstersVector = SpanMonster.CreateTestarrayoftablesVectorBlock(ref fbb, monstersSpan);

            var mainName = fbb.CreateString("MainMonster");
            SpanMonster.StartMonster(ref fbb);
            SpanMonster.AddName(ref fbb, mainName);
            SpanMonster.AddTestarrayoftables(ref fbb, monstersVector);
            var mainMonster = SpanMonster.EndMonster(ref fbb);
            SpanMonster.FinishMonsterBuffer(ref fbb, mainMonster);

            var monster = SpanMonster.GetRootAsMonster(fbb.DataBuffer);

            Assert.IsTrue(monster.Testarrayoftables.HasValue);
            var monsters = monster.Testarrayoftables.Value;
            Assert.AreEqual(3, monsters.Length);
            Assert.AreEqual("Monster1", monsters[0].Name);
            Assert.AreEqual("Monster2", monsters[1].Name);
            Assert.AreEqual("Monster3", monsters[2].Name);
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_TestStructAccess()
        {
            var fbb = CreateSpanBuilderForMonster();

            var nameOffset = fbb.CreateString("StructMonster");
            SpanMonster.StartMonster(ref fbb);
            SpanMonster.AddName(ref fbb, nameOffset);
            SpanMonster.AddPos(ref fbb, MyGame.Example.StackBuffer.Vec3.CreateVec3(ref fbb, 
                1.5f, 2.5f, 3.5f, 4.5, Color.Red, (short)100, (sbyte)50));
            var monsterOffset = SpanMonster.EndMonster(ref fbb);
            SpanMonster.FinishMonsterBuffer(ref fbb, monsterOffset);

            var monster = SpanMonster.GetRootAsMonster(fbb.DataBuffer);

            Assert.IsTrue(monster.Pos.HasValue);
            var pos = monster.Pos.Value;

            Assert.AreEqual(1.5f, pos.X, 6);
            Assert.AreEqual(2.5f, pos.Y, 6);
            Assert.AreEqual(3.5f, pos.Z, 6);
            Assert.AreEqual(4.5, pos.Test1, 6);
            Assert.AreEqual(Color.Red, pos.Test2);
            Assert.AreEqual((short)100, pos.Test3.A);
            Assert.AreEqual((sbyte)50, pos.Test3.B);
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_TestStackAllocatedBufferReading()
        {
            var builder = new FlatBufferBuilder(128);
            var nameOffset = builder.CreateString("StackMonster");
            Monster.StartMonster(builder);
            Monster.AddName(builder, nameOffset);
            Monster.AddHp(builder, 999);
            var monsterOffset = Monster.EndMonster(builder);
            Monster.FinishMonsterBuffer(builder, monsterOffset);
            
            var serializedData = builder.DataBuffer.ToSizedSpan();
            Span<byte> stackBuffer = stackalloc byte[serializedData.Length];
            serializedData.CopyTo(stackBuffer);

            var spanBuffer = new ByteSpanBuffer(stackBuffer);
            var monster = SpanMonster.GetRootAsMonster(spanBuffer);
            Assert.AreEqual("StackMonster", monster.Name);
            Assert.AreEqual(999, monster.Hp);
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_TestMutateAndVerify()
        {
            var fbb = CreateSpanBuilderForMonster();

            var nameOffset = fbb.CreateString("VerifyMutate");
            SpanMonster.StartMonster(ref fbb);
            SpanMonster.AddName(ref fbb, nameOffset);
            SpanMonster.AddHp(ref fbb, 50);  // Use non-default value so it's in the buffer
            var monsterOffset = SpanMonster.EndMonster(ref fbb);
            SpanMonster.FinishMonsterBuffer(ref fbb, monsterOffset);

            // Verify before mutation
            Assert.IsTrue(SpanMonster.VerifyMonster(fbb.DataBuffer));

            var monster = SpanMonster.GetRootAsMonster(fbb.DataBuffer);
            Assert.AreEqual(50, monster.Hp);
            Assert.IsTrue(monster.MutateHp(200));
            Assert.AreEqual(200, monster.Hp);

            // Verify after mutation - buffer should still be valid
            Assert.IsTrue(SpanMonster.VerifyMonster(fbb.DataBuffer));

            // Read again and verify mutated value persists
            var monster2 = SpanMonster.GetRootAsMonster(fbb.DataBuffer);
            Assert.AreEqual(200, monster2.Hp);
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_TestCrossVariantCompatibility()
        {
            var fbb = new FlatBufferBuilder(128);

            Span<byte> inventoryData = stackalloc byte[] { 1, 2, 3 };
            var inv = Monster.CreateInventoryVectorBlock(fbb, inventoryData);

            var nameOffset = fbb.CreateString("CrossVariant");
            Monster.StartMonster(fbb);
            Monster.AddName(fbb, nameOffset);
            Monster.AddInventory(fbb, inv);
            Monster.AddHp(fbb, 123);
            var monsterOffset = Monster.EndMonster(fbb);
            Monster.FinishMonsterBuffer(fbb, monsterOffset);

            // Read with regular Monster
            var regularMonster = Monster.GetRootAsMonster(fbb.DataBuffer);
            Assert.AreEqual("CrossVariant", regularMonster.Name);
            Assert.AreEqual(123, regularMonster.Hp);

            // Read with StackBuffer Monster
            var bb = ByteBufferUtil.ToSizedByteSpanBuffer(fbb.DataBuffer);
            var spanMonster = SpanMonster.GetRootAsMonster(bb);
            Assert.AreEqual("CrossVariant", spanMonster.Name);
            Assert.AreEqual(123, spanMonster.Hp);

            // Both should read the same data
            Assert.AreEqual(regularMonster.Hp, spanMonster.Hp);
            Assert.AreEqual(regularMonster.Name, spanMonster.Name);
        }
    }
}
