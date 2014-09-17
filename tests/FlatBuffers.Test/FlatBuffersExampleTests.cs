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
using MyGame.Example;

namespace FlatBuffers.Test
{
    public class FlatBuffersExampleTests
    {
        public void RunTests()
        {
            CanCreateNewFlatBufferFromScratch();
            CanReadCppGeneratedWireFile();
        }

        public void CanCreateNewFlatBufferFromScratch()
        {
            // Second, let's create a FlatBuffer from scratch in C#, and test it also.
            // We use an initial size of 1 to exercise the reallocation algorithm,
            // normally a size larger than the typical FlatBuffer you generate would be
            // better for performance.
            var fbb = new FlatBufferBuilder(1);

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

            Monster.StartMonster(fbb);
            Monster.AddHp(fbb, (short)20);
            var mon2 = Monster.EndMonster(fbb);

            Monster.StartTest4Vector(fbb, 2);
            MyGame.Example.Test.CreateTest(fbb, (short)10, (sbyte)20);
            MyGame.Example.Test.CreateTest(fbb, (short)30, (sbyte)40);
            var test4 = fbb.EndVector();

            Monster.StartTestarrayofstringVector(fbb, 2);
            fbb.AddOffset(test2);
            fbb.AddOffset(test1);
            var testArrayOfString = fbb.EndVector();


            Monster.StartMonster(fbb);
            Monster.AddPos(fbb, Vec3.CreateVec3(fbb, 1.0f, 2.0f, 3.0f, 3.0,
                                                     (sbyte)4, (short)5, (sbyte)6));
            Monster.AddHp(fbb, (short)80);
            Monster.AddName(fbb, str);
            Monster.AddInventory(fbb, inv);
            Monster.AddTestType(fbb, (byte)1);
            Monster.AddTest(fbb, mon2);
            Monster.AddTest4(fbb, test4);
            Monster.AddTestarrayofstring(fbb, testArrayOfString);
            var mon = Monster.EndMonster(fbb);

            fbb.Finish(mon);

            // Dump to output directory so we can inspect later, if needed
            using (var ms = new MemoryStream(fbb.DataBuffer().Data, fbb.DataBuffer().position(), fbb.Offset()))
            {
                var data = ms.ToArray();
                File.WriteAllBytes(@"Resources/monsterdata_cstest.bin",data);
            }

            // Now assert the buffer
            TestBuffer(fbb.DataBuffer());
        }

        private void TestBuffer(ByteBuffer bb)
        {
            var monster = Monster.GetRootAsMonster(bb);

            Assert.AreEqual(80, monster.Hp());
            Assert.AreEqual(150, monster.Mana());
            Assert.AreEqual("MyMonster", monster.Name());

            var pos = monster.Pos();
            Assert.AreEqual(1.0f, pos.X());
            Assert.AreEqual(2.0f, pos.Y());
            Assert.AreEqual(3.0f, pos.Z());

            Assert.AreEqual(3.0f, pos.Test1());
            Assert.AreEqual((sbyte)4, pos.Test2());
            var t = pos.Test3();
            Assert.AreEqual((short)5, t.A());
            Assert.AreEqual((sbyte)6, t.B());

            Assert.AreEqual((byte)Any.Monster, monster.TestType());

            var monster2 = new Monster();
            Assert.IsTrue(monster.Test(monster2) != null);
            Assert.AreEqual(20, monster2.Hp());


            Assert.AreEqual(5, monster.InventoryLength());
            var invsum = 0;
            for (var i = 0; i < monster.InventoryLength(); i++)
            {
                invsum += monster.Inventory(i);
            }
            Assert.AreEqual(10, invsum);

            var test0 = monster.Test4(0);
            var test1 = monster.Test4(1);
            Assert.AreEqual(2, monster.Test4Length());

            Assert.AreEqual(100, test0.A() + test0.B() + test1.A() + test1.B());


            Assert.AreEqual(2, monster.TestarrayofstringLength());
            Assert.AreEqual("test1", monster.Testarrayofstring(0));
            Assert.AreEqual("test2", monster.Testarrayofstring(1));
        }

        public void CanReadCppGeneratedWireFile()
        {
            var data = File.ReadAllBytes(@"Resources/monsterdata_test.bin");
            var bb = new ByteBuffer(data);
            TestBuffer(bb);
        }
    }
}
