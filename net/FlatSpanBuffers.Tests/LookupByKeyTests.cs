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
using Google.FlatSpanBuffers;
using KeyTest;
using SpanKeyTest = KeyTest.StackBuffer;

namespace Google.FlatSpanBuffers.Tests
{
    [FlatBuffersTestClass]
    public class LookupByKeyTests
    {
        [FlatBuffersTestMethod]
        public void TestStringKeyLookup_Monster_Found()
        {
            var builder = new FlatBufferBuilder(1024);

            var name1 = builder.CreateString("Alice");
            var name2 = builder.CreateString("Bob");
            var name3 = builder.CreateString("Charlie");

            var monster1 = KeyTestMonster.CreateKeyTestMonster(builder, name1, 100);
            var monster2 = KeyTestMonster.CreateKeyTestMonster(builder, name2, 200);
            var monster3 = KeyTestMonster.CreateKeyTestMonster(builder, name3, 150);

            Span<Offset<KeyTestMonster>> offsets = stackalloc Offset<KeyTestMonster>[] { monster1, monster2, monster3 };
            var vectorOffset = KeyTestMonster.CreateSortedVectorOfKeyTestMonster(builder, offsets);

            builder.StartTable(1);
            builder.AddOffset(0, vectorOffset.Value, 0);
            var rootOffset = builder.EndTable();
            builder.Finish(rootOffset);

            var bb = builder.DataBuffer;
            var rootTable = new Table(bb.Get<int>(bb.Position) + bb.Position, bb);
            var fieldOffset = rootTable.__offset(4);
            var vectorLocation = rootTable.__vector(fieldOffset);

            // Test finding each monster
            Assert.IsTrue(KeyTestMonster.TryGetByKey(vectorLocation, "Alice", bb, out var foundAlice));
            Assert.IsTrue(KeyTestMonster.TryGetByKey(vectorLocation, "Bob", bb, out var foundBob));
            Assert.IsTrue(KeyTestMonster.TryGetByKey(vectorLocation, "Charlie", bb, out var foundCharlie));

            Assert.AreEqual("Alice", foundAlice.Name);
            Assert.AreEqual(100, foundAlice.Hp);

            Assert.AreEqual("Bob", foundBob.Name);
            Assert.AreEqual(200, foundBob.Hp);

            Assert.AreEqual("Charlie", foundCharlie.Name);
            Assert.AreEqual(150, foundCharlie.Hp);
        }

        [FlatBuffersTestMethod]
        public void TestStringKeyLookup_Monster_NotFound()
        {
            var builder = new FlatBufferBuilder(1024);

            var name1 = builder.CreateString("Alice");
            var name2 = builder.CreateString("Charlie");

            var monster1 = KeyTestMonster.CreateKeyTestMonster(builder, name1, 100);
            var monster2 = KeyTestMonster.CreateKeyTestMonster(builder, name2, 150);

            Span<Offset<KeyTestMonster>> offsets = stackalloc Offset<KeyTestMonster>[] { monster1, monster2 };
            var vectorOffset = KeyTestMonster.CreateSortedVectorOfKeyTestMonster(builder, offsets);

            builder.StartTable(1);
            builder.AddOffset(0, vectorOffset.Value, 0);
            var rootOffset = builder.EndTable();
            builder.Finish(rootOffset);

            // Test lookup
            var bb = builder.DataBuffer;
            var rootTable = new Table(bb.Get<int>(bb.Position) + bb.Position, bb);
            var fieldOffset = rootTable.__offset(4);
            var vectorLocation = rootTable.__vector(fieldOffset);

            // Test looking for a monster that doesn't exist
            Assert.IsFalse(KeyTestMonster.TryGetByKey(vectorLocation, "Bob", bb, out _));
            Assert.IsFalse(KeyTestMonster.TryGetByKey(vectorLocation, "Dave", bb, out _));
        }

        [FlatBuffersTestMethod]
        public void TestScalarKeyLookup_Weapon_Found()
        {
            var builder = new FlatBufferBuilder(1024);

            var sword = builder.CreateString("Sword");
            var bow = builder.CreateString("Bow");
            var staff = builder.CreateString("Staff");

            var weapon1 = KeyTestWeapon.CreateKeyTestWeapon(builder, 10, sword, 50);    // id, name, damage
            var weapon2 = KeyTestWeapon.CreateKeyTestWeapon(builder, 20, bow, 30);      // id, name, damage
            var weapon3 = KeyTestWeapon.CreateKeyTestWeapon(builder, 15, staff, 40);    // id, name, damage

            Span<Offset<KeyTestWeapon>> offsets = stackalloc Offset<KeyTestWeapon>[] { weapon1, weapon2, weapon3 };
            var vectorOffset = KeyTestWeapon.CreateSortedVectorOfKeyTestWeapon(builder, offsets);

            builder.StartTable(1);
            builder.AddOffset(0, vectorOffset.Value, 0);
            var rootOffset = builder.EndTable();
            builder.Finish(rootOffset);

            var bb = builder.DataBuffer;
            var rootTable = new Table(bb.Get<int>(bb.Position) + bb.Position, bb);
            var fieldOffset = rootTable.__offset(4);
            var vectorLocation = rootTable.__vector(fieldOffset);

            Assert.IsTrue(vectorLocation != 0);

            // Test finding each weapon by ID
            Assert.IsTrue(KeyTestWeapon.TryGetByKey(vectorLocation, 10, bb, out var found10));
            Assert.IsTrue(KeyTestWeapon.TryGetByKey(vectorLocation, 15, bb, out var found15));
            Assert.IsTrue(KeyTestWeapon.TryGetByKey(vectorLocation, 20, bb, out var found20));

            Assert.AreEqual(10, found10.Id);
            Assert.AreEqual("Sword", found10.Name);
            Assert.AreEqual(50, found10.Damage);

            Assert.AreEqual(15, found15.Id);
            Assert.AreEqual("Staff", found15.Name);
            Assert.AreEqual(40, found15.Damage);

            Assert.AreEqual(20, found20.Id);
            Assert.AreEqual("Bow", found20.Name);
            Assert.AreEqual(30, found20.Damage);
        }

        [FlatBuffersTestMethod]
        public void TestScalarKeyLookup_Weapon_NotFound()
        {
            var builder = new FlatBufferBuilder(1024);

            var sword = builder.CreateString("Sword");
            var bow = builder.CreateString("Bow");

            var weapon1 = KeyTestWeapon.CreateKeyTestWeapon(builder, 10, sword, 50);
            var weapon2 = KeyTestWeapon.CreateKeyTestWeapon(builder, 20, bow, 30);

            Span<Offset<KeyTestWeapon>> offsets = stackalloc Offset<KeyTestWeapon>[] { weapon1, weapon2 };
            var vectorOffset = KeyTestWeapon.CreateSortedVectorOfKeyTestWeapon(builder, offsets);

            builder.StartTable(1);
            builder.AddOffset(0, vectorOffset.Value, 0);
            var rootOffset = builder.EndTable();
            builder.Finish(rootOffset);

            var bb = builder.DataBuffer;
            var rootTable = new Table(bb.Get<int>(bb.Position) + bb.Position, bb);
            var fieldOffset = rootTable.__offset(4);
            var vectorLocation = rootTable.__vector(fieldOffset);

            Assert.IsTrue(vectorLocation != 0);

            // Test looking for weapons that don't exist
            Assert.IsFalse(KeyTestWeapon.TryGetByKey(vectorLocation, 5, bb, out _));
            Assert.IsFalse(KeyTestWeapon.TryGetByKey(vectorLocation, 15, bb, out _));
            Assert.IsFalse(KeyTestWeapon.TryGetByKey(vectorLocation, 25, bb, out _));
        }

        [FlatBuffersTestMethod]
        public void TestEmptyVector_Lookup()
        {
            var builder = new FlatBufferBuilder(1024);

            var vectorOffset = builder.CreateVectorOfTables<KeyTestMonster>(new Offset<KeyTestMonster>[0]);

            builder.StartTable(1);
            builder.AddOffset(0, vectorOffset.Value, 0);
            var rootOffset = builder.EndTable();
            builder.Finish(rootOffset);

            var bb = builder.DataBuffer;
            var rootTable = new Table(bb.Get<int>(bb.Position) + bb.Position, bb);
            var fieldOffset = rootTable.__offset(4);
            var vectorLocation = rootTable.__vector(fieldOffset);

            var result = KeyTestMonster.TryGetByKey(vectorLocation, "Alice", bb, out _);
            Assert.IsFalse(result);
        }

        [FlatBuffersTestMethod]
        public void TestSingleElement_Lookup()
        {
            var builder = new FlatBufferBuilder(1024);

            var name = builder.CreateString("OnlyOne");
            var monster = KeyTestMonster.CreateKeyTestMonster(builder, name, 42);

            Span<Offset<KeyTestMonster>> offsets = stackalloc Offset<KeyTestMonster>[] { monster };
            var vectorOffset = KeyTestMonster.CreateSortedVectorOfKeyTestMonster(builder, offsets);

            builder.StartTable(1);
            builder.AddOffset(0, vectorOffset.Value, 0);
            var rootOffset = builder.EndTable();
            builder.Finish(rootOffset);

            // Test lookup
            var bb = builder.DataBuffer;
            var rootTable = new Table(bb.Get<int>(bb.Position) + bb.Position, bb);
            var fieldOffset = rootTable.__offset(4);
            var vectorLocation = rootTable.__vector(fieldOffset);

            // Should find the single element
            Assert.IsTrue(KeyTestMonster.TryGetByKey(vectorLocation, "OnlyOne", bb, out var found));
            Assert.AreEqual("OnlyOne", found.Name);
            Assert.AreEqual(42, found.Hp);

            // Should not find different keys
            Assert.IsFalse(KeyTestMonster.TryGetByKey(vectorLocation, "Missing", bb, out _));
        }

        [FlatBuffersTestMethod]
        public void TestLargeVector_StringLookup()
        {
            var builder = new FlatBufferBuilder(8192);

            var offsets = new Offset<KeyTestMonster>[100];
            for (int i = 0; i < 100; i++)
            {
                var name = builder.CreateString($"Monster{i:D3}"); // Monster000, Monster001, etc.
                offsets[i] = KeyTestMonster.CreateKeyTestMonster(builder, name, i * 10);
            }

            var vectorOffset = KeyTestMonster.CreateSortedVectorOfKeyTestMonster(builder, offsets.AsSpan());

            // Finish the buffer
            builder.StartTable(1);
            builder.AddOffset(0, vectorOffset.Value, 0);
            var rootOffset = builder.EndTable();
            builder.Finish(rootOffset);

            // Test lookup
            var bb = builder.DataBuffer;
            var rootTable = new Table(bb.Get<int>(bb.Position) + bb.Position, bb);
            var fieldOffset = rootTable.__offset(4);
            var vectorLocation = rootTable.__vector(fieldOffset);

            // Test finding various monsters
            Assert.IsTrue(KeyTestMonster.TryGetByKey(vectorLocation, "Monster000", bb, out var found000));
            Assert.IsTrue(KeyTestMonster.TryGetByKey(vectorLocation, "Monster050", bb, out var found050));
            Assert.IsTrue(KeyTestMonster.TryGetByKey(vectorLocation, "Monster099", bb, out var found099));

            Assert.AreEqual("Monster000", found000.Name);
            Assert.AreEqual(0, found000.Hp);

            Assert.AreEqual("Monster050", found050.Name);
            Assert.AreEqual(500, found050.Hp);

            Assert.AreEqual("Monster099", found099.Name);
            Assert.AreEqual(990, found099.Hp);

            // Test not found
            Assert.IsFalse(KeyTestMonster.TryGetByKey(vectorLocation, "Monster100", bb, out _));
        }

        [FlatBuffersTestMethod]
        public void TestEdgeCases_DuplicateKeys()
        {
            // Duplicate keys in sorted vectors are unspecified behavior in FlatBuffers.
            // This test only verifies the lookup does not crash and returns a valid entry.
            var builder = new FlatBufferBuilder(1024);

            var name = builder.CreateString("Duplicate");
            var monster1 = KeyTestMonster.CreateKeyTestMonster(builder, name, 100);
            var monster2 = KeyTestMonster.CreateKeyTestMonster(builder, name, 200);

            Span<Offset<KeyTestMonster>> offsets = stackalloc Offset<KeyTestMonster>[] { monster1, monster2 };
            var vectorOffset = KeyTestMonster.CreateSortedVectorOfKeyTestMonster(builder, offsets);

            builder.StartTable(1);
            builder.AddOffset(0, vectorOffset.Value, 0);
            var rootOffset = builder.EndTable();
            builder.Finish(rootOffset);

            var bb = builder.DataBuffer;
            var rootTable = new Table(bb.Get<int>(bb.Position) + bb.Position, bb);
            var fieldOffset = rootTable.__offset(4);
            var vectorLocation = rootTable.__vector(fieldOffset);

            // Should find one of the duplicates without throwing
            Assert.IsTrue(KeyTestMonster.TryGetByKey(vectorLocation, "Duplicate", bb, out var found));
            Assert.AreEqual("Duplicate", found.Name);
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_TestStringKeyLookup_Found()
        {
            var bbSpan = new ByteSpanBuffer(new byte[1024]);
            var builder = new FlatSpanBufferBuilder(bbSpan, vtableSpace: new int[4], vtableOffsetSpace: new int[16]);

            var name1 = builder.CreateString("Alice");
            var name2 = builder.CreateString("Bob");
            var name3 = builder.CreateString("Charlie");

            var monster1 = SpanKeyTest.KeyTestMonster.CreateKeyTestMonster(ref builder, name1, 100);
            var monster2 = SpanKeyTest.KeyTestMonster.CreateKeyTestMonster(ref builder, name2, 200);
            var monster3 = SpanKeyTest.KeyTestMonster.CreateKeyTestMonster(ref builder, name3, 150);

            Span<Offset<SpanKeyTest.KeyTestMonster>> offsets = stackalloc Offset<SpanKeyTest.KeyTestMonster>[] { monster1, monster2, monster3 };
            var vectorOffset = SpanKeyTest.KeyTestMonster.CreateSortedVectorOfKeyTestMonster(ref builder, offsets);

            builder.StartTable(1);
            builder.AddOffset(0, vectorOffset.Value, 0);
            var rootOffset = builder.EndTable();
            builder.Finish(rootOffset);

            var bb = builder.DataBuffer;
            var rootTable = new TableSpan(bb.Get<int>(bb.Position) + bb.Position, bb);
            var fieldOffset = rootTable.__offset(4);
            var vectorLocation = rootTable.__vector(fieldOffset);

            // Test finding each monster
            Assert.IsTrue(SpanKeyTest.KeyTestMonster.TryGetByKey(vectorLocation, "Alice", bb, out var foundAlice));
            Assert.IsTrue(SpanKeyTest.KeyTestMonster.TryGetByKey(vectorLocation, "Bob", bb, out var foundBob));
            Assert.IsTrue(SpanKeyTest.KeyTestMonster.TryGetByKey(vectorLocation, "Charlie", bb, out var foundCharlie));

            Assert.AreEqual("Alice", foundAlice.Name);
            Assert.AreEqual(100, foundAlice.Hp);

            Assert.AreEqual("Bob", foundBob.Name);
            Assert.AreEqual(200, foundBob.Hp);

            Assert.AreEqual("Charlie", foundCharlie.Name);
            Assert.AreEqual(150, foundCharlie.Hp);

            // Verify not-found also works
            Assert.IsFalse(SpanKeyTest.KeyTestMonster.TryGetByKey(vectorLocation, "Dave", bb, out _));
        }
    }
}
