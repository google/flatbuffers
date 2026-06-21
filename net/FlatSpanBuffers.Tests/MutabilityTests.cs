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
using MonsterTest;

namespace Google.FlatSpanBuffers.Tests
{
    [FlatBuffersTestClass]
    public class MutabilityTests
    {
        [FlatBuffersTestMethod]
        public void ByteBuffer_MutateScalarField_Success()
        {
            var builder = new FlatBufferBuilder(1024);
            var nameOffset = builder.CreateString("TestMonster");

            Monster.StartMonster(builder);
            Monster.AddName(builder, nameOffset);
            Monster.AddHp(builder, 80);     // Non-default (default is 100)
            Monster.AddMana(builder, 120);  // Non-default (default is 150)
            var monsterOffset = Monster.EndMonster(builder);
            builder.Finish(monsterOffset.Value);

            var buffer = builder.DataBuffer;
            var monster = Monster.GetRootAsMonster(buffer);

            Assert.AreEqual(80, monster.Hp);
            Assert.AreEqual(120, monster.Mana);
            Assert.IsTrue(monster.MutateHp(150));
            Assert.IsTrue(monster.MutateMana(250));
            Assert.AreEqual(150, monster.Hp);
            Assert.AreEqual(250, monster.Mana);

            var monster2 = Monster.GetRootAsMonster(buffer);
            Assert.AreEqual(150, monster2.Hp);
            Assert.AreEqual(250, monster2.Mana);
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_MutateEnumField_Success()
        {
            var builder = new FlatBufferBuilder(1024);
            var nameOffset = builder.CreateString("TestMonster");

            Monster.StartMonster(builder);
            Monster.AddName(builder, nameOffset);
            Monster.AddColor(builder, Color.Red);
            var monsterOffset = Monster.EndMonster(builder);
            builder.Finish(monsterOffset.Value);

            var buffer = builder.DataBuffer;
            var monster = Monster.GetRootAsMonster(buffer);

            Assert.AreEqual(Color.Red, monster.Color);
            Assert.IsTrue(monster.MutateColor(Color.Green));
            Assert.AreEqual(Color.Green, monster.Color);
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_MutateVectorElement_Success()
        {
            var builder = new FlatBufferBuilder(1024);
            var nameOffset = builder.CreateString("TestMonster");

            Span<byte> inventoryData = stackalloc byte[] { 10, 20, 30, 40, 50 };
            var inventoryVector = Monster.CreateInventoryVectorBlock(builder, inventoryData);

            Monster.StartMonster(builder);
            Monster.AddName(builder, nameOffset);
            Monster.AddInventory(builder, inventoryVector);
            var monsterOffset = Monster.EndMonster(builder);
            builder.Finish(monsterOffset.Value);

            var buffer = builder.DataBuffer;
            var monster = Monster.GetRootAsMonster(buffer);

            var inventory = monster.Inventory;
            Assert.IsTrue(inventory.HasValue);
            Assert.AreEqual(inventoryData.Length, inventory.Value.Length);
            Assert.AreEqual(10, inventory.Value[0]);
            Assert.AreEqual(30, inventory.Value[2]);

            var inventorySpan = monster.MutableInventory;
            Assert.IsTrue(inventorySpan.HasValue);
            inventorySpan.Value[0] = 100;
            inventorySpan.Value[2] = 200;

            var inventory2 = monster.Inventory;
            Assert.AreEqual(100, inventory2.Value[0]);
            Assert.AreEqual(200, inventory2.Value[2]);
            Assert.AreEqual(20, inventory2.Value[1]); // unchanged
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_MutateWithSpan_Success()
        {
            var builder = new FlatBufferBuilder(1024);
            var nameOffset = builder.CreateString("TestMonster");

            Span<byte> inventoryData = stackalloc byte[] { 1, 2, 3, 4, 5 };
            var inventoryVector = Monster.CreateInventoryVectorBlock(builder, inventoryData);

            Monster.StartMonster(builder);
            Monster.AddName(builder, nameOffset);
            Monster.AddInventory(builder, inventoryVector);
            var monsterOffset = Monster.EndMonster(builder);
            builder.Finish(monsterOffset.Value);

            var buffer = builder.DataBuffer;
            var monster = Monster.GetRootAsMonster(buffer);

            var inventorySpan = monster.MutableInventory;
            Assert.IsTrue(inventorySpan.HasValue);
            Assert.AreEqual(inventoryData.Length, inventorySpan.Value.Length);

            inventorySpan.Value[0] = 100;
            inventorySpan.Value[4] = 200;

            // Verify changes through the original accessor
            var inventory = monster.Inventory;
            Assert.AreEqual(100, inventory.Value[0]);
            Assert.AreEqual(200, inventory.Value[4]);
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_MutateNonExistentField_ReturnsFalse()
        {
            var builder = new FlatBufferBuilder(1024);
            var nameOffset = builder.CreateString("TestMonster");

            // Create monster without setting Hp (will use default value 100)
            Monster.StartMonster(builder);
            Monster.AddName(builder, nameOffset);
            var monsterOffset = Monster.EndMonster(builder);
            builder.Finish(monsterOffset.Value);

            var buffer = builder.DataBuffer;
            var monster = Monster.GetRootAsMonster(buffer);
            Assert.AreEqual(100, monster.Hp); // default value

            // MutateHp returns false when the field is not present in the buffer
            Assert.IsFalse(monster.MutateHp(150));

            // Double check Hp still equals default
            Assert.AreEqual(100, monster.Hp);
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_MutateWithForceDefaults_Success()
        {
            var builder = new FlatBufferBuilder(1024);
            builder.ForceDefaults = true;

            var nameOffset = builder.CreateString("TestMonster");

            Monster.StartMonster(builder);
            Monster.AddName(builder, nameOffset);
            Monster.AddHp(builder, 100);  // Even default value will be written due to ForceDefaults
            var monsterOffset = Monster.EndMonster(builder);
            builder.Finish(monsterOffset.Value);

            var buffer = builder.DataBuffer;
            var monster = Monster.GetRootAsMonster(buffer);

            Assert.AreEqual(100, monster.Hp);
            Assert.IsTrue(monster.MutateHp(150));
            Assert.AreEqual(150, monster.Hp);
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_MutateScalarField_Success()
        {
            var builder = new FlatBufferBuilder(1024);
            var nameOffset = builder.CreateString("TestMonster");

            Monster.StartMonster(builder);
            Monster.AddName(builder, nameOffset);
            Monster.AddHp(builder, 80);     // Non-default (default is 100)
            Monster.AddMana(builder, 120);  // Non-default (default is 150)
            var monsterOffset = Monster.EndMonster(builder);
            builder.Finish(monsterOffset.Value);

            Span<byte> data = builder.DataBuffer.ToSizedSpan();
            var spanBuffer = new ByteSpanBuffer(data);

            var monster = MonsterTest.StackBuffer.Monster.GetRootAsMonster(spanBuffer);

            Assert.AreEqual(80, monster.Hp);
            Assert.AreEqual(120, monster.Mana);
            Assert.IsTrue(monster.MutateHp(150));
            Assert.IsTrue(monster.MutateMana(250));
            Assert.AreEqual(150, monster.Hp);
            Assert.AreEqual(250, monster.Mana);
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_MutateEnumField_Success()
        {
            var builder = new FlatBufferBuilder(1024);
            var nameOffset = builder.CreateString("TestMonster");

            Monster.StartMonster(builder);
            Monster.AddName(builder, nameOffset);
            Monster.AddColor(builder, Color.Red);
            var monsterOffset = Monster.EndMonster(builder);
            builder.Finish(monsterOffset.Value);

            Span<byte> data = builder.DataBuffer.ToSizedSpan();
            var spanBuffer = new ByteSpanBuffer(data);

            var monster = MonsterTest.StackBuffer.Monster.GetRootAsMonster(spanBuffer);

            Assert.AreEqual(Color.Red, monster.Color);
            Assert.IsTrue(monster.MutateColor(Color.Green));
            Assert.AreEqual(Color.Green, monster.Color);
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_MutateVectorElement_Success()
        {
            var builder = new FlatBufferBuilder(1024);
            var nameOffset = builder.CreateString("TestMonster");

            Span<byte> inventoryData = stackalloc byte[] { 10, 20, 30, 40, 50 };
            var inventoryVector = Monster.CreateInventoryVectorBlock(builder, inventoryData);

            Monster.StartMonster(builder);
            Monster.AddName(builder, nameOffset);
            Monster.AddInventory(builder, inventoryVector);
            var monsterOffset = Monster.EndMonster(builder);
            builder.Finish(monsterOffset.Value);

            Span<byte> data = builder.DataBuffer.ToSizedSpan();
            var spanBuffer = new ByteSpanBuffer(data);

            var monster = MonsterTest.StackBuffer.Monster.GetRootAsMonster(spanBuffer);

            var inventory = monster.Inventory;
            Assert.IsTrue(inventory.HasValue);
            Assert.AreEqual(5, inventory.Value.Length);
            Assert.AreEqual(10, inventory.Value[0]);
            Assert.AreEqual(30, inventory.Value[2]);

            var inventorySpan = monster.MutableInventory;
            Assert.IsTrue(inventorySpan.HasValue);
            inventorySpan.Value[0] = 100;
            inventorySpan.Value[2] = 200;

            // Verify mutated values
            var monster2 = MonsterTest.StackBuffer.Monster.GetRootAsMonster(spanBuffer);
            var inventory2 = monster2.Inventory;
            Assert.AreEqual(100, inventory2.Value[0]);
            Assert.AreEqual(200, inventory2.Value[2]);
            Assert.AreEqual(20, inventory2.Value[1]);
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_MutateWithSpan_Success()
        {
            var builder = new FlatBufferBuilder(1024);
            var nameOffset = builder.CreateString("TestMonster");

            Span<byte> inventoryData = stackalloc byte[] { 1, 2, 3, 4, 5 };
            var inventoryVector = Monster.CreateInventoryVectorBlock(builder, inventoryData);

            Monster.StartMonster(builder);
            Monster.AddName(builder, nameOffset);
            Monster.AddInventory(builder, inventoryVector);
            var monsterOffset = Monster.EndMonster(builder);
            builder.Finish(monsterOffset.Value);

            Span<byte> data = builder.DataBuffer.ToSizedSpan();
            var spanBuffer = new ByteSpanBuffer(data);

            var monster = MonsterTest.StackBuffer.Monster.GetRootAsMonster(spanBuffer);

            var inventorySpan = monster.MutableInventory;
            Assert.IsTrue(inventorySpan.HasValue);
            Assert.AreEqual(5, inventorySpan.Value.Length);
            inventorySpan.Value[0] = 100;
            inventorySpan.Value[4] = 200;

            // Verify mutated values
            var monster2 = MonsterTest.StackBuffer.Monster.GetRootAsMonster(spanBuffer);
            var inventory = monster2.Inventory;
            Assert.AreEqual(100, inventory.Value[0]);
            Assert.AreEqual(200, inventory.Value[4]);
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_MutateOnStackAllocatedBuffer_Success()
        {
            var builder = new FlatBufferBuilder(1024);
            var nameOffset = builder.CreateString("TestMonster");

            Monster.StartMonster(builder);
            Monster.AddName(builder, nameOffset);
            Monster.AddHp(builder, 50);
            var monsterOffset = Monster.EndMonster(builder);
            builder.Finish(monsterOffset.Value);

            var serializedData = builder.DataBuffer.ToSizedSpan();
            Span<byte> stackBuffer = stackalloc byte[serializedData.Length];
            serializedData.CopyTo(stackBuffer);

            var spanBuffer = new ByteSpanBuffer(stackBuffer);
            var monster = MonsterTest.StackBuffer.Monster.GetRootAsMonster(spanBuffer);

            Assert.AreEqual(50, monster.Hp);
            Assert.IsTrue(monster.MutateHp(99));
            Assert.AreEqual(99, monster.Hp);

            // Verify the stack buffer was modified
            var monster2 = MonsterTest.StackBuffer.Monster.GetRootAsMonster(spanBuffer);
            Assert.AreEqual(99, monster2.Hp);
        }

        [FlatBuffersTestMethod]
        public void CrossVariant_MutationsProduceSameResults()
        {
            var builder = new FlatBufferBuilder(1024);
            var nameOffset = builder.CreateString("TestMonster");

            Span<byte> inventoryData = stackalloc byte[] { 1, 2, 3, 4, 5 };
            var inventoryVector = Monster.CreateInventoryVectorBlock(builder, inventoryData);

            Monster.StartMonster(builder);
            Monster.AddName(builder, nameOffset);
            Monster.AddHp(builder, 80);      // Non-default (default is 100)
            Monster.AddMana(builder, 120);   // Non-default (default is 150)
            Monster.AddColor(builder, Color.Red);  // Non-default (default is Blue)
            Monster.AddInventory(builder, inventoryVector);
            var monsterOffset = Monster.EndMonster(builder);
            builder.Finish(monsterOffset.Value);

            var serializedData = builder.DataBuffer.ToSizedSpan();
            var copy1 = new byte[serializedData.Length];
            var copy2 = new byte[serializedData.Length];
            serializedData.CopyTo(copy1.AsSpan());
            serializedData.CopyTo(copy2.AsSpan());

            // Mutate using ByteBuffer
            var byteBuffer = new ByteBuffer(copy1);
            var monster1 = Monster.GetRootAsMonster(byteBuffer);
            monster1.MutateHp(150);
            monster1.MutateMana(250);
            monster1.MutateColor(Color.Green);
            var invSpan1 = monster1.MutableInventory;
            Assert.IsTrue(invSpan1.HasValue);
            invSpan1.Value[0] = 10;
            invSpan1.Value[2] = 30;

            // Mutate using StackBuffer
            var spanBuffer = new ByteSpanBuffer(copy2);
            var monster2 = MonsterTest.StackBuffer.Monster.GetRootAsMonster(spanBuffer);
            monster2.MutateHp(150);
            monster2.MutateMana(250);
            monster2.MutateColor(Color.Green);
            var invSpan2 = monster2.MutableInventory;
            Assert.IsTrue(invSpan2.HasValue);
            invSpan2.Value[0] = 10;
            invSpan2.Value[2] = 30;

            Assert.SpanEqual(copy1, copy2);
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_MutateStructVectorElement_Success()
        {
            var builder = new FlatBufferBuilder(1024);
            var nameOffset = builder.CreateString("TestMonster");

            Monster.StartPathVector(builder, 3);
            Vec3.CreateVec3(builder, 3.0f, 3.0f, 3.0f);
            Vec3.CreateVec3(builder, 2.0f, 2.0f, 2.0f);
            Vec3.CreateVec3(builder, 1.0f, 1.0f, 1.0f);
            var pathVector = builder.EndVector();

            Monster.StartMonster(builder);
            Monster.AddName(builder, nameOffset);
            Monster.AddPath(builder, pathVector);
            var monsterOffset = Monster.EndMonster(builder);
            builder.Finish(monsterOffset.Value);

            var buffer = builder.DataBuffer;
            var monster = Monster.GetRootAsMonster(buffer);

            var path = monster.Path;
            Assert.IsTrue(path.HasValue);
            Assert.AreEqual(3, path.Value.Length);

            var vec0 = path.Value[0];
            Assert.AreEqual(1.0f, vec0.X, 3);

            var vec1 = path.Value[1];
            Assert.AreEqual(2.0f, vec1.X, 3);

            vec0.MutateX(10.0f);
            vec0.MutateY(20.0f);
            vec0.MutateZ(30.0f);

            var monster2 = Monster.GetRootAsMonster(buffer);
            var path2 = monster2.Path;
            var vec0_2 = path2.Value[0];

            Assert.AreEqual(10.0f, vec0_2.X, 3);
            Assert.AreEqual(20.0f, vec0_2.Y, 3);
            Assert.AreEqual(30.0f, vec0_2.Z, 3);

            var vec1_2 = path2.Value[1];
            Assert.AreEqual(2.0f, vec1_2.X, 3);
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_MutateStructVectorElement_Success()
        {
            var builder = new FlatBufferBuilder(1024);
            var nameOffset = builder.CreateString("TestMonster");

            Monster.StartPathVector(builder, 3);
            Vec3.CreateVec3(builder, 3.0f, 3.0f, 3.0f);
            Vec3.CreateVec3(builder, 2.0f, 2.0f, 2.0f);
            Vec3.CreateVec3(builder, 1.0f, 1.0f, 1.0f);
            var pathVector = builder.EndVector();

            Monster.StartMonster(builder);
            Monster.AddName(builder, nameOffset);
            Monster.AddPath(builder, pathVector);
            var monsterOffset = Monster.EndMonster(builder);
            builder.Finish(monsterOffset.Value);

            Span<byte> data = builder.DataBuffer.ToSizedSpan();
            var spanBuffer = new ByteSpanBuffer(data);
            var monster = MonsterTest.StackBuffer.Monster.GetRootAsMonster(spanBuffer);

            var path = monster.Path;
            Assert.IsTrue(path.HasValue);
            Assert.AreEqual(3, path.Value.Length);

            var vec0 = path.Value[0];
            Assert.AreEqual(1.0f, vec0.X, 3);

            vec0.MutateX(10.0f);
            vec0.MutateY(20.0f);
            vec0.MutateZ(30.0f);

            var monster2 = MonsterTest.StackBuffer.Monster.GetRootAsMonster(spanBuffer);
            var path2 = monster2.Path;
            var vec0_2 = path2.Value[0];

            Assert.AreEqual(10.0f, vec0_2.X, 3);
            Assert.AreEqual(20.0f, vec0_2.Y, 3);
            Assert.AreEqual(30.0f, vec0_2.Z, 3);

            var vec1_2 = path2.Value[1];
            Assert.AreEqual(2.0f, vec1_2.X, 3);
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_MutateInlineStruct_Success()
        {
            var builder = new FlatBufferBuilder(1024);
            var nameOffset = builder.CreateString("TestMonster");

            Monster.StartMonster(builder);
            Monster.AddName(builder, nameOffset);
            Monster.AddPos(builder, Vec3.CreateVec3(builder, 1.0f, 2.0f, 3.0f));
            var monsterOffset = Monster.EndMonster(builder);
            builder.Finish(monsterOffset.Value);

            var buffer = builder.DataBuffer;
            var monster = Monster.GetRootAsMonster(buffer);

            var pos = monster.Pos;
            Assert.IsTrue(pos.HasValue);
            Assert.AreEqual(1.0f, pos.Value.X, 3);
            Assert.AreEqual(2.0f, pos.Value.Y, 3);
            Assert.AreEqual(3.0f, pos.Value.Z, 3);

            pos.Value.MutateX(100.0f);
            pos.Value.MutateY(200.0f);
            pos.Value.MutateZ(300.0f);

            var monster2 = Monster.GetRootAsMonster(buffer);
            var pos2 = monster2.Pos;
            Assert.AreEqual(100.0f, pos2.Value.X, 3);
            Assert.AreEqual(200.0f, pos2.Value.Y, 3);
            Assert.AreEqual(300.0f, pos2.Value.Z, 3);
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_MutateInlineStruct_Success()
        {
            var builder = new FlatBufferBuilder(1024);
            var nameOffset = builder.CreateString("TestMonster");

            Monster.StartMonster(builder);
            Monster.AddName(builder, nameOffset);
            Monster.AddPos(builder, Vec3.CreateVec3(builder, 1.0f, 2.0f, 3.0f));
            var monsterOffset = Monster.EndMonster(builder);
            builder.Finish(monsterOffset.Value);

            Span<byte> data = builder.DataBuffer.ToSizedSpan();
            var spanBuffer = new ByteSpanBuffer(data);
            var monster = MonsterTest.StackBuffer.Monster.GetRootAsMonster(spanBuffer);

            var pos = monster.Pos;
            Assert.IsTrue(pos.HasValue);
            Assert.AreEqual(1.0f, pos.Value.X, 3);

            pos.Value.MutateX(100.0f);
            pos.Value.MutateY(200.0f);
            pos.Value.MutateZ(300.0f);
            
            var monster2 = MonsterTest.StackBuffer.Monster.GetRootAsMonster(spanBuffer);
            var pos2 = monster2.Pos;
            Assert.AreEqual(100.0f, pos2.Value.X, 3);
            Assert.AreEqual(200.0f, pos2.Value.Y, 3);
            Assert.AreEqual(300.0f, pos2.Value.Z, 3);
        }

        [FlatBuffersTestMethod]
        public void ByteBuffer_TypeAliases_MutateMultiByteVectorElement_Success()
        {
            var builder = new FlatBufferBuilder(1024);
            Span<double> vf64Data = stackalloc double[] { 1.1, 2.2, 3.3, 4.4 };
            var vf64Vector = MyGame.Example.TypeAliases.CreateVf64VectorBlock(builder, vf64Data);

            MyGame.Example.TypeAliases.StartTypeAliases(builder);
            MyGame.Example.TypeAliases.AddVf64(builder, vf64Vector);
            var offset = MyGame.Example.TypeAliases.EndTypeAliases(builder);
            builder.Finish(offset.Value);

            var buffer = builder.DataBuffer;
            var ta = MyGame.Example.TypeAliases.GetRootAsTypeAliases(buffer);

            var vf64 = ta.Vf64;
            Assert.IsTrue(vf64.HasValue);
            Assert.AreEqual(4, vf64.Value.Length);
            Assert.AreEqual(1.1, vf64.Value[0], 6);
            Assert.AreEqual(3.3, vf64.Value[2], 6);

            Assert.IsTrue(ta.MutateVf64(0, 10.5));
            Assert.IsTrue(ta.MutateVf64(2, 30.5));

            var ta2 = MyGame.Example.TypeAliases.GetRootAsTypeAliases(buffer);
            var vf64_2 = ta2.Vf64;
            Assert.AreEqual(10.5, vf64_2.Value[0], 6);
            Assert.AreEqual(2.2,  vf64_2.Value[1], 6);
            Assert.AreEqual(30.5, vf64_2.Value[2], 6);
            Assert.AreEqual(4.4,  vf64_2.Value[3], 6);
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_TypeAliases_MutateMultiByteVectorElement_Success()
        {
            var bb = new ByteSpanBuffer(new byte[1024]);
            var fbb = new FlatSpanBufferBuilder(bb, vtableSpace: new int[16], vtableOffsetSpace: new int[16]);

            Span<double> vf64Data = stackalloc double[] { 1.1, 2.2, 3.3, 4.4 };
            var vf64Vector = MyGame.Example.StackBuffer.TypeAliases.CreateVf64VectorBlock(ref fbb, vf64Data);

            MyGame.Example.StackBuffer.TypeAliases.StartTypeAliases(ref fbb);
            MyGame.Example.StackBuffer.TypeAliases.AddVf64(ref fbb, vf64Vector);
            var offset = MyGame.Example.StackBuffer.TypeAliases.EndTypeAliases(ref fbb);
            fbb.Finish(offset.Value);

            var spanBuffer = fbb.DataBuffer;
            var ta = MyGame.Example.StackBuffer.TypeAliases.GetRootAsTypeAliases(spanBuffer);

            var vf64 = ta.Vf64;
            Assert.IsTrue(vf64.HasValue);
            Assert.AreEqual(4, vf64.Value.Length);
            Assert.AreEqual(1.1, vf64.Value[0], 6);
            Assert.AreEqual(3.3, vf64.Value[2], 6);

            Assert.IsTrue(ta.MutateVf64(0, 10.5));
            Assert.IsTrue(ta.MutateVf64(2, 30.5));

            var ta2 = MyGame.Example.StackBuffer.TypeAliases.GetRootAsTypeAliases(spanBuffer);
            var vf64_2 = ta2.Vf64;
            Assert.AreEqual(10.5, vf64_2.Value[0], 6);
            Assert.AreEqual(2.2,  vf64_2.Value[1], 6);
            Assert.AreEqual(30.5, vf64_2.Value[2], 6);
            Assert.AreEqual(4.4,  vf64_2.Value[3], 6);
        }
    }
}
