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
using System.Collections.Generic;
using ComprehensiveTest;
using ExampleAny = MyGame.Example.Any;
using ExampleColor = MyGame.Example.Color;
using ExampleMonster = MyGame.Example.Monster;
using StackExampleMonster = MyGame.Example.StackBuffer.Monster;

namespace Google.FlatSpanBuffers.Tests
{
    [FlatBuffersTestClass]
    public class ObjectApiStackBufferTests
    {
        private static FlatSpanBufferBuilder CreateStackBufferBuilderForExampleMonster()
        {
            const int ExampleMonsterVtableSize = 64;
            var bb = new ByteSpanBuffer(new byte[1024]);
            return new FlatSpanBufferBuilder(
                bb,
                vtableSpace: new int[ExampleMonsterVtableSize],
                vtableOffsetSpace: new int[16]);
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_ObjectAPI_UnPack()
        {
            var fbb = CreateStackBufferBuilderForExampleMonster();

            Span<byte> inventoryData = stackalloc byte[] { 1, 2, 3, 4, 5 };
            var inv = StackExampleMonster.CreateInventoryVector(ref fbb, inventoryData);

            StackExampleMonster.StartTest4Vector(ref fbb, 2);
            MyGame.Example.StackBuffer.Test.CreateTest(ref fbb, (short)100, (sbyte)50);
            MyGame.Example.StackBuffer.Test.CreateTest(ref fbb, (short)200, (sbyte)60);
            var test4 = fbb.EndVector();

            var name = fbb.CreateString("UnPackMonster");
            StackExampleMonster.StartMonster(ref fbb);
            StackExampleMonster.AddName(ref fbb, name);
            StackExampleMonster.AddHp(ref fbb, 300);
            StackExampleMonster.AddMana(ref fbb, 200);
            StackExampleMonster.AddInventory(ref fbb, inv);
            StackExampleMonster.AddPos(ref fbb, MyGame.Example.StackBuffer.Vec3.CreateVec3(ref fbb,
                1.0f, 2.0f, 3.0f, 4.0, ExampleColor.Green, (short)10, (sbyte)20));
            StackExampleMonster.AddTest4(ref fbb, test4);
            StackExampleMonster.AddColor(ref fbb, ExampleColor.Red);
            var mon = StackExampleMonster.EndMonster(ref fbb);
            StackExampleMonster.FinishMonsterBuffer(ref fbb, mon);

            var bb = fbb.DataBuffer;
            var monster = StackExampleMonster.GetRootAsMonster(bb);
            var monsterT = monster.UnPack();

            Assert.AreEqual("UnPackMonster", monsterT.Name);
            Assert.AreEqual(300, monsterT.Hp);
            Assert.AreEqual(200, monsterT.Mana);
            Assert.AreEqual(ExampleColor.Red, monsterT.Color);

            Assert.AreEqual(5, monsterT.Inventory.Count);
            for (int i = 0; i < 5; i++)
            {
                Assert.AreEqual(i + 1, monsterT.Inventory[i]);
            }

            Assert.IsNotNull(monsterT.Pos);
            Assert.AreEqual(1.0f, monsterT.Pos.X, 6);
            Assert.AreEqual(2.0f, monsterT.Pos.Y, 6);
            Assert.AreEqual(3.0f, monsterT.Pos.Z, 6);
            Assert.AreEqual(4.0, monsterT.Pos.Test1, 6);
            Assert.AreEqual(ExampleColor.Green, monsterT.Pos.Test2);
            Assert.AreEqual((short)10, monsterT.Pos.Test3.A);
            Assert.AreEqual((sbyte)20, monsterT.Pos.Test3.B);

            Assert.AreEqual(2, monsterT.Test4.Count);
            Assert.AreEqual((short)200, monsterT.Test4[0].A);
            Assert.AreEqual((sbyte)60, monsterT.Test4[0].B);
            Assert.AreEqual((short)100, monsterT.Test4[1].A);
            Assert.AreEqual((sbyte)50, monsterT.Test4[1].B);
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_ObjectAPI_Pack()
        {
            var monsterT = new MyGame.Example.MonsterT
            {
                Name = "PackedMonster",
                Hp = 300,
                Mana = 200,
                Color = ExampleColor.Red,
                Inventory = new List<byte> { 1, 2, 3, 4, 5 },
                Pos = new MyGame.Example.Vec3T
                {
                    X = 1.0f,
                    Y = 2.0f,
                    Z = 3.0f,
                    Test1 = 4.0,
                    Test2 = ExampleColor.Green,
                    Test3 = new MyGame.Example.TestT { A = 10, B = 20 }
                }
            };

            var fbb = CreateStackBufferBuilderForExampleMonster();
            var monsterOffset = StackExampleMonster.Pack(ref fbb, monsterT);
            StackExampleMonster.FinishMonsterBuffer(ref fbb, monsterOffset);

            var bb = fbb.DataBuffer;
            var monster = StackExampleMonster.GetRootAsMonster(bb);

            Assert.AreEqual("PackedMonster", monster.Name);
            Assert.AreEqual(300, monster.Hp);
            Assert.AreEqual(200, monster.Mana);
            Assert.AreEqual(ExampleColor.Red, monster.Color);

            Assert.IsTrue(monster.Inventory.HasValue);
            Assert.AreEqual(5, monster.Inventory.Value.Length);

            Assert.IsTrue(monster.Pos.HasValue);
            var pos = monster.Pos.Value;
            Assert.AreEqual(1.0f, pos.X, 6);
            Assert.AreEqual(2.0f, pos.Y, 6);
            Assert.AreEqual(3.0f, pos.Z, 6);
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_ObjectAPI_UnPack_BasicTable()
        {
            var fbb = CreateStackBufferBuilderForExampleMonster();

            var nameOffset = fbb.CreateString("UnPackMonster");
            StackExampleMonster.StartMonster(ref fbb);
            StackExampleMonster.AddName(ref fbb, nameOffset);
            StackExampleMonster.AddHp(ref fbb, 300);
            StackExampleMonster.AddMana(ref fbb, 200);
            StackExampleMonster.AddColor(ref fbb, ExampleColor.Red);
            var monsterOffset = StackExampleMonster.EndMonster(ref fbb);
            StackExampleMonster.FinishMonsterBuffer(ref fbb, monsterOffset);

            var bb = fbb.DataBuffer;
            var monster = StackExampleMonster.GetRootAsMonster(bb);
            var monsterT = monster.UnPack();

            Assert.AreEqual("UnPackMonster", monsterT.Name);
            Assert.AreEqual(300, monsterT.Hp);
            Assert.AreEqual(200, monsterT.Mana);
            Assert.AreEqual(ExampleColor.Red, monsterT.Color);
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_ObjectAPI_UnPack_WithVectorOfScalars()
        {
            var fbb = CreateStackBufferBuilderForExampleMonster();

            Span<byte> inventoryData = stackalloc byte[] { 10, 20, 30, 40, 50 };
            var inv = StackExampleMonster.CreateInventoryVectorBlock(ref fbb, inventoryData);

            Span<long> longsData = stackalloc long[] { 100, 200, 300 };
            var longs = StackExampleMonster.CreateVectorOfLongsVector(ref fbb, longsData);

            var nameOffset = fbb.CreateString("VectorMonster");
            StackExampleMonster.StartMonster(ref fbb);
            StackExampleMonster.AddName(ref fbb, nameOffset);
            StackExampleMonster.AddInventory(ref fbb, inv);
            StackExampleMonster.AddVectorOfLongs(ref fbb, longs);
            var monsterOffset = StackExampleMonster.EndMonster(ref fbb);
            StackExampleMonster.FinishMonsterBuffer(ref fbb, monsterOffset);

            var bb = fbb.DataBuffer;
            var monster = StackExampleMonster.GetRootAsMonster(bb);
            var monsterT = monster.UnPack();

            Assert.AreEqual(5, monsterT.Inventory.Count);
            Assert.AreEqual(10, monsterT.Inventory[0]);
            Assert.AreEqual(50, monsterT.Inventory[4]);

            Assert.AreEqual(3, monsterT.VectorOfLongs.Count);
            Assert.AreEqual(100L, monsterT.VectorOfLongs[0]);
            Assert.AreEqual(300L, monsterT.VectorOfLongs[2]);
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_ObjectAPI_UnPack_WithVectorOfStrings()
        {
            var fbb = CreateStackBufferBuilderForExampleMonster();

            var str1 = fbb.CreateString("alpha");
            var str2 = fbb.CreateString("beta");
            var str3 = fbb.CreateString("gamma");
            Span<StringOffset> strings = stackalloc StringOffset[] { str1, str2, str3 };
            var stringsOffset = StackExampleMonster.CreateTestarrayofstringVectorBlock(ref fbb, strings);

            var nameOffset = fbb.CreateString("StringMonster");
            StackExampleMonster.StartMonster(ref fbb);
            StackExampleMonster.AddName(ref fbb, nameOffset);
            StackExampleMonster.AddTestarrayofstring(ref fbb, stringsOffset);
            var monsterOffset = StackExampleMonster.EndMonster(ref fbb);
            StackExampleMonster.FinishMonsterBuffer(ref fbb, monsterOffset);

            var bb = fbb.DataBuffer;
            var monster = StackExampleMonster.GetRootAsMonster(bb);
            var monsterT = monster.UnPack();

            Assert.AreEqual(3, monsterT.Testarrayofstring.Count);
            Assert.AreEqual("alpha", monsterT.Testarrayofstring[0]);
            Assert.AreEqual("beta", monsterT.Testarrayofstring[1]);
            Assert.AreEqual("gamma", monsterT.Testarrayofstring[2]);
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_ObjectAPI_UnPack_WithVectorOfEnums()
        {
            var fbb = CreateStackBufferBuilderForExampleMonster();

            Span<ExampleColor> colors = stackalloc ExampleColor[] { ExampleColor.Red, ExampleColor.Green, ExampleColor.Blue };
            var colorsOffset = StackExampleMonster.CreateVectorOfEnumsVector(ref fbb, colors);

            var nameOffset = fbb.CreateString("EnumMonster");
            StackExampleMonster.StartMonster(ref fbb);
            StackExampleMonster.AddName(ref fbb, nameOffset);
            StackExampleMonster.AddVectorOfEnums(ref fbb, colorsOffset);
            var monsterOffset = StackExampleMonster.EndMonster(ref fbb);
            StackExampleMonster.FinishMonsterBuffer(ref fbb, monsterOffset);

            var bb = fbb.DataBuffer;
            var monster = StackExampleMonster.GetRootAsMonster(bb);
            var monsterT = monster.UnPack();

            Assert.AreEqual(3, monsterT.VectorOfEnums.Count);
            Assert.AreEqual(ExampleColor.Red, monsterT.VectorOfEnums[0]);
            Assert.AreEqual(ExampleColor.Green, monsterT.VectorOfEnums[1]);
            Assert.AreEqual(ExampleColor.Blue, monsterT.VectorOfEnums[2]);
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_ObjectAPI_UnPack_WithStruct()
        {
            var fbb = CreateStackBufferBuilderForExampleMonster();

            var nameOffset = fbb.CreateString("StructMonster");
            StackExampleMonster.StartMonster(ref fbb);
            StackExampleMonster.AddName(ref fbb, nameOffset);
            StackExampleMonster.AddPos(ref fbb, MyGame.Example.StackBuffer.Vec3.CreateVec3(ref fbb, 1.5f, 2.5f, 3.5f, 4.5,
                                                     ExampleColor.Green, (short)100, (sbyte)50));
            var monsterOffset = StackExampleMonster.EndMonster(ref fbb);
            StackExampleMonster.FinishMonsterBuffer(ref fbb, monsterOffset);

            var bb = fbb.DataBuffer;
            var monster = StackExampleMonster.GetRootAsMonster(bb);
            var monsterT = monster.UnPack();

            Assert.IsNotNull(monsterT.Pos);
            Assert.AreEqual(1.5f, monsterT.Pos.X, 6);
            Assert.AreEqual(2.5f, monsterT.Pos.Y, 6);
            Assert.AreEqual(3.5f, monsterT.Pos.Z, 6);
            Assert.AreEqual(4.5, monsterT.Pos.Test1, 6);
            Assert.AreEqual(ExampleColor.Green, monsterT.Pos.Test2);
            Assert.AreEqual((short)100, monsterT.Pos.Test3.A);
            Assert.AreEqual((sbyte)50, monsterT.Pos.Test3.B);
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_ObjectAPI_UnPack_WithVectorOfStructs()
        {
            var fbb = CreateStackBufferBuilderForExampleMonster();

            StackExampleMonster.StartTest4Vector(ref fbb, 3);
            MyGame.Example.StackBuffer.Test.CreateTest(ref fbb, (short)100, (sbyte)10);
            MyGame.Example.StackBuffer.Test.CreateTest(ref fbb, (short)200, (sbyte)20);
            MyGame.Example.StackBuffer.Test.CreateTest(ref fbb, (short)300, (sbyte)30);
            var test4 = fbb.EndVector();

            var nameOffset = fbb.CreateString("StructVecMonster");
            StackExampleMonster.StartMonster(ref fbb);
            StackExampleMonster.AddName(ref fbb, nameOffset);
            StackExampleMonster.AddTest4(ref fbb, test4);
            var monsterOffset = StackExampleMonster.EndMonster(ref fbb);
            StackExampleMonster.FinishMonsterBuffer(ref fbb, monsterOffset);

            var bb = fbb.DataBuffer;
            var monster = StackExampleMonster.GetRootAsMonster(bb);
            var monsterT = monster.UnPack();

            Assert.AreEqual(3, monsterT.Test4.Count);
            Assert.AreEqual((short)300, monsterT.Test4[0].A);
            Assert.AreEqual((sbyte)30, monsterT.Test4[0].B);
            Assert.AreEqual((short)100, monsterT.Test4[2].A);
            Assert.AreEqual((sbyte)10, monsterT.Test4[2].B);
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_ObjectAPI_UnPackTo_ReusesExistingObject()
        {
            var fbb = CreateStackBufferBuilderForExampleMonster();

            var nameOffset = fbb.CreateString("ReusedMonster");
            StackExampleMonster.StartMonster(ref fbb);
            StackExampleMonster.AddName(ref fbb, nameOffset);
            StackExampleMonster.AddHp(ref fbb, 500);
            StackExampleMonster.AddMana(ref fbb, 250);
            var monsterOffset = StackExampleMonster.EndMonster(ref fbb);
            StackExampleMonster.FinishMonsterBuffer(ref fbb, monsterOffset);

            var bb = fbb.DataBuffer;
            var monster = StackExampleMonster.GetRootAsMonster(bb);

            var existingMonsterT = new MyGame.Example.MonsterT();
            existingMonsterT.Name = "OldName";
            existingMonsterT.Hp = 50;

            monster.UnPackTo(existingMonsterT);

            Assert.AreEqual("ReusedMonster", existingMonsterT.Name);
            Assert.AreEqual(500, existingMonsterT.Hp);
            Assert.AreEqual(250, existingMonsterT.Mana);
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_ObjectAPI_UnPackTo_WithVectors_ReusesLists()
        {
            var fbb1 = CreateStackBufferBuilderForExampleMonster();
            Span<byte> inv1 = stackalloc byte[] { 1, 2, 3, 4, 5 };
            var invOffset1 = StackExampleMonster.CreateInventoryVectorBlock(ref fbb1, inv1);
            var name1 = fbb1.CreateString("Monster1");
            StackExampleMonster.StartMonster(ref fbb1);
            StackExampleMonster.AddName(ref fbb1, name1);
            StackExampleMonster.AddInventory(ref fbb1, invOffset1);
            var monsterOffset1 = StackExampleMonster.EndMonster(ref fbb1);
            StackExampleMonster.FinishMonsterBuffer(ref fbb1, monsterOffset1);

            var bb1 = fbb1.DataBuffer;
            var monster1 = StackExampleMonster.GetRootAsMonster(bb1);
            var monsterT = monster1.UnPack();
            var originalList = monsterT.Inventory;

            var fbb2 = CreateStackBufferBuilderForExampleMonster();
            Span<byte> inv2 = stackalloc byte[] { 10, 20, 30 };
            var invOffset2 = StackExampleMonster.CreateInventoryVectorBlock(ref fbb2, inv2);
            var name2 = fbb2.CreateString("Monster2");
            StackExampleMonster.StartMonster(ref fbb2);
            StackExampleMonster.AddName(ref fbb2, name2);
            StackExampleMonster.AddInventory(ref fbb2, invOffset2);
            var monsterOffset2 = StackExampleMonster.EndMonster(ref fbb2);
            StackExampleMonster.FinishMonsterBuffer(ref fbb2, monsterOffset2);

            var bb2 = fbb2.DataBuffer;
            var monster2 = StackExampleMonster.GetRootAsMonster(bb2);
            monster2.UnPackTo(monsterT);

            Assert.IsTrue(ReferenceEquals(originalList, monsterT.Inventory));
            Assert.AreEqual(3, monsterT.Inventory.Count);
            Assert.AreEqual(10, monsterT.Inventory[0]);
            Assert.AreEqual(30, monsterT.Inventory[2]);
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_ObjectAPI_UnPack_WithUnion()
        {
            var fbb = CreateStackBufferBuilderForExampleMonster();

            var nestedName = fbb.CreateString("NestedMonster");
            StackExampleMonster.StartMonster(ref fbb);
            StackExampleMonster.AddName(ref fbb, nestedName);
            StackExampleMonster.AddHp(ref fbb, 999);
            var nestedMonster = StackExampleMonster.EndMonster(ref fbb);

            var mainName = fbb.CreateString("UnionMonster");
            StackExampleMonster.StartMonster(ref fbb);
            StackExampleMonster.AddName(ref fbb, mainName);
            StackExampleMonster.AddTestType(ref fbb, ExampleAny.Monster);
            StackExampleMonster.AddTest(ref fbb, nestedMonster.Value);
            var mainMonster = StackExampleMonster.EndMonster(ref fbb);
            StackExampleMonster.FinishMonsterBuffer(ref fbb, mainMonster);

            var bb = fbb.DataBuffer;
            var monster = StackExampleMonster.GetRootAsMonster(bb);
            var monsterT = monster.UnPack();

            Assert.IsNotNull(monsterT.Test);
            Assert.AreEqual(ExampleAny.Monster, monsterT.Test.Type);
            var nestedT = monsterT.Test.Value as MyGame.Example.MonsterT;
            Assert.IsNotNull(nestedT);
            Assert.AreEqual("NestedMonster", nestedT.Name);
            Assert.AreEqual(999, nestedT.Hp);
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_ObjectAPI_UnPack_WithVectorOfTables()
        {
            var fbb = CreateStackBufferBuilderForExampleMonster();

            var name1 = fbb.CreateString("Child1");
            StackExampleMonster.StartMonster(ref fbb);
            StackExampleMonster.AddName(ref fbb, name1);
            StackExampleMonster.AddHp(ref fbb, 100);
            var child1 = StackExampleMonster.EndMonster(ref fbb);

            var name2 = fbb.CreateString("Child2");
            StackExampleMonster.StartMonster(ref fbb);
            StackExampleMonster.AddName(ref fbb, name2);
            StackExampleMonster.AddHp(ref fbb, 200);
            var child2 = StackExampleMonster.EndMonster(ref fbb);

            Span<Offset<StackExampleMonster>> children = stackalloc Offset<StackExampleMonster>[] { child1, child2 };
            var childrenOffset = StackExampleMonster.CreateTestarrayoftablesVectorBlock(ref fbb, children);

            var parentName = fbb.CreateString("Parent");
            StackExampleMonster.StartMonster(ref fbb);
            StackExampleMonster.AddName(ref fbb, parentName);
            StackExampleMonster.AddTestarrayoftables(ref fbb, childrenOffset);
            var parentMonster = StackExampleMonster.EndMonster(ref fbb);
            StackExampleMonster.FinishMonsterBuffer(ref fbb, parentMonster);

            var bb = fbb.DataBuffer;
            var monster = StackExampleMonster.GetRootAsMonster(bb);
            var monsterT = monster.UnPack();

            Assert.AreEqual(2, monsterT.Testarrayoftables.Count);
            Assert.AreEqual("Child1", monsterT.Testarrayoftables[0].Name);
            Assert.AreEqual(100, monsterT.Testarrayoftables[0].Hp);
            Assert.AreEqual("Child2", monsterT.Testarrayoftables[1].Name);
            Assert.AreEqual(200, monsterT.Testarrayoftables[1].Hp);
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_ObjectAPI_DefaultValuesInConstructor()
        {
            var monsterT = new MyGame.Example.MonsterT();

            Assert.IsNull(monsterT.Name);
            Assert.AreEqual(150, monsterT.Mana);
            Assert.AreEqual(100, monsterT.Hp);
            Assert.AreEqual(ExampleColor.Blue, monsterT.Color);
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_ObjectAPI_DefaultValuesPreservedAfterRoundTrip()
        {
            var monsterT = new MyGame.Example.MonsterT
            {
                Name = "DefaultMonster"
            };

            var fbb = CreateStackBufferBuilderForExampleMonster();
            var offset = StackExampleMonster.Pack(ref fbb, monsterT);
            StackExampleMonster.FinishMonsterBuffer(ref fbb, offset);

            var bb = fbb.DataBuffer;
            var monster = StackExampleMonster.GetRootAsMonster(bb);
            var unpacked = monster.UnPack();

            Assert.AreEqual("DefaultMonster", unpacked.Name);
            Assert.AreEqual(150, unpacked.Mana);
            Assert.AreEqual(100, unpacked.Hp);
            Assert.AreEqual(ExampleColor.Blue, unpacked.Color);
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_ObjectAPI_RoundTrip_ComplexObject()
        {
            var originalMonster = new MyGame.Example.MonsterT
            {
                Name = "ComplexMonster",
                Hp = 999,
                Mana = 500,
                Color = ExampleColor.Red,
                Inventory = new List<byte> { 1, 2, 3, 4, 5 },
                Testarrayofstring = new List<string> { "tag1", "tag2" },
                VectorOfLongs = new List<long> { 100, 200, 300 },
                VectorOfEnums = new List<ExampleColor> { ExampleColor.Red, ExampleColor.Green },
                Pos = new MyGame.Example.Vec3T
                {
                    X = 1.0f,
                    Y = 2.0f,
                    Z = 3.0f,
                    Test1 = 4.0,
                    Test2 = ExampleColor.Blue,
                    Test3 = new MyGame.Example.TestT { A = 10, B = 20 }
                }
            };

            var fbb = CreateStackBufferBuilderForExampleMonster();
            var offset = StackExampleMonster.Pack(ref fbb, originalMonster);
            StackExampleMonster.FinishMonsterBuffer(ref fbb, offset);

            var bb = fbb.DataBuffer;
            var monster = StackExampleMonster.GetRootAsMonster(bb);
            var unpacked = monster.UnPack();

            Assert.AreEqual(originalMonster.Name, unpacked.Name);
            Assert.AreEqual(originalMonster.Hp, unpacked.Hp);
            Assert.AreEqual(originalMonster.Mana, unpacked.Mana);
            Assert.AreEqual(originalMonster.Color, unpacked.Color);

            Assert.AreEqual(originalMonster.Inventory.Count, unpacked.Inventory.Count);
            for (int i = 0; i < originalMonster.Inventory.Count; i++)
            {
                Assert.AreEqual(originalMonster.Inventory[i], unpacked.Inventory[i]);
            }

            Assert.AreEqual(originalMonster.Testarrayofstring.Count, unpacked.Testarrayofstring.Count);
            for (int i = 0; i < originalMonster.Testarrayofstring.Count; i++)
            {
                Assert.AreEqual(originalMonster.Testarrayofstring[i], unpacked.Testarrayofstring[i]);
            }

            Assert.AreEqual(originalMonster.VectorOfLongs.Count, unpacked.VectorOfLongs.Count);
            for (int i = 0; i < originalMonster.VectorOfLongs.Count; i++)
            {
                Assert.AreEqual(originalMonster.VectorOfLongs[i], unpacked.VectorOfLongs[i]);
            }

            Assert.AreEqual(originalMonster.VectorOfEnums.Count, unpacked.VectorOfEnums.Count);
            for (int i = 0; i < originalMonster.VectorOfEnums.Count; i++)
            {
                Assert.AreEqual(originalMonster.VectorOfEnums[i], unpacked.VectorOfEnums[i]);
            }

            Assert.IsNotNull(unpacked.Pos);
            Assert.AreEqual(originalMonster.Pos.X, unpacked.Pos.X, 6);
            Assert.AreEqual(originalMonster.Pos.Y, unpacked.Pos.Y, 6);
            Assert.AreEqual(originalMonster.Pos.Z, unpacked.Pos.Z, 6);
            Assert.AreEqual(originalMonster.Pos.Test1, unpacked.Pos.Test1, 6);
            Assert.AreEqual(originalMonster.Pos.Test2, unpacked.Pos.Test2);
            Assert.AreEqual(originalMonster.Pos.Test3.A, unpacked.Pos.Test3.A);
            Assert.AreEqual(originalMonster.Pos.Test3.B, unpacked.Pos.Test3.B);
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_ObjectAPI_RoundTrip_MultipleUnpackOperations()
        {
            var monster1 = new MyGame.Example.MonsterT
            {
                Name = "Monster1",
                Hp = 100
            };

            var fbb1 = CreateStackBufferBuilderForExampleMonster();
            var offset1 = StackExampleMonster.Pack(ref fbb1, monster1);
            StackExampleMonster.FinishMonsterBuffer(ref fbb1, offset1);

            var bb1 = fbb1.DataBuffer;
            var readMonster1 = StackExampleMonster.GetRootAsMonster(bb1);
            var monster2 = readMonster1.UnPack();

            monster2.Hp = 200;

            var fbb2 = CreateStackBufferBuilderForExampleMonster();
            var offset2 = StackExampleMonster.Pack(ref fbb2, monster2);
            StackExampleMonster.FinishMonsterBuffer(ref fbb2, offset2);

            var bb2 = fbb2.DataBuffer;
            var readMonster2 = StackExampleMonster.GetRootAsMonster(bb2);
            var monster3 = readMonster2.UnPack();

            Assert.AreEqual("Monster1", monster3.Name);
            Assert.AreEqual(200, monster3.Hp);
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_ObjectAPI_UnPack_WithEnemy()
        {
            var fbb = CreateStackBufferBuilderForExampleMonster();

            var enemyName = fbb.CreateString("Enemy");
            StackExampleMonster.StartMonster(ref fbb);
            StackExampleMonster.AddName(ref fbb, enemyName);
            StackExampleMonster.AddHp(ref fbb, 50);
            var enemy = StackExampleMonster.EndMonster(ref fbb);

            var mainName = fbb.CreateString("Hero");
            StackExampleMonster.StartMonster(ref fbb);
            StackExampleMonster.AddName(ref fbb, mainName);
            StackExampleMonster.AddEnemy(ref fbb, enemy);
            var main = StackExampleMonster.EndMonster(ref fbb);
            StackExampleMonster.FinishMonsterBuffer(ref fbb, main);

            var bb = fbb.DataBuffer;
            var monster = StackExampleMonster.GetRootAsMonster(bb);
            var monsterT = monster.UnPack();

            Assert.IsNotNull(monsterT.Enemy);
            Assert.AreEqual("Enemy", monsterT.Enemy.Name);
            Assert.AreEqual(50, monsterT.Enemy.Hp);
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_ObjectAPI_UnPack_NullVectors_ReturnsEmptyLists()
        {
            var fbb = CreateStackBufferBuilderForExampleMonster();

            var nameOffset = fbb.CreateString("EmptyMonster");
            StackExampleMonster.StartMonster(ref fbb);
            StackExampleMonster.AddName(ref fbb, nameOffset);
            var monsterOffset = StackExampleMonster.EndMonster(ref fbb);
            StackExampleMonster.FinishMonsterBuffer(ref fbb, monsterOffset);

            var bb = fbb.DataBuffer;
            var monster = StackExampleMonster.GetRootAsMonster(bb);
            var monsterT = monster.UnPack();

            Assert.IsNotNull(monsterT.Inventory);
            Assert.AreEqual(0, monsterT.Inventory.Count);

            Assert.IsNotNull(monsterT.Testarrayofstring);
            Assert.AreEqual(0, monsterT.Testarrayofstring.Count);

            Assert.IsNotNull(monsterT.VectorOfLongs);
            Assert.AreEqual(0, monsterT.VectorOfLongs.Count);
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_ObjectAPI_CrossVariant_SameResults()
        {
            var fbb = new FlatBufferBuilder(128);

            Span<byte> inventoryData = stackalloc byte[] { 1, 2, 3 };
            var inv = ExampleMonster.CreateInventoryVectorBlock(fbb, inventoryData);

            var nameOffset = fbb.CreateString("CrossVariant");
            ExampleMonster.StartMonster(fbb);
            ExampleMonster.AddName(fbb, nameOffset);
            ExampleMonster.AddInventory(fbb, inv);
            ExampleMonster.AddHp(fbb, 123);
            ExampleMonster.AddMana(fbb, 456);
            ExampleMonster.AddColor(fbb, ExampleColor.Green);
            var monsterOffset = ExampleMonster.EndMonster(fbb);
            ExampleMonster.FinishMonsterBuffer(fbb, monsterOffset);

            var regularMonster = ExampleMonster.GetRootAsMonster(fbb.DataBuffer);
            var regularUnpacked = regularMonster.UnPack();

            var bb = Utils.ByteBufferUtil.ToSizedByteSpanBuffer(fbb.DataBuffer);
            var stackMonster = StackExampleMonster.GetRootAsMonster(bb);
            var stackUnpacked = stackMonster.UnPack();

            Assert.AreEqual(regularUnpacked.Name, stackUnpacked.Name);
            Assert.AreEqual(regularUnpacked.Hp, stackUnpacked.Hp);
            Assert.AreEqual(regularUnpacked.Mana, stackUnpacked.Mana);
            Assert.AreEqual(regularUnpacked.Color, stackUnpacked.Color);
            Assert.AreEqual(regularUnpacked.Inventory.Count, stackUnpacked.Inventory.Count);
            for (int i = 0; i < regularUnpacked.Inventory.Count; i++)
            {
                Assert.AreEqual(regularUnpacked.Inventory[i], stackUnpacked.Inventory[i]);
            }
        }
    }
}
