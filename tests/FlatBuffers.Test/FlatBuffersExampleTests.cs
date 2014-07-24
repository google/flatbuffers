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
using NUnit.Framework;

namespace FlatBuffers.Test
{
    [TestFixture]
    public class FlatBuffersExampleTests
    {
        [Test]
        public void CanCreateNewFlatBufferFromScratch()
        {
            // Second, let's create a FlatBuffer from scratch in C#, and test it also.
            // We use an initial size of 1 to exercise the reallocation algorithm,
            // normally a size larger than the typical FlatBuffer you generate would be
            // better for performance.
            var fbb = new FlatBufferBuilder(1);

            // We set up the same values as monsterdata.json:

            var str = fbb.CreateString("MyMonster");

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
            MyGame.Example.Test.CreateTest(fbb, (short)10, (byte)20);
            MyGame.Example.Test.CreateTest(fbb, (short)30, (byte)40);
            var test4 = fbb.EndVector();


            Monster.StartMonster(fbb);
            Monster.AddPos(fbb, Vec3.CreateVec3(fbb, 1.0f, 2.0f, 3.0f, 3.0,
                                                     (byte)4, (short)5, (byte)6));
            Monster.AddHp(fbb, (short)80);
            Monster.AddName(fbb, str);
            Monster.AddInventory(fbb, inv);
            Monster.AddTestType(fbb, (byte)1);
            Monster.AddTest(fbb, mon2);
            Monster.AddTest4(fbb, test4);
            var mon = Monster.EndMonster(fbb);

            fbb.Finish(mon);


            using (var ms= new MemoryStream(fbb.Data.Data, fbb.DataStart, fbb.Offset))
            {
                var data = ms.ToArray();
                File.WriteAllBytes(@"Resources/monsterdata_cstest_wire.bin",data);
            }

            


            // Now assert the buffer
            TestBuffer(fbb.Data, fbb.DataStart);

        }

        private void TestBuffer(ByteBuffer bb, int start)
        {
            var monster = Monster.GetRootAsMonster(bb, start);

            Assert.AreEqual(80, monster.Hp());
            Assert.AreEqual(150, monster.Mana());
            Assert.AreEqual("MyMonster", monster.Name());

            var pos = monster.Pos();
            Assert.AreEqual(1.0f, pos.X());
            Assert.AreEqual(2.0f, pos.Y());
            Assert.AreEqual(3.0f, pos.Z());

            Assert.AreEqual(3.0f, pos.Test1());
            Assert.AreEqual((byte)4, pos.Test2());
            var t = pos.Test3();
            Assert.AreEqual((short)5, t.A());
            Assert.AreEqual((byte)6, t.B());

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
        }

        [Test]
        public void CanReadCppGeneratedWireFile()
        {
            var data = File.ReadAllBytes(@"Resources/monsterdata_test_wire.bin");
            var bb = new ByteBuffer(data);
            
            TestBuffer(bb, 0);
        }
    }
}
