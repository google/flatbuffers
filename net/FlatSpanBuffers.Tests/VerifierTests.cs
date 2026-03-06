/*
 * Copyright 2015 Google Inc. All rights reserved.
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
using ComprehensiveTest;

namespace Google.FlatSpanBuffers.Tests
{
    /// <summary>
    /// Verifier coverage for valid, corrupted, and constrained buffers.
    /// </summary>
    [FlatBuffersTestClass]
    public class VerifierTests
    {
        private static bool VerifyMonsterDirect(ByteBuffer buffer)
        {
            var verifier = new Verifier(buffer);
            return verifier.VerifyBuffer("", false, MonsterVerify.Verify);
        }

        private static bool VerifyMonsterWithOptions(ByteBuffer buffer, Options options, bool sizePrefixed = false)
        {
            var verifier = new Verifier(buffer, options);
            return verifier.VerifyBuffer("", sizePrefixed, MonsterVerify.Verify);
        }

        [FlatBuffersTestMethod]
        public void Monster_VerifyValidBuffer_ReturnsTrue()
        {
            var builder = new FlatBufferBuilder(1024);

            var nameOffset = builder.CreateString("Orc");
            
            Monster.StartMonster(builder);
            Monster.AddName(builder, nameOffset);
            Monster.AddHp(builder, 300);
            Monster.AddMana(builder, 150);
            var monsterOffset = Monster.EndMonster(builder);

            Monster.FinishMonsterBuffer(builder, monsterOffset);

            var buffer = builder.DataBuffer;
            Assert.IsTrue(VerifyMonsterDirect(buffer));
        }

        [FlatBuffersTestMethod]
        public void Monster_VerifyWithAllFields_ReturnsTrue()
        {
            var builder = new FlatBufferBuilder(1024);

            var nameOffset = builder.CreateString("Boss Monster");

            Span<byte> inventoryData = stackalloc byte[] { 1, 2, 3 };
            var inventoryVector = Monster.CreateInventoryVectorBlock(builder, inventoryData);

            var sword = MonsterTest.Weapon.CreateWeapon(builder, builder.CreateString("Sword"), 10);
            Span<Offset<MonsterTest.Weapon>> weaponsData = stackalloc Offset<MonsterTest.Weapon>[] { sword };
            var weaponsVector = Monster.CreateWeaponsVectorBlock(builder, weaponsData);

            var equippedName = builder.CreateString("Shield");
            var equipped = MonsterTest.Weapon.CreateWeapon(builder, equippedName, 5);

            Monster.StartPathVector(builder, 2);
            MonsterTest.Vec3.CreateVec3(builder, 1.0f, 2.0f, 3.0f);
            MonsterTest.Vec3.CreateVec3(builder, 4.0f, 5.0f, 6.0f);
            var pathVector = builder.EndVector();

            var pos = new MonsterTest.Vec3T { X = 100.0f, Y = 50.0f, Z = 25.0f };

            Monster.StartMonster(builder);
            Monster.AddPos(builder, MonsterTest.Vec3.Pack(builder, pos));
            Monster.AddMana(builder, 200);
            Monster.AddHp(builder, 500);
            Monster.AddName(builder, nameOffset);
            Monster.AddInventory(builder, inventoryVector);
            Monster.AddColor(builder, MonsterTest.Color.Red);
            Monster.AddWeapons(builder, weaponsVector);
            Monster.AddEquippedType(builder, MonsterTest.Equipment.Weapon);
            Monster.AddEquipped(builder, equipped.Value);
            Monster.AddPath(builder, pathVector);
            var monsterOffset = Monster.EndMonster(builder);

            Monster.FinishMonsterBuffer(builder, monsterOffset);

            var buffer = builder.DataBuffer;
            Assert.IsTrue(VerifyMonsterDirect(buffer));
        }

        [FlatBuffersTestMethod]
        public void GameSession_VerifyValidBuffer_ReturnsTrue()
        {
            var builder = new FlatBufferBuilder(1024);

            var sessionIdOffset = builder.CreateString("SESSION-001");

            GameSession.StartGameSession(builder);
            GameSession.AddSessionId(builder, sessionIdOffset);
            GameSession.AddPlayerCount(builder, 4);
            GameSession.AddStartTime(builder, DateTimeOffset.UtcNow.ToUnixTimeSeconds());
            var sessionOffset = GameSession.EndGameSession(builder);

            GameSession.FinishGameSessionBuffer(builder, sessionOffset);

            var buffer = builder.DataBuffer;
            Assert.IsTrue(GameSession.VerifyGameSession(buffer));
        }

        [FlatBuffersTestMethod]
        public void Player_VerifyWithAllBasicFields_ReturnsTrue()
        {
            var builder = new FlatBufferBuilder(1024);

            var playerName = builder.CreateString("CompletePlayer");

            Span<int> statsData = stackalloc int[] { 10, 15, 8 };
            var statsVector = Player.CreateStatsVectorBlock(builder, statsData);

            var skill1 = builder.CreateString("Slash");
            Span<StringOffset> skillsData = stackalloc StringOffset[] { skill1 };
            var skillsVector = Player.CreateSkillsVectorBlock(builder, skillsData);

            Player.StartPlayer(builder);
            Player.AddId(builder, 42);
            Player.AddName(builder, playerName);
            Player.AddLevel(builder, 25);
            Player.AddExperience(builder, 150000);
            Player.AddHealth(builder, 200.0f);
            Player.AddMana(builder, 100.0f);
            Player.AddStats(builder, statsVector);
            Player.AddSkills(builder, skillsVector);
            Player.AddStatus(builder, Status.Active);
            var playerOffset = Player.EndPlayer(builder);

            builder.Finish(playerOffset.Value);

            var buffer = builder.DataBuffer;
            var verifier = new Verifier(buffer);
            var rootOffset = buffer.Get<uint>(buffer.Position);
            Assert.IsTrue(PlayerVerify.Verify(ref verifier, (uint)buffer.Position + rootOffset));
        }

        [FlatBuffersTestMethod]
        public void MonsterWeapon_VerifyValidBuffer_ReturnsTrue()
        {
            var builder = new FlatBufferBuilder(512);

            var weaponName = builder.CreateString("Excalibur");
            var weapon = MonsterTest.Weapon.CreateWeapon(builder, weaponName, 100);

            builder.Finish(weapon.Value);

            var buffer = builder.DataBuffer;
            var verifier = new Verifier(buffer);
            var rootOffset = buffer.Get<uint>(buffer.Position);
            Assert.IsTrue(MonsterTest.WeaponVerify.Verify(ref verifier, (uint)buffer.Position + rootOffset));
        }

        [FlatBuffersTestMethod]
        public void Monster_VerifyCorruptedRootOffset_ReturnsFalse()
        {
            var builder = new FlatBufferBuilder(1024);

            var nameOffset = builder.CreateString("Orc");

            Monster.StartMonster(builder);
            Monster.AddName(builder, nameOffset);
            Monster.AddHp(builder, 100);
            var monsterOffset = Monster.EndMonster(builder);

            Monster.FinishMonsterBuffer(builder, monsterOffset);
            
            var sizedSpan = builder.DataBuffer.ToSizedSpan();
            var sourceData = sizedSpan.ToArray();

            // Corrupt by setting root offset to point way outside buffer
            // The root offset is at the beginning of the sized span (position 0 in the array)
            sourceData[0] = 0xFF;
            sourceData[1] = 0xFF;
            sourceData[2] = 0xFF;
            sourceData[3] = 0x7F;

            var corruptedBuffer = new ByteBuffer(sourceData);
            Assert.IsFalse(VerifyMonsterDirect(corruptedBuffer));
        }

        [FlatBuffersTestMethod]
        public void Monster_VerifyTruncatedBuffer_ReturnsFalse()
        {
            var builder = new FlatBufferBuilder(1024);

            var nameOffset = builder.CreateString("Orc");

            Monster.StartMonster(builder);
            Monster.AddName(builder, nameOffset);
            var monsterOffset = Monster.EndMonster(builder);

            Monster.FinishMonsterBuffer(builder, monsterOffset);

            // Get the buffer data and truncate it
            var sourceSpan = builder.DataBuffer.ToSizedSpan();
            var truncatedLength = Math.Max(8, sourceSpan.Length / 2);
            var truncatedData = sourceSpan.Slice(0, truncatedLength).ToArray();

            var truncatedBuffer = new ByteBuffer(truncatedData);
            Assert.IsFalse(VerifyMonsterDirect(truncatedBuffer));
        }

        [FlatBuffersTestMethod]
        public void Monster_VerifyEmptyBuffer_ReturnsFalse()
        {
            var emptyBuffer = new ByteBuffer(new byte[4]);
            Assert.IsFalse(VerifyMonsterDirect(emptyBuffer));
        }

        [FlatBuffersTestMethod]
        public void Monster_VerifyWithCustomOptions_ReturnsTrue()
        {
            var builder = new FlatBufferBuilder(1024);

            var nameOffset = builder.CreateString("Test Monster");

            Monster.StartMonster(builder);
            Monster.AddName(builder, nameOffset);
            var monsterOffset = Monster.EndMonster(builder);

            Monster.FinishMonsterBuffer(builder, monsterOffset);

            var buffer = builder.DataBuffer;

            // Test with custom options
            var options = new Options(
                maxDepth: 32,
                maxTables: 500000,
                stringEndCheck: true,
                alignmentCheck: true
            );

            Assert.IsTrue(VerifyMonsterWithOptions(buffer, options));
        }

        [FlatBuffersTestMethod]
        public void Monster_VerifyWithDisabledAlignmentCheck_ReturnsTrue()
        {
            var builder = new FlatBufferBuilder(1024);

            var nameOffset = builder.CreateString("Test Monster");

            Monster.StartMonster(builder);
            Monster.AddName(builder, nameOffset);
            var monsterOffset = Monster.EndMonster(builder);

            Monster.FinishMonsterBuffer(builder, monsterOffset);

            var buffer = builder.DataBuffer;

            // Test with alignment check disabled
            var options = new Options(
                maxDepth: Options.DEFAULT_MAX_DEPTH,
                maxTables: Options.DEFAULT_MAX_TABLES,
                stringEndCheck: true,
                alignmentCheck: false
            );

            Assert.IsTrue(VerifyMonsterWithOptions(buffer, options));
        }

        [FlatBuffersTestMethod]
        public void Monster_VerifyWithDisabledStringEndCheck_ReturnsTrue()
        {
            var builder = new FlatBufferBuilder(1024);

            var nameOffset = builder.CreateString("Test Monster");

            Monster.StartMonster(builder);
            Monster.AddName(builder, nameOffset);
            var monsterOffset = Monster.EndMonster(builder);

            Monster.FinishMonsterBuffer(builder, monsterOffset);

            var buffer = builder.DataBuffer;

            var options = new Options(
                maxDepth: Options.DEFAULT_MAX_DEPTH,
                maxTables: Options.DEFAULT_MAX_TABLES,
                stringEndCheck: false,
                alignmentCheck: true
            );

            Assert.IsTrue(VerifyMonsterWithOptions(buffer, options));
        }

        [FlatBuffersTestMethod]
        public void Monster_VerifySizePrefixedBuffer_ReturnsTrue()
        {
            var builder = new FlatBufferBuilder(1024);

            var nameOffset = builder.CreateString("Size Prefixed Monster");

            Monster.StartMonster(builder);
            Monster.AddName(builder, nameOffset);
            Monster.AddHp(builder, 200);
            var monsterOffset = Monster.EndMonster(builder);

            Monster.FinishSizePrefixedMonsterBuffer(builder, monsterOffset);

            var buffer = builder.DataBuffer;
            var options = new Options();
            Assert.IsTrue(VerifyMonsterWithOptions(buffer, options, sizePrefixed: true));
        }

        [FlatBuffersTestMethod]
        public void GameSession_VerifySizePrefixedBuffer_ReturnsTrue()
        {
            var builder = new FlatBufferBuilder(1024);

            var sessionIdOffset = builder.CreateString("SIZED-SESSION-001");

            GameSession.StartGameSession(builder);
            GameSession.AddSessionId(builder, sessionIdOffset);
            GameSession.AddPlayerCount(builder, 8);
            var sessionOffset = GameSession.EndGameSession(builder);

            GameSession.FinishSizePrefixedGameSessionBuffer(builder, sessionOffset);

            var buffer = builder.DataBuffer;
            var verifier = new Verifier(buffer);
            var result = verifier.VerifyBuffer("TEST", true, GameSessionVerify.Verify);
            Assert.IsTrue(result);
        }

        [FlatBuffersTestMethod]
        public void Monster_VerifyWithByteSpanBuffer_ReturnsTrue()
        {
            var builder = new FlatBufferBuilder(1024);

            var nameOffset = builder.CreateString("Span Monster");

            Monster.StartMonster(builder);
            Monster.AddName(builder, nameOffset);
            Monster.AddHp(builder, 150);
            var monsterOffset = Monster.EndMonster(builder);

            Monster.FinishMonsterBuffer(builder, monsterOffset);

            // Create ByteSpanBuffer from the sized data (starts at position 0)
            var bufferSpan = builder.DataBuffer.ToSizedSpan();
            var byteSpanBuffer = new ByteSpanBuffer(bufferSpan);

            var verifier = new Verifier(byteSpanBuffer);
            var result = verifier.VerifyBuffer(null, false, MonsterVerify.Verify);
            Assert.IsTrue(result);
        }

        [FlatBuffersTestMethod]
        public void VerifyDeeplyNestedTables_WithLowMaxDepth_ReturnsFalse()
        {
            var builder = new FlatBufferBuilder(2048);

            var weaponName = builder.CreateString("Nested Weapon");
            var weapon = MonsterTest.Weapon.CreateWeapon(builder, weaponName, 10);

            Span<Offset<MonsterTest.Weapon>> weaponsData = stackalloc Offset<MonsterTest.Weapon>[] { weapon };
            var weaponsVector = Monster.CreateWeaponsVectorBlock(builder, weaponsData);

            var nameOffset = builder.CreateString("Deep Monster");

            Monster.StartMonster(builder);
            Monster.AddName(builder, nameOffset);
            Monster.AddWeapons(builder, weaponsVector);
            var monsterOffset = Monster.EndMonster(builder);

            Monster.FinishMonsterBuffer(builder, monsterOffset);

            var buffer = builder.DataBuffer;

            // Test with very low max depth
            var options = new Options(
                maxDepth: 1,
                maxTables: Options.DEFAULT_MAX_TABLES,
                stringEndCheck: true,
                alignmentCheck: true
            );

            Assert.IsFalse(VerifyMonsterWithOptions(buffer, options));
        }

        [FlatBuffersTestMethod]
        public void VerifyManyTables_WithLowMaxTables_ReturnsFalse()
        {
            var builder = new FlatBufferBuilder(4096);

            var weapons = new Offset<MonsterTest.Weapon>[5];
            for (int i = 0; i < 5; i++)
            {
                var weaponName = builder.CreateString($"Weapon{i}");
                weapons[i] = MonsterTest.Weapon.CreateWeapon(builder, weaponName, (short)(i * 10));
            }

            var weaponsVector = Monster.CreateWeaponsVectorBlock(builder, weapons);
            var nameOffset = builder.CreateString("Multi-Weapon Monster");

            Monster.StartMonster(builder);
            Monster.AddName(builder, nameOffset);
            Monster.AddWeapons(builder, weaponsVector);
            var monsterOffset = Monster.EndMonster(builder);

            Monster.FinishMonsterBuffer(builder, monsterOffset);

            var buffer = builder.DataBuffer;

            // Test with very low max tables
            var options = new Options(
                maxDepth: Options.DEFAULT_MAX_DEPTH,
                maxTables: 1,
                stringEndCheck: true,
                alignmentCheck: true
            );

            Assert.IsFalse(VerifyMonsterWithOptions(buffer, options));
        }

        [FlatBuffersTestMethod]
        public void VerifyDeeplyNestedTables_WithHighMaxDepth_ReturnsTrue()
        {
            var builder = new FlatBufferBuilder(2048);

            // Create nested weapons
            var weaponName = builder.CreateString("Nested Weapon");
            var weapon = MonsterTest.Weapon.CreateWeapon(builder, weaponName, 10);

            Span<Offset<MonsterTest.Weapon>> weaponsData = stackalloc Offset<MonsterTest.Weapon>[] { weapon };
            var weaponsVector = Monster.CreateWeaponsVectorBlock(builder, weaponsData);

            var nameOffset = builder.CreateString("Deep Monster");

            Monster.StartMonster(builder);
            Monster.AddName(builder, nameOffset);
            Monster.AddWeapons(builder, weaponsVector);
            var monsterOffset = Monster.EndMonster(builder);

            Monster.FinishMonsterBuffer(builder, monsterOffset);

            var buffer = builder.DataBuffer;

            // Test with sufficient max depth
            var options = new Options(
                maxDepth: 10,
                maxTables: Options.DEFAULT_MAX_TABLES,
                stringEndCheck: true,
                alignmentCheck: true
            );

            Assert.IsTrue(VerifyMonsterWithOptions(buffer, options));
        }

        [FlatBuffersTestMethod]
        public void Monster_SerializeAndVerify_RoundTrip()
        {
            var originalMonster = new MonsterT
            {
                Name = "Roundtrip Monster",
                Hp = 500,
                Mana = 200,
                Color = MonsterTest.Color.Red
            };

            var binaryData = originalMonster.SerializeToBinary();

            var buffer = new ByteBuffer(binaryData);
            Assert.IsTrue(VerifyMonsterDirect(buffer));

            // Deserialize and verify
            var deserializedMonster = MonsterT.DeserializeFromBinary(binaryData);
            Assert.AreEqual(originalMonster.Name, deserializedMonster.Name);
            Assert.AreEqual(originalMonster.Hp, deserializedMonster.Hp);
            Assert.AreEqual(originalMonster.Mana, deserializedMonster.Mana);
            Assert.AreEqual(originalMonster.Color, deserializedMonster.Color);
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_Monster_VerifyValidBuffer_ReturnsTrue()
        {
            var builder = new FlatBufferBuilder(1024);

            var nameOffset = builder.CreateString("StackBuffer Monster");

            Monster.StartMonster(builder);
            Monster.AddName(builder, nameOffset);
            Monster.AddHp(builder, 200);
            Monster.AddMana(builder, 100);
            var monsterOffset = Monster.EndMonster(builder);

            Monster.FinishMonsterBuffer(builder, monsterOffset);

            var sourceSpan = builder.DataBuffer.ToSizedSpan();
            var byteSpanBuffer = new ByteSpanBuffer(sourceSpan);

            var result = MonsterTest.StackBuffer.Monster.VerifyMonster(byteSpanBuffer);
            Assert.IsTrue(result);
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_GameSession_VerifyValidBuffer_ReturnsTrue()
        {
            var builder = new FlatBufferBuilder(2048);

            var sessionId = builder.CreateString("session-12345");

            GameSession.StartGameSession(builder);
            GameSession.AddSessionId(builder, sessionId);
            GameSession.AddPlayerCount(builder, 4);
            GameSession.AddStartTime(builder, 1234567890L);
            var sessionOffset = GameSession.EndGameSession(builder);

            GameSession.FinishGameSessionBuffer(builder, sessionOffset);

            var sourceSpan = builder.DataBuffer.ToSizedSpan();
            var byteSpanBuffer = new ByteSpanBuffer(sourceSpan);

            var result = ComprehensiveTest.StackBuffer.GameSession.VerifyGameSession(byteSpanBuffer);
            Assert.IsTrue(result);
        }

        [FlatBuffersTestMethod]
        public void Verify_DepthCounters_ReturnsFalse()
        {
            var builder = new FlatBufferBuilder(4096);

            var weapons = new Offset<MonsterTest.Weapon>[10];
            for (int i = 0; i < 10; i++)
            {
                var weaponName = builder.CreateString("W" + i);
                weapons[i] = MonsterTest.Weapon.CreateWeapon(builder, weaponName, (short)(i * 10));
            }

            var weaponsVector = Monster.CreateWeaponsVectorBlock(builder, weapons);
            var nameOffset = builder.CreateString("Multi-Weapon Monster");

            Monster.StartMonster(builder);
            Monster.AddName(builder, nameOffset);
            Monster.AddWeapons(builder, weaponsVector);
            var monsterOffset = Monster.EndMonster(builder);

            Monster.FinishMonsterBuffer(builder, monsterOffset);

            var buffer = builder.DataBuffer;

            // 11 Total tables: 1 Monster + 10 Weapons = 11 tables
            // maxTables = 5 should cause failure
            var options = new Options(
                maxDepth: Options.DEFAULT_MAX_DEPTH,
                maxTables: 5,
                stringEndCheck: true,
                alignmentCheck: true
            );

            Assert.IsFalse(VerifyMonsterWithOptions(buffer, options));
        }

        [FlatBuffersTestMethod]
        public void StackBuffer_Monster_VerifyCorruptedBuffer_ReturnsFalse()
        {
            var builder = new FlatBufferBuilder(1024);

            var nameOffset = builder.CreateString("Valid Monster");

            Monster.StartMonster(builder);
            Monster.AddName(builder, nameOffset);
            var monsterOffset = Monster.EndMonster(builder);

            Monster.FinishMonsterBuffer(builder, monsterOffset);

            var sourceSpan = builder.DataBuffer.ToSizedSpan();
            var corruptedData = sourceSpan.ToArray();

            corruptedData[0] = 0xFF;
            corruptedData[1] = 0xFF;
            corruptedData[2] = 0xFF;
            corruptedData[3] = 0xFF;

            var byteSpanBuffer = new ByteSpanBuffer(corruptedData);

            var result = MonsterTest.StackBuffer.Monster.VerifyMonster(byteSpanBuffer);
            Assert.IsFalse(result);
        }
    }
}
