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
using System.IO;
using System.Text;
using System.Threading;
using Google.FlatSpanBuffers.Utils;
using MyGame.Example;
using optional_scalars;
using KeywordTest;

namespace Google.FlatSpanBuffers.Tests
{
    // Copied from: tests/FlatBuffers.Test/FlatBuffersExampleTests.cs
    // Adapted for Google.FlatSpanBuffers namespace and API
    [FlatBuffersTestClass]
    public class FlatBuffersExampleTests
    {
        [FlatBuffersTestMethod]
        public void CanCreateNewFlatBufferFromScratch()
        {
            CanCreateNewFlatBufferFromScratch(false);
        }

        [FlatBuffersTestMethod]
        public void CanCreateNewFlatBufferFromScratchWithSizePrefix()
        {
            CanCreateNewFlatBufferFromScratch(true);
        }

        private void CanCreateNewFlatBufferFromScratch(bool sizePrefix)
        {
            // Second, let's create a FlatBuffer from scratch in C#, and test it also.
            // We use an initial size of 1 to exercise the reallocation algorithm,
            // normally a size larger than the typical FlatBuffer you generate would be
            // better for performance.
            var fbb = new FlatBufferBuilder(1);

            // Create sorted monster names (Frodo, Barney, Wilma - will be sorted alphabetically)
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
            Span<Offset<Monster>> monstersSpan = off;
            var sortMons = Monster.CreateSortedVectorOfMonster(fbb, monstersSpan);

            // We set up the same values as monsterdata.json:

            var str = fbb.CreateString("MyMonster");
            var test1 = fbb.CreateString("test1");
            var test2 = fbb.CreateString("test2");

            Span<byte> inventoryData = stackalloc byte[] { 0, 1, 2, 3, 4 };
            var inv = Monster.CreateInventoryVectorBlock(fbb, inventoryData);

            var fred = fbb.CreateString("Fred");
            Monster.StartMonster(fbb);
            Monster.AddName(fbb, fred);
            var mon2 = Monster.EndMonster(fbb);

            Monster.StartTest4Vector(fbb, 2);
            MyGame.Example.Test.CreateTest(fbb, (short)10, (sbyte)20);
            MyGame.Example.Test.CreateTest(fbb, (short)30, (sbyte)40);
            var test4 = fbb.EndVector();

            Span<StringOffset> testStrings = stackalloc StringOffset[] { test1, test2 };
            var testArrayOfString = Monster.CreateTestarrayofstringVectorBlock(fbb, testStrings);

            var longsVector = Monster.CreateVectorOfLongsVector(fbb, new long[] { 1, 100, 10000, 1000000, 100000000 });
            var doublesVector = Monster.CreateVectorOfDoublesVector(fbb, new double[] { -1.7976931348623157e+308, 0, 1.7976931348623157e+308 });

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
            Monster.AddTestbool(fbb, true);
            Monster.AddTestarrayoftables(fbb, sortMons);
            Monster.AddVectorOfLongs(fbb, longsVector);
            Monster.AddVectorOfDoubles(fbb, doublesVector);
            var mon = Monster.EndMonster(fbb);

            if (sizePrefix)
            {
                Monster.FinishSizePrefixedMonsterBuffer(fbb, mon);
            }
            else
            {
                Monster.FinishMonsterBuffer(fbb, mon);
            }

            // Remove the size prefix if necessary for further testing
            ByteBuffer dataBuffer = fbb.DataBuffer;
            if (sizePrefix)
            {
                Assert.AreEqual(dataBuffer.GetSizePrefix() + FlatBufferConstants.SizePrefixLength,
                                dataBuffer.Length - dataBuffer.Position);
                dataBuffer.RemoveSizePrefix();
            }

            // Now assert the buffer
            TestBuffer(dataBuffer);

            // Accessing a vector of sorted by the key tables
            Monster monster = Monster.GetRootAsMonster(dataBuffer);
            var testarrayoftables = monster.Testarrayoftables;
            Assert.IsTrue(testarrayoftables.HasValue);
            var testarrayoftablesVec = testarrayoftables.Value;
            Assert.AreEqual(testarrayoftablesVec[0].Name, "Barney");
            Assert.AreEqual(testarrayoftablesVec[1].Name, "Frodo");
            Assert.AreEqual(testarrayoftablesVec[2].Name, "Wilma");

            // Example of searching for a table by the key
            Assert.IsTrue(monster.TryGetTestarrayoftablesByKey("Frodo", out var frodo));
            Assert.AreEqual(frodo.Name, "Frodo");
            Assert.IsTrue(monster.TryGetTestarrayoftablesByKey("Barney", out var barney));
            Assert.AreEqual(barney.Name, "Barney");
            Assert.IsTrue(monster.TryGetTestarrayoftablesByKey("Wilma", out var wilma));
            Assert.AreEqual(wilma.Name, "Wilma");

            // Test Object API
            TestObjectAPI(monster);
        }

        private void TestBuffer(ByteBuffer bb)
        {
            bool test = Monster.VerifyMonster(bb);
            Assert.AreEqual(true, test);

            Monster monster = Monster.GetRootAsMonster(bb);

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

            var monster2 = monster.Test<Monster>().Value;
            Assert.AreEqual("Fred", monster2.Name);

            Assert.AreEqual(5, monster.Inventory.Value.Length);
            var invsum = 0;
            for (var i = 0; i < monster.Inventory.Value.Length; i++)
            {
                invsum += monster.Inventory.Value[i];
            }
            Assert.AreEqual(10, invsum);

            // Get the inventory as span and subtract the sum to get it back to 0
            var inventorySpan = monster.Inventory.Value;
            Assert.AreEqual(5, inventorySpan.Length);
            foreach (var inv in inventorySpan)
            {
                invsum -= inv;
            }
            Assert.AreEqual(0, invsum);

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

            var testarrayofbools = monster.Testarrayofbools;
            if (!testarrayofbools.HasValue)
            {
                Assert.IsFalse(testarrayofbools.HasValue);
            }
            else
            {
                Assert.IsTrue(testarrayofbools.Value.Length != 0);
            }

            var vectorOfLongs = monster.VectorOfLongs;
            Assert.IsTrue(vectorOfLongs.HasValue);
            var longArray = vectorOfLongs.Value;
            Assert.IsTrue(longArray.Length * 8 > 0);

            var vectorOfDoubles = monster.VectorOfDoubles;
            Assert.IsTrue(vectorOfDoubles.HasValue);
            var doublesArray = vectorOfDoubles.Value;
            Assert.IsTrue(doublesArray.Length * 8 > 0);
        }

        [FlatBuffersTestMethod]
        public void CanReadCppGeneratedWireFile()
        {
            var data = File.ReadAllBytes(Path.Combine(AppContext.BaseDirectory, "monsterdata_test.mon"));
            var bb = new ByteBuffer(data);
            TestBuffer(bb);
            TestObjectAPI(Monster.GetRootAsMonster(bb));
        }

        [FlatBuffersTestMethod]
        public void CanReadJsonFile()
        {
            var jsonText = File.ReadAllText(Path.Combine(AppContext.BaseDirectory, "monsterdata_test.json"));
            var mon = MonsterT.DeserializeFromJson(jsonText);
            var fbb = new FlatBufferBuilder(1);
            Monster.FinishMonsterBuffer(fbb, Monster.Pack(fbb, mon));
            TestBuffer(fbb.DataBuffer);
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
        public void TestVectorOfEnums()
        {
            const string monsterName = "TestVectorOfEnumsMonster";
            Span<Color> colorVec = stackalloc Color[] { Color.Red, Color.Green, Color.Blue };
            var fbb = new FlatBufferBuilder(32);
            var str1 = fbb.CreateString(monsterName);
            var vec1 = Monster.CreateVectorOfEnumsVector(fbb, colorVec);
            Monster.StartMonster(fbb);
            Monster.AddName(fbb, str1);
            Monster.AddVectorOfEnums(fbb, vec1);
            var monster1 = Monster.EndMonster(fbb);
            Monster.FinishMonsterBuffer(fbb, monster1);

            var mons = Monster.GetRootAsMonster(fbb.DataBuffer);
            var colors = mons.VectorOfEnums.Value;
            Assert.AreEqual(3, colors.Length);
            Assert.AreEqual(Color.Red, colors[0]);
            Assert.AreEqual(Color.Green, colors[1]);
            Assert.AreEqual(Color.Blue, colors[2]);

            TestObjectAPI(mons);
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
            var fbb1Bytes = fbb1.SizedReadOnlySpan().ToArray();

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

            TestObjectAPI(mons);
            TestObjectAPI(nestedMonster);
        }

        [FlatBuffersTestMethod]
        public void TestFixedLenghtArrays()
        {
            FlatBufferBuilder builder = new FlatBufferBuilder(100);

            float a;
            int[] b = new int[15];
            sbyte c;
            int[,] d_a = new int[2, 2];
            TestEnum[] d_b = new TestEnum[2];
            TestEnum[,] d_c = new TestEnum[2, 2];
            long[,] d_d = new long[2, 2];
            int e;
            long[] f = new long[2];

            a = 0.5f;
            for (int i = 0; i < 15; i++) b[i] = i;
            c = 1;
            d_a[0, 0] = 1;
            d_a[0, 1] = 2;
            d_a[1, 0] = 3;
            d_a[1, 1] = 4;
            d_b[0] = TestEnum.B;
            d_b[1] = TestEnum.C;
            d_c[0, 0] = TestEnum.A;
            d_c[0, 1] = TestEnum.B;
            d_c[1, 0] = TestEnum.C;
            d_c[1, 1] = TestEnum.B;
            d_d[0, 0] = -1;
            d_d[0, 1] = 1;
            d_d[1, 0] = -2;
            d_d[1, 1] = 2;
            e = 2;
            f[0] = -1;
            f[1] = 1;

            Offset<ArrayStruct> arrayOffset = ArrayStruct.CreateArrayStruct(
                builder, a, b, c, d_a, d_b, d_c, d_d, e, f);

            // Create a table with the ArrayStruct.
            ArrayTable.StartArrayTable(builder);
            ArrayTable.AddA(builder, arrayOffset);
            Offset<ArrayTable> tableOffset = ArrayTable.EndArrayTable(builder);

            ArrayTable.FinishArrayTableBuffer(builder, tableOffset);

            ArrayTable table = ArrayTable.GetRootAsArrayTable(builder.DataBuffer);

            Assert.AreEqual(table.A.Value.A, 0.5f);
            for (int i = 0; i < 15; i++) Assert.AreEqual(table.A.Value.B(i), i);
            Assert.AreEqual(table.A.Value.C, (sbyte)1);
            Assert.AreEqual(table.A.Value.D(0).A(0), 1);
            Assert.AreEqual(table.A.Value.D(0).A(1), 2);
            Assert.AreEqual(table.A.Value.D(1).A(0), 3);
            Assert.AreEqual(table.A.Value.D(1).A(1), 4);
            Assert.AreEqual(table.A.Value.D(0).B, TestEnum.B);
            Assert.AreEqual(table.A.Value.D(1).B, TestEnum.C);
            Assert.AreEqual(table.A.Value.D(0).C(0), TestEnum.A);
            Assert.AreEqual(table.A.Value.D(0).C(1), TestEnum.B);
            Assert.AreEqual(table.A.Value.D(1).C(0), TestEnum.C);
            Assert.AreEqual(table.A.Value.D(1).C(1), TestEnum.B);
            Assert.AreEqual(table.A.Value.D(0).D(0), -1);
            Assert.AreEqual(table.A.Value.D(0).D(1), 1);
            Assert.AreEqual(table.A.Value.D(1).D(0), -2);
            Assert.AreEqual(table.A.Value.D(1).D(1), 2);
            Assert.AreEqual(table.A.Value.E, 2);
            Assert.AreEqual(table.A.Value.F(0), -1);
            Assert.AreEqual(table.A.Value.F(1), 1);

            TestObjectAPI(table);
        }

        [FlatBuffersTestMethod]
        public void TestUnionVector()
        {
            var fbb = new FlatBufferBuilder(100);
            var rapunzel = Rapunzel.CreateRapunzel(fbb, 40).Value;

            var characterTypes = new[]
            {
                MyGame.Example.Character.MuLan,
                MyGame.Example.Character.Belle,
                MyGame.Example.Character.Other,
            };
            var characterTypesOffset = MyGame.Example.Movie.CreateCharactersTypeVector(fbb, characterTypes);

            var characterOffsets = new[]
            {
                Attacker.CreateAttacker(fbb, 10).Value,
                BookReader.CreateBookReader(fbb, 20).Value,
                fbb.CreateSharedString("Chip").Value,
            };
            var charactersOffset = MyGame.Example.Movie.CreateCharactersVector(fbb, characterOffsets);

            var movieOffset = MyGame.Example.Movie.CreateMovie(
                fbb,
                MyGame.Example.Character.Rapunzel,
                rapunzel,
                characterTypesOffset,
                charactersOffset);
            MyGame.Example.Movie.FinishMovieBuffer(fbb, movieOffset);

            var movie = MyGame.Example.Movie.GetRootAsMovie(fbb.DataBuffer);
            Assert.AreEqual(MyGame.Example.Character.Rapunzel, movie.MainCharacterType);
            Assert.AreEqual(40, movie.MainCharacter<Rapunzel>().Value.HairLength);

            var charactersType = movie.CharactersType;
            Assert.IsTrue(charactersType.HasValue);
            var charactersTypeValue = charactersType.Value;

            var charactersVector = movie.Characters;
            Assert.IsTrue(charactersVector.HasValue);
            var charactersValue = charactersVector.Value;

            Assert.AreEqual(3, charactersTypeValue.Length);
            Assert.AreEqual(3, charactersValue.Length);
            Assert.AreEqual(MyGame.Example.Character.MuLan, charactersTypeValue[0]);
            Assert.AreEqual(10, charactersValue.GetAs<Attacker>(0).SwordAttackDamage);
            Assert.AreEqual(MyGame.Example.Character.Belle, charactersTypeValue[1]);
            Assert.AreEqual(20, charactersValue.GetAs<BookReader>(1).BooksRead);
            Assert.AreEqual(MyGame.Example.Character.Other, charactersTypeValue[2]);

            var movieObject = movie.UnPack();
            Assert.AreEqual("Chip", movieObject.Characters[2].AsOther());

            TestObjectAPI(movie);
        }

        [FlatBuffersTestMethod]
        public void TestUnionUtility()
        {
            var movie = new MyGame.Example.MovieT
            {
                MainCharacter = MyGame.Example.CharacterUnion.FromRapunzel(new MyGame.Example.RapunzelT { HairLength = 40 }),
                Characters = new System.Collections.Generic.List<MyGame.Example.CharacterUnion>
                {
                    MyGame.Example.CharacterUnion.FromMuLan(new MyGame.Example.AttackerT { SwordAttackDamage = 10 }),
                    MyGame.Example.CharacterUnion.FromBelle(new MyGame.Example.BookReaderT { BooksRead = 20 }),
                    MyGame.Example.CharacterUnion.FromOther("Chip"),
                },
            };

            var fbb = new FlatBufferBuilder(100);
            MyGame.Example.Movie.FinishMovieBuffer(fbb, MyGame.Example.Movie.Pack(fbb, movie));

            TestObjectAPI(MyGame.Example.Movie.GetRootAsMovie(fbb.DataBuffer));
        }

        private void AreEqual(Monster a, MonsterT b)
        {
            Assert.AreEqual(a.Hp, b.Hp);
            Assert.AreEqual(a.Mana, b.Mana);
            Assert.AreEqual(a.Name, b.Name);

            var posA = a.Pos;
            var posB = b.Pos;
            if (posA.HasValue)
            {
                Assert.AreEqual(posA.Value.X, posB.X, 6);
                Assert.AreEqual(posA.Value.Y, posB.Y, 6);
                Assert.AreEqual(posA.Value.Z, posB.Z, 6);

                Assert.AreEqual(posA.Value.Test1, posB.Test1, 6);
                Assert.AreEqual(posA.Value.Test2, posB.Test2);
                var tA = posA.Value.Test3;
                var tB = posB.Test3;
                Assert.AreEqual(tA.A, tB.A);
                Assert.AreEqual(tA.B, tB.B);
            }

            var bTestType = b.Test?.Type ?? Any.NONE;
            Assert.AreEqual(a.TestType, bTestType);
            if (a.TestType == Any.Monster)
            {
                var monster2A = a.Test<Monster>().Value;
                var monster2B = b.Test.AsMonster();
                Assert.AreEqual(monster2A.Name, monster2B.Name);
            }

            Assert.AreEqual(a.Inventory.Value.Length, b.Inventory.Count);
            for (var i = 0; i < a.Inventory.Value.Length; ++i)
            {
                Assert.AreEqual(a.Inventory.Value[i], b.Inventory[i]);
            }

            var aTest4 = a.Test4;
            var aTest4Len = aTest4.HasValue ? aTest4.Value.Length : 0;
            Assert.AreEqual(aTest4Len, b.Test4.Count);
            if (aTest4.HasValue)
            {
                var aTest4Vec = aTest4.Value;
                for (var i = 0; i < aTest4Len; ++i)
                {
                    var t4A = aTest4Vec[i];
                    var t4B = b.Test4[i];
                    Assert.AreEqual(t4A.A, t4B.A);
                    Assert.AreEqual(t4A.B, t4B.B);
                }
            }

            var aTestarrayofstring = a.Testarrayofstring;
            var aTestarrayofstringLen = aTestarrayofstring.HasValue ? aTestarrayofstring.Value.Length : 0;
            Assert.AreEqual(aTestarrayofstringLen, b.Testarrayofstring.Count);
            if (aTestarrayofstring.HasValue)
            {
                var aTestarrayofstringVec = aTestarrayofstring.Value;
                for (var i = 0; i < aTestarrayofstringLen; ++i)
                {
                    Assert.AreEqual(aTestarrayofstringVec[i], b.Testarrayofstring[i]);
                }
            }

            Assert.AreEqual(a.Testbool, b.Testbool);

            Assert.AreEqual(a.Testarrayofbools.Value.Length, b.Testarrayofbools.Count);
            for (var i = 0; i < a.Testarrayofbools.Value.Length; ++i)
            {
                Assert.AreEqual(a.Testarrayofbools.Value[i], b.Testarrayofbools[i]);
            }

            Assert.AreEqual(a.VectorOfLongs.Value.Length, b.VectorOfLongs.Count);
            for (var i = 0; i < a.VectorOfLongs.Value.Length; ++i)
            {
                Assert.AreEqual(a.VectorOfLongs.Value[i], b.VectorOfLongs[i]);
            }

            Assert.AreEqual(a.VectorOfDoubles.Value.Length, b.VectorOfDoubles.Count);
            for (var i = 0; i < a.VectorOfDoubles.Value.Length; ++i)
            {
                Assert.AreEqual(a.VectorOfDoubles.Value[i], b.VectorOfDoubles[i]);
            }

            Assert.AreEqual(a.VectorOfEnums.Value.Length, b.VectorOfEnums.Count);
            for (var i = 0; i < a.VectorOfEnums.Value.Length; ++i)
            {
                Assert.AreEqual(a.VectorOfEnums.Value[i], b.VectorOfEnums[i]);
            }
        }

        private void AreEqual(Monster a, Monster b)
        {
            Assert.AreEqual(a.Hp, b.Hp);
            Assert.AreEqual(a.Mana, b.Mana);
            Assert.AreEqual(a.Name, b.Name);

            var posA = a.Pos;
            var posB = b.Pos;
            if (posA.HasValue)
            {
                Assert.AreEqual(posA.Value.X, posB.Value.X, 6);
                Assert.AreEqual(posA.Value.Y, posB.Value.Y, 6);
                Assert.AreEqual(posA.Value.Z, posB.Value.Z, 6);

                Assert.AreEqual(posA.Value.Test1, posB.Value.Test1, 6);
                Assert.AreEqual(posA.Value.Test2, posB.Value.Test2);
                var tA = posA.Value.Test3;
                var tB = posB.Value.Test3;
                Assert.AreEqual(tA.A, tB.A);
                Assert.AreEqual(tA.B, tB.B);
            }

            Assert.AreEqual(a.TestType, b.TestType);
            if (a.TestType == Any.Monster)
            {
                var monster2A = a.Test<Monster>().Value;
                var monster2B = b.Test<Monster>().Value;
                Assert.AreEqual(monster2A.Name, monster2B.Name);
            }

            Assert.AreEqual(a.Inventory.Value.Length, b.Inventory.Value.Length);
            for (var i = 0; i < a.Inventory.Value.Length; ++i)
            {
                Assert.AreEqual(a.Inventory.Value[i], b.Inventory.Value[i]);
            }

            var aTest4 = a.Test4;
            var bTest4 = b.Test4;
            var aTest4Len = aTest4.HasValue ? aTest4.Value.Length : 0;
            var bTest4Len = bTest4.HasValue ? bTest4.Value.Length : 0;
            Assert.AreEqual(aTest4Len, bTest4Len);
            if (aTest4.HasValue && bTest4.HasValue)
            {
                var aTest4Vec = aTest4.Value;
                var bTest4Vec = bTest4.Value;
                for (var i = 0; i < aTest4Len; ++i)
                {
                    var t4A = aTest4Vec[i];
                    var t4B = bTest4Vec[i];
                    Assert.AreEqual(t4A.A, t4B.A);
                    Assert.AreEqual(t4A.B, t4B.B);
                }
            }

            var aTestarrayofstring = a.Testarrayofstring;
            var bTestarrayofstring = b.Testarrayofstring;
            var aTestarrayofstringLen = aTestarrayofstring.HasValue ? aTestarrayofstring.Value.Length : 0;
            var bTestarrayofstringLen = bTestarrayofstring.HasValue ? bTestarrayofstring.Value.Length : 0;
            Assert.AreEqual(aTestarrayofstringLen, bTestarrayofstringLen);
            if (aTestarrayofstring.HasValue && bTestarrayofstring.HasValue)
            {
                var aTestarrayofstringVec = aTestarrayofstring.Value;
                var bTestarrayofstringVec = bTestarrayofstring.Value;
                for (var i = 0; i < aTestarrayofstringLen; ++i)
                {
                    Assert.AreEqual(aTestarrayofstringVec[i], bTestarrayofstringVec[i]);
                }
            }

            Assert.AreEqual(a.Testbool, b.Testbool);

            Assert.AreEqual(a.Testarrayofbools.Value.Length, b.Testarrayofbools.Value.Length);
            for (var i = 0; i < a.Testarrayofbools.Value.Length; ++i)
            {
                Assert.AreEqual(a.Testarrayofbools.Value[i], b.Testarrayofbools.Value[i]);
            }

            Assert.AreEqual(a.VectorOfLongs.Value.Length, b.VectorOfLongs.Value.Length);
            for (var i = 0; i < a.VectorOfLongs.Value.Length; ++i)
            {
                Assert.AreEqual(a.VectorOfLongs.Value[i], b.VectorOfLongs.Value[i]);
            }

            Assert.AreEqual(a.VectorOfDoubles.Value.Length, b.VectorOfDoubles.Value.Length);
            for (var i = 0; i < a.VectorOfDoubles.Value.Length; ++i)
            {
                Assert.AreEqual(a.VectorOfDoubles.Value[i], b.VectorOfDoubles.Value[i]);
            }

            Assert.AreEqual(a.VectorOfEnums.Value.Length, b.VectorOfEnums.Value.Length);
            for (var i = 0; i < a.VectorOfEnums.Value.Length; ++i)
            {
                Assert.AreEqual(a.VectorOfEnums.Value[i], b.VectorOfEnums.Value[i]);
            }
        }

        private void TestObjectAPI(Monster a)
        {
            var b = a.UnPack();
            AreEqual(a, b);

            var fbb = new FlatBufferBuilder(1);
            fbb.Finish(Monster.Pack(fbb, b).Value);
            var c = Monster.GetRootAsMonster(fbb.DataBuffer);
            AreEqual(a, c);

            var jsonText = b.SerializeToJson();
            var d = MonsterT.DeserializeFromJson(jsonText);
            AreEqual(a, d);

            var fbBuffer = b.SerializeToBinary();
            Assert.IsTrue(Monster.MonsterBufferHasIdentifier(new ByteBuffer(fbBuffer)));
            var e = MonsterT.DeserializeFromBinary(fbBuffer);
            AreEqual(a, e);
        }

        private void AreEqual(ArrayTable a, ArrayTableT b)
        {
            Assert.AreEqual(a.A.Value.A, b.A.A);

            for (int i = 0; i < 15; ++i)
            {
                Assert.AreEqual(a.A.Value.B(i), b.A.B[i]);
            }

            Assert.AreEqual(a.A.Value.C, b.A.C);

            for (int i = 0; i < 2; ++i)
            {
                var ad = a.A.Value.D(i);
                var bd = b.A.D[i];

                for (int j = 0; j < 2; ++j)
                {
                    Assert.AreEqual(ad.A(j), bd.A[j]);
                }

                Assert.AreEqual(ad.B, bd.B);

                for (int j = 0; j < 2; ++j)
                {
                    Assert.AreEqual(ad.C(j), bd.C[j]);
                }

                for (int j = 0; j < 2; ++j)
                {
                    Assert.AreEqual(ad.D(j), bd.D[j]);
                }
            }

            Assert.AreEqual(a.A.Value.E, b.A.E);

            for (int i = 0; i < 2; ++i)
            {
                Assert.AreEqual(a.A.Value.F(i), b.A.F[i]);
            }
        }

        private void AreEqual(ArrayTable a, ArrayTable b)
        {
            Assert.AreEqual(a.A.Value.A, b.A.Value.A);

            for (int i = 0; i < 15; ++i)
            {
                Assert.AreEqual(a.A.Value.B(i), b.A.Value.B(i));
            }

            Assert.AreEqual(a.A.Value.C, b.A.Value.C);

            for (int i = 0; i < 2; ++i)
            {
                var ad = a.A.Value.D(i);
                var bd = b.A.Value.D(i);

                for (int j = 0; j < 2; ++j)
                {
                    Assert.AreEqual(ad.A(j), bd.A(j));
                }

                Assert.AreEqual(ad.B, bd.B);

                for (int j = 0; j < 2; ++j)
                {
                    Assert.AreEqual(ad.C(j), bd.C(j));
                }

                for (int j = 0; j < 2; ++j)
                {
                    Assert.AreEqual(ad.D(j), bd.D(j));
                }
            }

            Assert.AreEqual(a.A.Value.E, b.A.Value.E);

            for (int i = 0; i < 2; ++i)
            {
                Assert.AreEqual(a.A.Value.F(i), b.A.Value.F(i));
            }
        }

        private void TestObjectAPI(ArrayTable a)
        {
            var b = a.UnPack();
            AreEqual(a, b);

            var fbb = new FlatBufferBuilder(1);
            fbb.Finish(ArrayTable.Pack(fbb, b).Value);
            var c = ArrayTable.GetRootAsArrayTable(fbb.DataBuffer);
            AreEqual(a, c);

            var jsonText = b.SerializeToJson();
            var d = ArrayTableT.DeserializeFromJson(jsonText);
            AreEqual(a, d);

            var fbBuffer = b.SerializeToBinary();
            Assert.IsTrue(ArrayTable.ArrayTableBufferHasIdentifier(new ByteBuffer(fbBuffer)));
            var e = ArrayTableT.DeserializeFromBinary(fbBuffer);
            AreEqual(a, e);
        }

        private void AreEqual(Movie a, MovieT b)
        {
            Assert.AreEqual(a.MainCharacterType, b.MainCharacter.Type);
            Assert.AreEqual(a.MainCharacter<Rapunzel>().Value.HairLength, b.MainCharacter.AsRapunzel().HairLength);

            var aCharactersType = a.CharactersType;
            var aCharacters = a.Characters;
            var aCharactersLength = aCharacters.HasValue ? aCharacters.Value.Length : 0;

            Assert.AreEqual(aCharactersLength, b.Characters.Count);
            if (aCharacters.HasValue && aCharactersType.HasValue)
            {
                var aCharactersTypeValue = aCharactersType.Value;
                var aCharactersValue = aCharacters.Value;

                Assert.AreEqual(aCharactersTypeValue[0], b.Characters[0].Type);
                Assert.AreEqual(aCharactersValue.GetAs<Attacker>(0).SwordAttackDamage, b.Characters[0].AsMuLan().SwordAttackDamage);
                Assert.AreEqual(aCharactersTypeValue[1], b.Characters[1].Type);
                Assert.AreEqual(aCharactersValue.GetAs<BookReader>(1).BooksRead, b.Characters[1].AsBelle().BooksRead);
                Assert.AreEqual(aCharactersTypeValue[2], b.Characters[2].Type);
                Assert.AreEqual(aCharactersValue.GetAsString(2), b.Characters[2].AsOther());
            }
        }

        private void AreEqual(Movie a, Movie b)
        {
            Assert.AreEqual(a.MainCharacterType, b.MainCharacterType);
            Assert.AreEqual(a.MainCharacter<Rapunzel>().Value.HairLength, b.MainCharacter<Rapunzel>().Value.HairLength);

            var aCharactersType = a.CharactersType;
            var bCharactersType = b.CharactersType;
            var aCharacters = a.Characters;
            var bCharacters = b.Characters;
            var aCharactersLength = aCharacters.HasValue ? aCharacters.Value.Length : 0;
            var bCharactersLength = bCharacters.HasValue ? bCharacters.Value.Length : 0;

            Assert.AreEqual(aCharactersLength, bCharactersLength);
            if (aCharacters.HasValue && bCharacters.HasValue && aCharactersType.HasValue && bCharactersType.HasValue)
            {
                var aCharactersTypeValue = aCharactersType.Value;
                var bCharactersTypeValue = bCharactersType.Value;
                var aCharactersValue = aCharacters.Value;
                var bCharactersValue = bCharacters.Value;

                Assert.AreEqual(aCharactersTypeValue[0], bCharactersTypeValue[0]);
                Assert.AreEqual(aCharactersValue.GetAs<Attacker>(0).SwordAttackDamage, bCharactersValue.GetAs<Attacker>(0).SwordAttackDamage);
                Assert.AreEqual(aCharactersTypeValue[1], bCharactersTypeValue[1]);
                Assert.AreEqual(aCharactersValue.GetAs<BookReader>(1).BooksRead, bCharactersValue.GetAs<BookReader>(1).BooksRead);
                Assert.AreEqual(aCharactersTypeValue[2], bCharactersTypeValue[2]);
                Assert.AreEqual(aCharactersValue.GetAsString(2), bCharactersValue.GetAsString(2));
                
                var bCharsAsString = bCharactersValue.GetAsString(2);
                int byteCount = Encoding.UTF8.GetByteCount(bCharsAsString);
                Span<byte> utf8Bytes = new byte[byteCount];
                Encoding.UTF8.GetBytes(bCharsAsString, utf8Bytes);
                Assert.SpanEqual(aCharactersValue.GetAsStringBytes(2), utf8Bytes);
            }
        }

        private void TestObjectAPI(Movie a)
        {
            var b = a.UnPack();
            AreEqual(a, b);

            var fbb = new FlatBufferBuilder(1);
            fbb.Finish(Movie.Pack(fbb, b).Value);
            var c = Movie.GetRootAsMovie(fbb.DataBuffer);
            AreEqual(a, c);

            var jsonText = b.SerializeToJson();
            var d = MovieT.DeserializeFromJson(jsonText);
            AreEqual(a, d);

            var fbBuffer = b.SerializeToBinary();
            Assert.IsTrue(Movie.MovieBufferHasIdentifier(new ByteBuffer(fbBuffer)));
            var e = MovieT.DeserializeFromBinary(fbBuffer);
            AreEqual(a, e);
        }

        // For use in TestParallelAccess test case.
        static private int _comparisons = 0;
        static private int _failures = 0;
        static private void KeepComparing(Monster mon, int count, float floatValue, double doubleValue)
        {
            int i = 0;
            while (++i <= count)
            {
                Interlocked.Add(ref _comparisons, 1);
                if (mon.Pos.Value.Test1 != doubleValue || mon.Pos.Value.Z != floatValue)
                {
                    Interlocked.Add(ref _failures, 1);
                }
            }
        }

        [FlatBuffersTestMethod]
        public void TestParallelAccess()
        {
            // Tests that reading from a flatbuffer over multiple threads is thread-safe in regard to double and float
            // values, since they previously were non-thread safe
            const float floatValue = 3.141592F;
            const double doubleValue = 1.618033988;

            var fbb = new FlatBufferBuilder(1);
            var str = fbb.CreateString("ParallelTest");
            Monster.StartMonster(fbb);
            Monster.AddPos(fbb, Vec3.CreateVec3(fbb, 1.0f, 2.0f, floatValue, doubleValue,
                                                     Color.Green, (short)5, (sbyte)6));

            Monster.AddName(fbb, str);
            Monster.FinishMonsterBuffer(fbb, Monster.EndMonster(fbb));

            var mon = Monster.GetRootAsMonster(fbb.DataBuffer);

            var pos = mon.Pos.Value;
            Assert.AreEqual(pos.Test1, doubleValue, 6);
            Assert.AreEqual(pos.Z, floatValue, 6);

            const int thread_count = 10;
            const int reps = 1000000;

            // Need to use raw Threads since Tasks are not supported in .NET 3.5
            Thread[] threads = new Thread[thread_count];
            for (int i = 0; i < thread_count; i++)
            {
                threads[i] = new Thread(() => KeepComparing(mon, reps, floatValue, doubleValue));
            }
            for (int i = 0; i < thread_count; i++)
            {
                threads[i].Start();
            }
            for (int i = 0; i < thread_count; i++)
            {
                threads[i].Join();
            }

            // Make sure the threads actually did the comparisons.
            Assert.AreEqual(thread_count * reps, _comparisons);

            // Make sure we never read the values incorrectly.
            Assert.AreEqual(0, _failures);
        }

        [FlatBuffersTestMethod]
        public void TestScalarOptional_EmptyBuffer()
        {
            var fbb = new FlatBufferBuilder(1);
            ScalarStuff.StartScalarStuff(fbb);
            var offset = ScalarStuff.EndScalarStuff(fbb);
            ScalarStuff.FinishScalarStuffBuffer(fbb, offset);

            ScalarStuff scalarStuff = ScalarStuff.GetRootAsScalarStuff(fbb.DataBuffer);
            Assert.AreEqual((sbyte)0, scalarStuff.JustI8);
            Assert.AreEqual(null, scalarStuff.MaybeI8);
            Assert.AreEqual((sbyte)42, scalarStuff.DefaultI8);
            Assert.AreEqual((byte)0, scalarStuff.JustU8);
            Assert.AreEqual(null, scalarStuff.MaybeU8);
            Assert.AreEqual((byte)42, scalarStuff.DefaultU8);

            Assert.AreEqual((short)0, scalarStuff.JustI16);
            Assert.AreEqual(null, scalarStuff.MaybeI16);
            Assert.AreEqual((short)42, scalarStuff.DefaultI16);
            Assert.AreEqual((ushort)0, scalarStuff.JustU16);
            Assert.AreEqual(null, scalarStuff.MaybeU16);
            Assert.AreEqual((ushort)42, scalarStuff.DefaultU16);

            Assert.AreEqual((int)0, scalarStuff.JustI32);
            Assert.AreEqual(null, scalarStuff.MaybeI32);
            Assert.AreEqual((int)42, scalarStuff.DefaultI32);
            Assert.AreEqual((uint)0, scalarStuff.JustU32);
            Assert.AreEqual(null, scalarStuff.MaybeU32);
            Assert.AreEqual((uint)42, scalarStuff.DefaultU32);

            Assert.AreEqual((long)0, scalarStuff.JustI64);
            Assert.AreEqual(null, scalarStuff.MaybeI64);
            Assert.AreEqual((long)42, scalarStuff.DefaultI64);
            Assert.AreEqual((ulong)0, scalarStuff.JustU64);
            Assert.AreEqual(null, scalarStuff.MaybeU64);
            Assert.AreEqual((ulong)42, scalarStuff.DefaultU64);

            Assert.AreEqual((float)0.0F, scalarStuff.JustF32);
            Assert.AreEqual(null, scalarStuff.MaybeF32);
            Assert.AreEqual((float)42.0F, scalarStuff.DefaultF32);

            Assert.AreEqual((double)0.0, scalarStuff.JustF64);
            Assert.AreEqual(null, scalarStuff.MaybeF64);
            Assert.AreEqual((double)42.0, scalarStuff.DefaultF64);

            Assert.AreEqual(false, scalarStuff.JustBool);
            Assert.AreEqual(null, scalarStuff.MaybeBool);
            Assert.AreEqual(true, scalarStuff.DefaultBool);

            Assert.AreEqual(OptionalByte.None, scalarStuff.JustEnum);
            Assert.AreEqual(null, scalarStuff.MaybeEnum);
            Assert.AreEqual(OptionalByte.One, scalarStuff.DefaultEnum);
        }

        [FlatBuffersTestMethod]
        public void TestScalarOptional_Construction()
        {
            var fbb = new FlatBufferBuilder(1);
            ScalarStuff.StartScalarStuff(fbb);
            ScalarStuff.AddJustI8(fbb, 5);
            ScalarStuff.AddMaybeI8(fbb, 5);
            ScalarStuff.AddDefaultI8(fbb, 5);
            ScalarStuff.AddJustU8(fbb, 6);
            ScalarStuff.AddMaybeU8(fbb, 6);
            ScalarStuff.AddDefaultU8(fbb, 6);

            ScalarStuff.AddJustI16(fbb, 7);
            ScalarStuff.AddMaybeI16(fbb, 7);
            ScalarStuff.AddDefaultI16(fbb, 7);
            ScalarStuff.AddJustU16(fbb, 8);
            ScalarStuff.AddMaybeU16(fbb, 8);
            ScalarStuff.AddDefaultU16(fbb, 8);

            ScalarStuff.AddJustI32(fbb, 9);
            ScalarStuff.AddMaybeI32(fbb, 9);
            ScalarStuff.AddDefaultI32(fbb, 9);
            ScalarStuff.AddJustU32(fbb, 10);
            ScalarStuff.AddMaybeU32(fbb, 10);
            ScalarStuff.AddDefaultU32(fbb, 10);

            ScalarStuff.AddJustI64(fbb, 11);
            ScalarStuff.AddMaybeI64(fbb, 11);
            ScalarStuff.AddDefaultI64(fbb, 11);
            ScalarStuff.AddJustU64(fbb, 12);
            ScalarStuff.AddMaybeU64(fbb, 12);
            ScalarStuff.AddDefaultU64(fbb, 12);

            ScalarStuff.AddJustF32(fbb, 13.0f);
            ScalarStuff.AddMaybeF32(fbb, 13.0f);
            ScalarStuff.AddDefaultF32(fbb, 13.0f);
            ScalarStuff.AddJustF64(fbb, 14.0);
            ScalarStuff.AddMaybeF64(fbb, 14.0);
            ScalarStuff.AddDefaultF64(fbb, 14.0);

            ScalarStuff.AddJustBool(fbb, true);
            ScalarStuff.AddMaybeBool(fbb, true);
            ScalarStuff.AddDefaultBool(fbb, false); // note this is the opposite

            ScalarStuff.AddJustEnum(fbb, OptionalByte.Two);
            ScalarStuff.AddMaybeEnum(fbb, OptionalByte.Two);
            ScalarStuff.AddDefaultEnum(fbb, OptionalByte.Two);

            var offset = ScalarStuff.EndScalarStuff(fbb);
            ScalarStuff.FinishScalarStuffBuffer(fbb, offset);

            ScalarStuff scalarStuff = ScalarStuff.GetRootAsScalarStuff(fbb.DataBuffer);
            Assert.AreEqual((sbyte)5, scalarStuff.JustI8);
            Assert.AreEqual((sbyte)5, scalarStuff.MaybeI8);
            Assert.AreEqual((sbyte)5, scalarStuff.DefaultI8);
            Assert.AreEqual((byte)6, scalarStuff.JustU8);
            Assert.AreEqual((byte)6, scalarStuff.MaybeU8);
            Assert.AreEqual((byte)6, scalarStuff.DefaultU8);

            Assert.AreEqual((short)7, scalarStuff.JustI16);
            Assert.AreEqual((short)7, scalarStuff.MaybeI16);
            Assert.AreEqual((short)7, scalarStuff.DefaultI16);
            Assert.AreEqual((ushort)8, scalarStuff.JustU16);
            Assert.AreEqual((ushort)8, scalarStuff.MaybeU16);
            Assert.AreEqual((ushort)8, scalarStuff.DefaultU16);

            Assert.AreEqual((int)9, scalarStuff.JustI32);
            Assert.AreEqual((int)9, scalarStuff.MaybeI32);
            Assert.AreEqual((int)9, scalarStuff.DefaultI32);
            Assert.AreEqual((uint)10, scalarStuff.JustU32);
            Assert.AreEqual((uint)10, scalarStuff.MaybeU32);
            Assert.AreEqual((uint)10, scalarStuff.DefaultU32);

            Assert.AreEqual((long)11, scalarStuff.JustI64);
            Assert.AreEqual((long)11, scalarStuff.MaybeI64);
            Assert.AreEqual((long)11, scalarStuff.DefaultI64);
            Assert.AreEqual((ulong)12, scalarStuff.JustU64);
            Assert.AreEqual((ulong)12, scalarStuff.MaybeU64);
            Assert.AreEqual((ulong)12, scalarStuff.DefaultU64);

            Assert.AreEqual((float)13.0F, scalarStuff.JustF32);
            Assert.AreEqual((float)13.0F, scalarStuff.MaybeF32);
            Assert.AreEqual((float)13.0F, scalarStuff.DefaultF32);

            Assert.AreEqual((double)14.0, scalarStuff.JustF64);
            Assert.AreEqual((double)14.0, scalarStuff.MaybeF64);
            Assert.AreEqual((double)14.0, scalarStuff.DefaultF64);

            Assert.AreEqual(true, scalarStuff.JustBool);
            Assert.AreEqual(true, scalarStuff.MaybeBool);
            Assert.AreEqual(false, scalarStuff.DefaultBool);

            Assert.AreEqual(OptionalByte.Two, scalarStuff.JustEnum);
            Assert.AreEqual(OptionalByte.Two, scalarStuff.MaybeEnum);
            Assert.AreEqual(OptionalByte.Two, scalarStuff.DefaultEnum);
        }

        [FlatBuffersTestMethod]
        public void TestScalarOptional_Construction_CreatorMethod()
        {
            var fbb = new FlatBufferBuilder(1);

            var offset = ScalarStuff.CreateScalarStuff(fbb, 5, 5, 5, 6, 6, 6, 7, 7, 7,
                8, 8, 8, 9, 9, 9, 10, 10, 10, 11, 11, 11, 12, 12, 12, 13.0f, 13.0f, 13.0f, 14.0,
                14.0, 14.0, true, true, false, OptionalByte.Two, OptionalByte.Two,
                OptionalByte.Two);
            ScalarStuff.FinishScalarStuffBuffer(fbb, offset);

            ScalarStuff scalarStuff = ScalarStuff.GetRootAsScalarStuff(fbb.DataBuffer);
            Assert.AreEqual((sbyte)5, scalarStuff.JustI8);
            Assert.AreEqual((sbyte)5, scalarStuff.MaybeI8);
            Assert.AreEqual((sbyte)5, scalarStuff.DefaultI8);
            Assert.AreEqual((byte)6, scalarStuff.JustU8);
            Assert.AreEqual((byte)6, scalarStuff.MaybeU8);
            Assert.AreEqual((byte)6, scalarStuff.DefaultU8);

            Assert.AreEqual((short)7, scalarStuff.JustI16);
            Assert.AreEqual((short)7, scalarStuff.MaybeI16);
            Assert.AreEqual((short)7, scalarStuff.DefaultI16);
            Assert.AreEqual((ushort)8, scalarStuff.JustU16);
            Assert.AreEqual((ushort)8, scalarStuff.MaybeU16);
            Assert.AreEqual((ushort)8, scalarStuff.DefaultU16);

            Assert.AreEqual((int)9, scalarStuff.JustI32);
            Assert.AreEqual((int)9, scalarStuff.MaybeI32);
            Assert.AreEqual((int)9, scalarStuff.DefaultI32);
            Assert.AreEqual((uint)10, scalarStuff.JustU32);
            Assert.AreEqual((uint)10, scalarStuff.MaybeU32);
            Assert.AreEqual((uint)10, scalarStuff.DefaultU32);

            Assert.AreEqual((long)11, scalarStuff.JustI64);
            Assert.AreEqual((long)11, scalarStuff.MaybeI64);
            Assert.AreEqual((long)11, scalarStuff.DefaultI64);
            Assert.AreEqual((ulong)12, scalarStuff.JustU64);
            Assert.AreEqual((ulong)12, scalarStuff.MaybeU64);
            Assert.AreEqual((ulong)12, scalarStuff.DefaultU64);

            Assert.AreEqual((float)13.0F, scalarStuff.JustF32);
            Assert.AreEqual((float)13.0F, scalarStuff.MaybeF32);
            Assert.AreEqual((float)13.0F, scalarStuff.DefaultF32);

            Assert.AreEqual((double)14.0, scalarStuff.JustF64);
            Assert.AreEqual((double)14.0, scalarStuff.MaybeF64);
            Assert.AreEqual((double)14.0, scalarStuff.DefaultF64);

            Assert.AreEqual(true, scalarStuff.JustBool);
            Assert.AreEqual(true, scalarStuff.MaybeBool);
            Assert.AreEqual(false, scalarStuff.DefaultBool);

            Assert.AreEqual(OptionalByte.Two, scalarStuff.JustEnum);
            Assert.AreEqual(OptionalByte.Two, scalarStuff.MaybeEnum);
            Assert.AreEqual(OptionalByte.Two, scalarStuff.DefaultEnum);
        }

        [FlatBuffersTestMethod]
        public void TestKeywordEscaping()
        {
            Assert.AreEqual((int)KeywordTest.@public.NONE, 0);

            Assert.AreEqual((int)KeywordTest.ABC.@void, 0);
            Assert.AreEqual((int)KeywordTest.ABC.where, 1);
            Assert.AreEqual((int)KeywordTest.ABC.@stackalloc, 2);

            var fbb = new FlatBufferBuilder(1);
            var offset = KeywordsInTable.CreateKeywordsInTable(
                fbb, KeywordTest.ABC.@stackalloc, KeywordTest.@public.NONE);
            fbb.Finish(offset.Value);

            KeywordsInTable keywordsInTable =
                KeywordsInTable.GetRootAsKeywordsInTable(fbb.DataBuffer);

            Assert.AreEqual(keywordsInTable.Is, KeywordTest.ABC.@stackalloc);
            Assert.AreEqual(keywordsInTable.Private, KeywordTest.@public.NONE);
        }

        [FlatBuffersTestMethod]
        public void AddOptionalEnum_WhenPassNull_ShouldWorkProperly()
        {
            var fbb = new FlatBufferBuilder(1);
            ScalarStuff.StartScalarStuff(fbb);
            ScalarStuff.AddMaybeEnum(fbb, null);
            var offset = ScalarStuff.EndScalarStuff(fbb);
            ScalarStuff.FinishScalarStuffBuffer(fbb, offset);

            ScalarStuff scalarStuff = ScalarStuff.GetRootAsScalarStuff(fbb.DataBuffer);
            Assert.AreEqual(null, scalarStuff.MaybeEnum);
        }

        [FlatBuffersTestMethod]
        public void SortKey_WithDefaultedValue_IsFindable()
        {
            // This checks if using the `key` attribute that includes the
            // default value (e.g., 0) is still searchable. This is a regression
            // test for https://github.com/google/flatbuffers/issues/7380.
            var fbb = new FlatBufferBuilder(1);

            // Create a vector of Stat objects, with Count being the key.
            var stat_offsets = new Offset<Stat>[4];
            for (ushort i = 0; i < stat_offsets.Length; i++)
            {
                Stat.StartStat(fbb);
                Stat.AddCount(fbb, i);
                stat_offsets[stat_offsets.Length - 1 - i] = Stat.EndStat(fbb);
            }

            // Ensure the sort works.
            Span<Offset<Stat>> statSpan = stat_offsets;
            var sort = Stat.CreateSortedVectorOfStat(fbb, statSpan);

            // Create the monster with the sorted vector of Stat objects.
            var str = fbb.CreateString("MyMonster");
            Monster.StartMonster(fbb);
            Monster.AddName(fbb, str);
            Monster.AddScalarKeySortedTables(fbb, sort);
            fbb.Finish(Monster.EndMonster(fbb).Value);

            // Get the monster.
            var monster = Monster.GetRootAsMonster(fbb.DataBuffer);

            // Ensure each key is findable.
            for (ushort i = 0; i < stat_offsets.Length; i++)
            {
                Assert.IsTrue(monster.TryGetScalarKeySortedTablesByKey(i, out var stat));
                Assert.AreEqual(stat.Count, i);
            }
        }

        [FlatBuffersTestMethod]
        public void CanWriteAndReadBackEquivalentCppGeneratedWireFile()
        {
            // Create a buffer matching the structure of monsterdata_test.json
            var fbb = new FlatBufferBuilder(1024);

            var monsterName = fbb.CreateString("MyMonster");
            var fredName = fbb.CreateString("Fred");
            var test1Str = fbb.CreateString("test1");
            var test2Str = fbb.CreateString("test2");

            Monster.StartMonster(fbb);
            Monster.AddName(fbb, fredName);
            var enemyOffset = Monster.EndMonster(fbb);

            var fredName2 = fbb.CreateString("Fred");
            Monster.StartMonster(fbb);
            Monster.AddName(fbb, fredName2);
            var testMonsterOffset = Monster.EndMonster(fbb);

            Span<byte> inventoryData = stackalloc byte[] { 0, 1, 2, 3, 4 };
            var inventoryVector = Monster.CreateInventoryVectorBlock(fbb, inventoryData);

            Monster.StartTest4Vector(fbb, 2);
            MyGame.Example.Test.CreateTest(fbb, (short)10, (sbyte)20);
            MyGame.Example.Test.CreateTest(fbb, (short)30, (sbyte)40);
            var test4Vector = fbb.EndVector();

            Span<StringOffset> testStrings = stackalloc StringOffset[] { test1Str, test2Str };
            var testarrayofstringVector = Monster.CreateTestarrayofstringVectorBlock(fbb, testStrings);

            Span<bool> boolsData = stackalloc bool[] { true, false, true };
            var testarrayofboolsVector = Monster.CreateTestarrayofboolsVectorBlock(fbb, boolsData);

            var longsVector = Monster.CreateVectorOfLongsVector(fbb, new long[] { 1, 100, 10000, 1000000, 100000000 });
            var doublesVector = Monster.CreateVectorOfDoublesVector(fbb, new double[] { -1.7976931348623157e+308, 0, 1.7976931348623157e+308 });

            Monster.StartMonster(fbb);
            Monster.AddPos(fbb, Vec3.CreateVec3(fbb, 1.0f, 2.0f, 3.0f, 3.0, Color.Green, (short)5, (sbyte)6));
            Monster.AddHp(fbb, 80);
            Monster.AddName(fbb, monsterName);
            Monster.AddInventory(fbb, inventoryVector);
            Monster.AddTestType(fbb, Any.Monster);
            Monster.AddTest(fbb, testMonsterOffset.Value);
            Monster.AddTest4(fbb, test4Vector);
            Monster.AddTestarrayofstring(fbb, testarrayofstringVector);
            Monster.AddTestbool(fbb, true);
            Monster.AddEnemy(fbb, enemyOffset);
            Monster.AddTestarrayofbools(fbb, testarrayofboolsVector);
            Monster.AddVectorOfLongs(fbb, longsVector);
            Monster.AddVectorOfDoubles(fbb, doublesVector);
            var monsterOffset = Monster.EndMonster(fbb);

            Monster.FinishMonsterBuffer(fbb, monsterOffset);

            // make a copy before verify. In place would access fbb.DataBuffer.Buffer.
            var bufferBytes = fbb.DataBuffer.ToSizedSpan().ToArray(); 
            var readBuffer = new ByteBuffer(bufferBytes);

            // Verify the buffer
            TestBuffer(readBuffer);
        }

        [FlatBuffersTestMethod]
        public void TestDefaultValues()
        {
            // Test that default values are correctly returned when fields are not set
            var fbb = new FlatBufferBuilder(256);

            var name = fbb.CreateString("DefaultsMonster");
            Monster.StartMonster(fbb);
            Monster.AddName(fbb, name);
            var mon = Monster.EndMonster(fbb);
            Monster.FinishMonsterBuffer(fbb, mon);

            var buffer = fbb.DataBuffer;
            var monster = Monster.GetRootAsMonster(buffer);

            // Check default values from schema
            Assert.AreEqual(150, monster.Mana); 
            Assert.AreEqual(100, monster.Hp);
            Assert.AreEqual(Color.Blue, monster.Color);
            Assert.AreEqual(3.14159f, monster.Testf, 6);
            Assert.AreEqual(3.0f, monster.Testf2, 6);
            Assert.AreEqual(0.0f, monster.Testf3, 6);
            Assert.AreEqual(Race.None, monster.SignedEnum);
        }

        [FlatBuffersTestMethod]
        public void TestStringAsSpan()
        {
            var fbb = new FlatBufferBuilder(256);

            var testName = "SpanTestMonster";
            var name = fbb.CreateString(testName);
            Monster.StartMonster(fbb);
            Monster.AddName(fbb, name);
            var mon = Monster.EndMonster(fbb);
            Monster.FinishMonsterBuffer(fbb, mon);

            var buffer = fbb.DataBuffer;
            var monster = Monster.GetRootAsMonster(buffer);
            var nameBytes = monster.GetNameBytes();
            var expectedBytes = Encoding.UTF8.GetBytes(testName);

            Assert.AreEqual(expectedBytes.Length, nameBytes.Length);
            Assert.SpanEqual<byte>(expectedBytes, nameBytes);
        }

        [FlatBuffersTestMethod]
        public void TestInventoryAsSpan()
        {
            var fbb = new FlatBufferBuilder(256);

            Span<byte> inventoryData = stackalloc byte[] { 10, 20, 30, 40, 50 };
            var inv = Monster.CreateInventoryVectorBlock(fbb, inventoryData);

            var name = fbb.CreateString("InventoryMonster");
            Monster.StartMonster(fbb);
            Monster.AddName(fbb, name);
            Monster.AddInventory(fbb, inv);
            var mon = Monster.EndMonster(fbb);
            Monster.FinishMonsterBuffer(fbb, mon);

            var buffer = fbb.DataBuffer;
            var monster = Monster.GetRootAsMonster(buffer);
            var inventoryBytes = monster.Inventory.Value;
            var expectedInventory = new byte[] { 10, 20, 30, 40, 50 };

            Assert.AreEqual(expectedInventory.Length, inventoryBytes.Length);
            Assert.SpanEqual<byte>(expectedInventory, inventoryBytes);
        }

        [FlatBuffersTestMethod]
        public void TestUnion()
        {
            var fbb = new FlatBufferBuilder(256);

            var fred = fbb.CreateString("Fred");
            Monster.StartMonster(fbb);
            Monster.AddName(fbb, fred);
            Monster.AddHp(fbb, 200);
            var unionMonster = Monster.EndMonster(fbb);

            var name = fbb.CreateString("UnionMonster");
            Monster.StartMonster(fbb);
            Monster.AddName(fbb, name);
            Monster.AddTestType(fbb, Any.Monster);
            Monster.AddTest(fbb, unionMonster.Value);
            var mon = Monster.EndMonster(fbb);
            Monster.FinishMonsterBuffer(fbb, mon);

            var buffer = fbb.DataBuffer;
            var monster = Monster.GetRootAsMonster(buffer);

            Assert.AreEqual(Any.Monster, monster.TestType);

            var testMonster = monster.Test<Monster>().Value;
            Assert.AreEqual("Fred", testMonster.Name);
            Assert.AreEqual(200, testMonster.Hp);
        }

        [FlatBuffersTestMethod]
        public void TestVectorOfStructs()
        {
            var fbb = new FlatBufferBuilder(256);

            Monster.StartTest4Vector(fbb, 3);
            Test.CreateTest(fbb, 30, 40);
            Test.CreateTest(fbb, 20, 30);
            Test.CreateTest(fbb, 10, 20);
            var test4Vector = fbb.EndVector();

            var name = fbb.CreateString("Test4Monster");
            Monster.StartMonster(fbb);
            Monster.AddName(fbb, name);
            Monster.AddTest4(fbb, test4Vector);
            var mon = Monster.EndMonster(fbb);
            Monster.FinishMonsterBuffer(fbb, mon);

            var buffer = fbb.DataBuffer;
            var monster = Monster.GetRootAsMonster(buffer);

            var test4 = monster.Test4;
            Assert.IsTrue(test4.HasValue);
            var test4Vec = test4.Value;
            Assert.AreEqual(3, test4Vec.Length);

            var test0 = test4Vec[0];
            Assert.AreEqual((short)10, test0.A);
            Assert.AreEqual((sbyte)20, test0.B);

            var test1 = test4Vec[1];
            Assert.AreEqual((short)20, test1.A);
            Assert.AreEqual((sbyte)30, test1.B);

            var test2 = test4Vec[2];
            Assert.AreEqual((short)30, test2.A);
            Assert.AreEqual((sbyte)40, test2.B);
        }

        [FlatBuffersTestMethod]
        public void TestEmptyBuffer()
        {
            var fbb = new FlatBufferBuilder(256);

            // Create a minimal monster with just a name
            var name = fbb.CreateString("EmptyMonster");
            Monster.StartMonster(fbb);
            Monster.AddName(fbb, name);
            var mon = Monster.EndMonster(fbb);
            Monster.FinishMonsterBuffer(fbb, mon);

            var buffer = fbb.DataBuffer;
            var monster = Monster.GetRootAsMonster(buffer);

            // Test that empty vectors return null/false HasValue
            Assert.IsFalse(monster.Inventory.HasValue);
            Assert.IsFalse(monster.Test4.HasValue);
            Assert.IsFalse(monster.Testarrayofstring.HasValue);

            // Test that optional struct returns null
            Assert.IsFalse(monster.Pos.HasValue);
        }

        [FlatBuffersTestMethod]
        public void TestBuilderReuse()
        {
            var fbb = new FlatBufferBuilder(256);

            var name1 = fbb.CreateString("Monster1");
            Monster.StartMonster(fbb);
            Monster.AddName(fbb, name1);
            Monster.AddHp(fbb, 100);
            var mon1 = Monster.EndMonster(fbb);
            Monster.FinishMonsterBuffer(fbb, mon1);

            var buffer1 = fbb.DataBuffer.ToSizedSpan().ToArray();

            fbb.Clear();

            var name2 = fbb.CreateString("Monster2");
            Monster.StartMonster(fbb);
            Monster.AddName(fbb, name2);
            Monster.AddHp(fbb, 200);
            var mon2 = Monster.EndMonster(fbb);
            Monster.FinishMonsterBuffer(fbb, mon2);

            var bb1 = new ByteBuffer(buffer1);
            var monster1 = Monster.GetRootAsMonster(bb1);
            Assert.AreEqual("Monster1", monster1.Name);
            Assert.AreEqual(100, monster1.Hp);

            var monster2 = Monster.GetRootAsMonster(fbb.DataBuffer);
            Assert.AreEqual("Monster2", monster2.Name);
            Assert.AreEqual(200, monster2.Hp);
        }

        [FlatBuffersTestMethod]
        public void TestVectorOfLongs()
        {
            var fbb = new FlatBufferBuilder(256);

            Span<long> longsData = stackalloc long[] { 1L, 2L, 3L, long.MaxValue, long.MinValue };
            var longsVec = Monster.CreateVectorOfLongsVector(fbb, longsData);

            var name = fbb.CreateString("LongsMonster");
            Monster.StartMonster(fbb);
            Monster.AddName(fbb, name);
            Monster.AddVectorOfLongs(fbb, longsVec);
            var mon = Monster.EndMonster(fbb);
            Monster.FinishMonsterBuffer(fbb, mon);

            var monster = Monster.GetRootAsMonster(fbb.DataBuffer);

            Assert.AreEqual(5, monster.VectorOfLongs.Value.Length);
            Assert.AreEqual(1L, monster.VectorOfLongs.Value[0]);
            Assert.AreEqual(2L, monster.VectorOfLongs.Value[1]);
            Assert.AreEqual(3L, monster.VectorOfLongs.Value[2]);
            Assert.AreEqual(long.MaxValue, monster.VectorOfLongs.Value[3]);
            Assert.AreEqual(long.MinValue, monster.VectorOfLongs.Value[4]);

            var longsBytes = monster.VectorOfLongs.Value;
            Assert.AreEqual(5, longsBytes.Length);
        }

        [FlatBuffersTestMethod]
        public void TestVectorOfDoubles()
        {
            var fbb = new FlatBufferBuilder(256);

            Span<double> doublesData = stackalloc double[] { 1.0, 2.5, 3.14159, double.MaxValue, double.MinValue };
            var doublesVec = Monster.CreateVectorOfDoublesVector(fbb, doublesData);

            var name = fbb.CreateString("DoublesMonster");
            Monster.StartMonster(fbb);
            Monster.AddName(fbb, name);
            Monster.AddVectorOfDoubles(fbb, doublesVec);
            var mon = Monster.EndMonster(fbb);
            Monster.FinishMonsterBuffer(fbb, mon);

            var monster = Monster.GetRootAsMonster(fbb.DataBuffer);

            Assert.AreEqual(5, monster.VectorOfDoubles.Value.Length);
            Assert.AreEqual(1.0, monster.VectorOfDoubles.Value[0], 6);
            Assert.AreEqual(2.5, monster.VectorOfDoubles.Value[1], 6);
            Assert.AreEqual(3.14159, monster.VectorOfDoubles.Value[2], 6);
            Assert.AreEqual(double.MaxValue, monster.VectorOfDoubles.Value[3], 6);
            Assert.AreEqual(double.MinValue, monster.VectorOfDoubles.Value[4], 6);

            var doublesBytes = monster.VectorOfDoubles.Value;
            Assert.AreEqual(5, doublesBytes.Length);
        }

        [FlatBuffersTestMethod]
        public void TestArrayOfBools()
        {
            var fbb = new FlatBufferBuilder(256);

            Span<bool> boolsData = stackalloc bool[] { true, false, true, false, true };
            var boolsVec = Monster.CreateTestarrayofboolsVector(fbb, boolsData);

            var name = fbb.CreateString("BoolsMonster");
            Monster.StartMonster(fbb);
            Monster.AddName(fbb, name);
            Monster.AddTestarrayofbools(fbb, boolsVec);
            var mon = Monster.EndMonster(fbb);
            Monster.FinishMonsterBuffer(fbb, mon);

            var monster = Monster.GetRootAsMonster(fbb.DataBuffer);

            Assert.AreEqual(5, monster.Testarrayofbools.Value.Length);
            Assert.AreEqual(true, monster.Testarrayofbools.Value[0]);
            Assert.AreEqual(false, monster.Testarrayofbools.Value[1]);
            Assert.AreEqual(true, monster.Testarrayofbools.Value[2]);
            Assert.AreEqual(false, monster.Testarrayofbools.Value[3]);
            Assert.AreEqual(true, monster.Testarrayofbools.Value[4]);

            TestObjectAPI(monster);
        }

        [FlatBuffersTestMethod]
        public void TestEnemy()
        {
            var fbb = new FlatBufferBuilder(256);

            var enemyName = fbb.CreateString("EnemyMonster");
            Monster.StartMonster(fbb);
            Monster.AddName(fbb, enemyName);
            Monster.AddHp(fbb, 50);
            var enemy = Monster.EndMonster(fbb);

            var name = fbb.CreateString("MainMonster");
            Monster.StartMonster(fbb);
            Monster.AddName(fbb, name);
            Monster.AddHp(fbb, 100);
            Monster.AddEnemy(fbb, enemy);
            var mon = Monster.EndMonster(fbb);
            Monster.FinishMonsterBuffer(fbb, mon);

            var monster = Monster.GetRootAsMonster(fbb.DataBuffer);

            Assert.AreEqual("MainMonster", monster.Name);
            Assert.AreEqual(100, monster.Hp);
            Assert.IsTrue(monster.Enemy.HasValue);
            Assert.AreEqual("EnemyMonster", monster.Enemy.Value.Name);
            Assert.AreEqual(50, monster.Enemy.Value.Hp);
        }

        [FlatBuffersTestMethod]
        public void TestNanAndInfinityDefaults()
        {
            var fbb = new FlatBufferBuilder(256);

            var name = fbb.CreateString("NanInfMonster");
            Monster.StartMonster(fbb);
            Monster.AddName(fbb, name);
            var mon = Monster.EndMonster(fbb);
            Monster.FinishMonsterBuffer(fbb, mon);

            var monster = Monster.GetRootAsMonster(fbb.DataBuffer);

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
        public void TestObjectAPI_Pack()
        {
            var monsterT = new MonsterT
            {
                Name = "PackedMonster",
                Hp = 300,
                Mana = 200,
                Color = Color.Red,
                Inventory = new System.Collections.Generic.List<byte> { 1, 2, 3, 4, 5 },
                Pos = new Vec3T
                {
                    X = 1.0f,
                    Y = 2.0f,
                    Z = 3.0f,
                    Test1 = 4.0,
                    Test2 = Color.Green,
                    Test3 = new TestT { A = 10, B = 20 }
                },
                Test4 = new System.Collections.Generic.List<TestT>
                {
                    new TestT { A = 100, B = 50 },
                    new TestT { A = 200, B = 60 }
                }
            };

            var fbb = new FlatBufferBuilder(1024);
            var offset = Monster.Pack(fbb, monsterT);
            Monster.FinishMonsterBuffer(fbb, offset);

            var buffer = fbb.DataBuffer;
            var monster = Monster.GetRootAsMonster(buffer);

            Assert.AreEqual(monsterT.Name, monster.Name);
            Assert.AreEqual(monsterT.Hp, monster.Hp);
            Assert.AreEqual(monsterT.Mana, monster.Mana);
            Assert.AreEqual(monsterT.Color, monster.Color);

            Assert.AreEqual(monsterT.Inventory.Count, monster.Inventory.Value.Length);
            for (var i = 0; i < monsterT.Inventory.Count; ++i)
            {
                Assert.AreEqual(monsterT.Inventory[i], monster.Inventory.Value[i]);
            }

            var pos = monster.Pos.Value;
            Assert.AreEqual(monsterT.Pos.X, pos.X, 6);
            Assert.AreEqual(monsterT.Pos.Y, pos.Y, 6);
            Assert.AreEqual(monsterT.Pos.Z, pos.Z, 6);
            Assert.AreEqual(monsterT.Pos.Test1, pos.Test1, 6);
            Assert.AreEqual(monsterT.Pos.Test2, pos.Test2);
            Assert.AreEqual(monsterT.Pos.Test3.A, pos.Test3.A);
            Assert.AreEqual(monsterT.Pos.Test3.B, pos.Test3.B);

            var test4 = monster.Test4;
            Assert.IsTrue(test4.HasValue);
            var test4Vec = test4.Value;
            Assert.AreEqual(monsterT.Test4.Count, test4Vec.Length);
            for (var i = 0; i < monsterT.Test4.Count; ++i)
            {
                Assert.AreEqual(monsterT.Test4[i].A, test4Vec[i].A);
                Assert.AreEqual(monsterT.Test4[i].B, test4Vec[i].B);
            }
        }
    }
}
