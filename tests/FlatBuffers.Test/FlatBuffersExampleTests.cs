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

using System.IO;
using System.Text;
using MyGame.Example;

namespace FlatBuffers.Test
{
    [FlatBuffersTestClass]
    public class FlatBuffersExampleTests
    {
        public void RunTests()
        {
            CanCreateNewFlatBufferFromScratch();
            CanReadCppGeneratedWireFile();
            TestEnums();
        }

        [FlatBuffersTestMethod]
        public void CanCreateNewFlatBufferFromScratch()
        {
            // Second, let's create a FlatBuffer from scratch in C#, and test it also.
            // We use an initial size of 1 to exercise the reallocation algorithm,
            // normally a size larger than the typical FlatBuffer you generate would be
            // better for performance.
            var fbb = new FlatBufferBuilder(1);

            StringOffset[] names = { fbb.CreateString("Frodo"), fbb.CreateString("Barney"), fbb.CreateString("Wilma") };
            Offset<Monster>[] off = new Offset<Monster>[3];
            Monster.StartMonster(fbb);
            Monster.AddName(fbb, names[0]);
            off[0] = Monster.EndMonster(fbb);
            Monster.StartMonster(fbb);
            Monster.AddName(fbb, names[1]);
            off[1] = Monster.EndMonster(fbb);
            Monster.StartMonster(fbb);
            Monster.AddName(fbb, names[2]);
            off[2] = Monster.EndMonster(fbb);
            var sortMons = Monster.CreateSortedVectorOfMonster(fbb, off);

            // We set up the same values as monsterdata.json:

            var str = fbb.CreateString("MyMonster");
            var test1 = fbb.CreateString("test1");
            var test2 = fbb.CreateString("test2");


            Monster.StartInventoryVector(fbb, 5);
            for (int i = 4; i >= 0; i--)
            {
                fbb.AddByte((byte)i);
            }
            var inv = fbb.EndVector();

            var fred = fbb.CreateString("Fred");
            Monster.StartMonster(fbb);
            Monster.AddName(fbb, fred);
            var mon2 = Monster.EndMonster(fbb);

            Monster.StartTest4Vector(fbb, 2);
            MyGame.Example.Test.CreateTest(fbb, (short)10, (sbyte)20);
            MyGame.Example.Test.CreateTest(fbb, (short)30, (sbyte)40);
            var test4 = fbb.EndVector();

            Monster.StartTestarrayofstringVector(fbb, 2);
            fbb.AddOffset(test2.Value);
            fbb.AddOffset(test1.Value);
            var testArrayOfString = fbb.EndVector();

            Monster.StartMonster(fbb);
            Monster.AddPos(fbb, Vec3.CreateVec3(fbb, 1.0f, 2.0f, 3.0f, 3.0,
                                                     Color.Green, (short)5, (sbyte)6));
            Monster.AddHp(fbb, (short)80);
            Monster.AddName(fbb, str);
            Monster.AddInventory(fbb, inv);
            Monster.AddTestType(fbb, Any.Monster);
            Monster.AddTest(fbb, mon2.Value);
            Monster.AddTest4(fbb, test4);
            Monster.AddTestarrayofstring(fbb, testArrayOfString);
            Monster.AddTestbool(fbb, false);
            Monster.AddTestarrayoftables(fbb, sortMons);
            var mon = Monster.EndMonster(fbb);

            Monster.FinishMonsterBuffer(fbb, mon);


            // Dump to output directory so we can inspect later, if needed
            using (var ms = new MemoryStream(fbb.DataBuffer.Data, fbb.DataBuffer.Position, fbb.Offset))
            {
                var data = ms.ToArray();
                File.WriteAllBytes(@"Resources/monsterdata_cstest.mon",data);
            }

            // Now assert the buffer
            TestBuffer(fbb.DataBuffer);

            //Attempt to mutate Monster fields and check whether the buffer has been mutated properly
            // revert to original values after testing
            Monster monster = Monster.GetRootAsMonster(fbb.DataBuffer);

            // mana is optional and does not exist in the buffer so the mutation should fail
            // the mana field should retain its default value
            Assert.AreEqual(monster.MutateMana((short)10), false);
            Assert.AreEqual(monster.Mana, (short)150);

            // Accessing a vector of sorted by the key tables
            Assert.AreEqual(monster.Testarrayoftables(0).Value.Name, "Barney");
            Assert.AreEqual(monster.Testarrayoftables(1).Value.Name, "Frodo");
            Assert.AreEqual(monster.Testarrayoftables(2).Value.Name, "Wilma");

            // Example of searching for a table by the key
            Assert.IsTrue(monster.TestarrayoftablesByKey("Frodo") != null);
            Assert.IsTrue(monster.TestarrayoftablesByKey("Barney") != null);
            Assert.IsTrue(monster.TestarrayoftablesByKey("Wilma") != null);

            // testType is an existing field and mutating it should succeed
            Assert.AreEqual(monster.TestType, Any.Monster);
            Assert.AreEqual(monster.MutateTestType(Any.NONE), true);
            Assert.AreEqual(monster.TestType, Any.NONE);
            Assert.AreEqual(monster.MutateTestType(Any.Monster), true);
            Assert.AreEqual(monster.TestType, Any.Monster);

            //mutate the inventory vector
            Assert.AreEqual(monster.MutateInventory(0, 1), true);
            Assert.AreEqual(monster.MutateInventory(1, 2), true);
            Assert.AreEqual(monster.MutateInventory(2, 3), true);
            Assert.AreEqual(monster.MutateInventory(3, 4), true);
            Assert.AreEqual(monster.MutateInventory(4, 5), true);

            for (int i = 0; i < monster.InventoryLength; i++)
            {
                Assert.AreEqual(monster.Inventory(i), i + 1);
            }

            //reverse mutation
            Assert.AreEqual(monster.MutateInventory(0, 0), true);
            Assert.AreEqual(monster.MutateInventory(1, 1), true);
            Assert.AreEqual(monster.MutateInventory(2, 2), true);
            Assert.AreEqual(monster.MutateInventory(3, 3), true);
            Assert.AreEqual(monster.MutateInventory(4, 4), true);

            // get a struct field and edit one of its fields
            Vec3 pos = (Vec3)monster.Pos;
            Assert.AreEqual(pos.X, 1.0f);
            pos.MutateX(55.0f);
            Assert.AreEqual(pos.X, 55.0f);
            pos.MutateX(1.0f);
            Assert.AreEqual(pos.X, 1.0f);

            TestBuffer(fbb.DataBuffer);
        }

        private void TestBuffer(ByteBuffer bb)
        {
            var monster = Monster.GetRootAsMonster(bb);

            Assert.AreEqual(80, monster.Hp);
            Assert.AreEqual(150, monster.Mana);
            Assert.AreEqual("MyMonster", monster.Name);

            var pos = monster.Pos.Value;
            Assert.AreEqual(1.0f, pos.X);
            Assert.AreEqual(2.0f, pos.Y);
            Assert.AreEqual(3.0f, pos.Z);

            Assert.AreEqual(3.0f, pos.Test1);
            Assert.AreEqual(Color.Green, pos.Test2);
            var t = (MyGame.Example.Test)pos.Test3;
            Assert.AreEqual((short)5, t.A);
            Assert.AreEqual((sbyte)6, t.B);

            Assert.AreEqual(Any.Monster, monster.TestType);

            var monster2 = monster.Test<Monster>().Value;
            Assert.AreEqual("Fred", monster2.Name);


            Assert.AreEqual(5, monster.InventoryLength);
            var invsum = 0;
            for (var i = 0; i < monster.InventoryLength; i++)
            {
                invsum += monster.Inventory(i);
            }
            Assert.AreEqual(10, invsum);

            var test0 = monster.Test4(0).Value;
            var test1 = monster.Test4(1).Value;
            Assert.AreEqual(2, monster.Test4Length);

            Assert.AreEqual(100, test0.A + test0.B + test1.A + test1.B);

            Assert.AreEqual(2, monster.TestarrayofstringLength);
            Assert.AreEqual("test1", monster.Testarrayofstring(0));
            Assert.AreEqual("test2", monster.Testarrayofstring(1));

            Assert.AreEqual(false, monster.Testbool);

            var nameBytes = monster.GetNameBytes().Value;
            Assert.AreEqual("MyMonster", Encoding.UTF8.GetString(nameBytes.Array, nameBytes.Offset, nameBytes.Count));

            if (0 == monster.TestarrayofboolsLength)
            {
                Assert.IsFalse(monster.GetTestarrayofboolsBytes().HasValue);
            }
            else
            {
                Assert.IsTrue(monster.GetTestarrayofboolsBytes().HasValue);
            }
        }

        [FlatBuffersTestMethod]
        public void CanReadCppGeneratedWireFile()
        {
            var data = File.ReadAllBytes(@"Resources/monsterdata_test.mon");
            var bb = new ByteBuffer(data);
            TestBuffer(bb);
        }

        [FlatBuffersTestMethod]
        public void TestEnums()
        {
            Assert.AreEqual("Red", Color.Red.ToString());
            Assert.AreEqual("Blue", Color.Blue.ToString());
            Assert.AreEqual("NONE", Any.NONE.ToString());
            Assert.AreEqual("Monster", Any.Monster.ToString());
        }

        [FlatBuffersTestMethod]
        public void TestNestedFlatBuffer()
        {
            const string nestedMonsterName = "NestedMonsterName";
            const short nestedMonsterHp = 600;
            const short nestedMonsterMana = 1024;
            // Create nested buffer as a Monster type
            var fbb1 = new FlatBufferBuilder(16);
            var str1 = fbb1.CreateString(nestedMonsterName);
            Monster.StartMonster(fbb1);
            Monster.AddName(fbb1, str1);
            Monster.AddHp(fbb1, nestedMonsterHp);
            Monster.AddMana(fbb1, nestedMonsterMana);
            var monster1 = Monster.EndMonster(fbb1);
            Monster.FinishMonsterBuffer(fbb1, monster1);
            var fbb1Bytes = fbb1.SizedByteArray();
            fbb1 = null;

            // Create a Monster which has the first buffer as a nested buffer
            var fbb2 = new FlatBufferBuilder(16);
            var str2 = fbb2.CreateString("My Monster");
            var nestedBuffer = Monster.CreateTestnestedflatbufferVector(fbb2, fbb1Bytes);
            Monster.StartMonster(fbb2);
            Monster.AddName(fbb2, str2);
            Monster.AddHp(fbb2, 50);
            Monster.AddMana(fbb2, 32);
            Monster.AddTestnestedflatbuffer(fbb2, nestedBuffer);
            var monster = Monster.EndMonster(fbb2);
            Monster.FinishMonsterBuffer(fbb2, monster);

            // Now test the data extracted from the nested buffer
            var mons = Monster.GetRootAsMonster(fbb2.DataBuffer);
            var nestedMonster = mons.GetTestnestedflatbufferAsMonster().Value;

            Assert.AreEqual(nestedMonsterMana, nestedMonster.Mana);
            Assert.AreEqual(nestedMonsterHp, nestedMonster.Hp);
            Assert.AreEqual(nestedMonsterName, nestedMonster.Name);
        }
    }
}
