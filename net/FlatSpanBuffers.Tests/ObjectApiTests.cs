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

namespace Google.FlatSpanBuffers.Tests
{
    [FlatBuffersTestClass]
    public class ObjectApiTests
    {
        private static FlatBufferBuilder CreateBuilder(int initialSize = 256)
        {
            var buffer = new ByteBuffer(initialSize);
            return new FlatBufferBuilder(buffer, new int[32], new int[32]);
        }

        [FlatBuffersTestMethod]
        public void UnPack_BasicTable_ReturnsCorrectValues()
        {
            var fbb = CreateBuilder();
            var nameOffset = fbb.CreateString("Excalibur");

            Weapon.StartWeapon(fbb);
            Weapon.AddName(fbb, nameOffset);
            Weapon.AddDamage(fbb, 150);
            Weapon.AddDurability(fbb, 95.5f);
            Weapon.AddEnchanted(fbb, true);
            Weapon.AddRarity(fbb, Color.Blue);
            var weaponOffset = Weapon.EndWeapon(fbb);
            fbb.Finish(weaponOffset.Value);

            var weapon = Weapon.GetRootAsWeapon(fbb.DataBuffer);
            var weaponT = weapon.UnPack();

            Assert.AreEqual("Excalibur", weaponT.Name);
            Assert.AreEqual(150, weaponT.Damage);
            Assert.AreEqual(95.5f, weaponT.Durability, 6);
            Assert.AreEqual(true, weaponT.Enchanted);
            Assert.AreEqual(Color.Blue, weaponT.Rarity);
        }

        [FlatBuffersTestMethod]
        public void UnPack_WithVectorOfScalars_ReturnsCorrectValues()
        {
            var fbb = CreateBuilder();
            var nameOffset = fbb.CreateString("MultiHitSword");

            Span<int> damageValues = stackalloc int[] { 10, 20, 30, 40, 50 };
            var damageValuesOffset = Weapon.CreateDamageValuesVectorBlock(fbb, damageValues);

            Span<float> modifiers = stackalloc float[] { 1.1f, 1.2f, 1.3f };
            var modifiersOffset = Weapon.CreateModifiersVectorBlock(fbb, modifiers);

            Weapon.StartWeapon(fbb);
            Weapon.AddName(fbb, nameOffset);
            Weapon.AddDamageValues(fbb, damageValuesOffset);
            Weapon.AddModifiers(fbb, modifiersOffset);
            var weaponOffset = Weapon.EndWeapon(fbb);
            fbb.Finish(weaponOffset.Value);

            var weapon = Weapon.GetRootAsWeapon(fbb.DataBuffer);
            var weaponT = weapon.UnPack();

            Assert.AreEqual(5, weaponT.DamageValues.Count);
            Assert.AreEqual(10, weaponT.DamageValues[0]);
            Assert.AreEqual(50, weaponT.DamageValues[4]);

            Assert.AreEqual(3, weaponT.Modifiers.Count);
            Assert.AreEqual(1.1f, weaponT.Modifiers[0], 6);
            Assert.AreEqual(1.3f, weaponT.Modifiers[2], 6);
        }

        [FlatBuffersTestMethod]
        public void UnPack_WithVectorOfStrings_ReturnsCorrectValues()
        {
            var fbb = CreateBuilder();
            var nameOffset = fbb.CreateString("TaggedWeapon");

            var tag1 = fbb.CreateString("legendary");
            var tag2 = fbb.CreateString("fire");
            var tag3 = fbb.CreateString("ancient");
            Span<StringOffset> tags = stackalloc StringOffset[] { tag1, tag2, tag3 };
            var tagsOffset = Weapon.CreateTagsVectorBlock(fbb, tags);

            Weapon.StartWeapon(fbb);
            Weapon.AddName(fbb, nameOffset);
            Weapon.AddTags(fbb, tagsOffset);
            var weaponOffset = Weapon.EndWeapon(fbb);
            fbb.Finish(weaponOffset.Value);

            var weapon = Weapon.GetRootAsWeapon(fbb.DataBuffer);
            var weaponT = weapon.UnPack();

            Assert.AreEqual(3, weaponT.Tags.Count);
            Assert.AreEqual("legendary", weaponT.Tags[0]);
            Assert.AreEqual("fire", weaponT.Tags[1]);
            Assert.AreEqual("ancient", weaponT.Tags[2]);
        }

        [FlatBuffersTestMethod]
        public void UnPack_WithVectorOfEnums_ReturnsCorrectValues()
        {
            var fbb = CreateBuilder();
            var nameOffset = fbb.CreateString("ColorfulWeapon");

            Span<Color> colors = stackalloc Color[] { Color.Red, Color.Green, Color.Blue };
            var colorsOffset = Weapon.CreateValidColorsVectorBlock(fbb, colors);

            Weapon.StartWeapon(fbb);
            Weapon.AddName(fbb, nameOffset);
            Weapon.AddValidColors(fbb, colorsOffset);
            var weaponOffset = Weapon.EndWeapon(fbb);
            fbb.Finish(weaponOffset.Value);

            var weapon = Weapon.GetRootAsWeapon(fbb.DataBuffer);
            var weaponT = weapon.UnPack();

            Assert.AreEqual(3, weaponT.ValidColors.Count);
            Assert.AreEqual(Color.Red, weaponT.ValidColors[0]);
            Assert.AreEqual(Color.Green, weaponT.ValidColors[1]);
            Assert.AreEqual(Color.Blue, weaponT.ValidColors[2]);
        }

        [FlatBuffersTestMethod]
        public void UnPackTo_ReusesExistingObject()
        {
            var fbb = CreateBuilder();
            var nameOffset = fbb.CreateString("ReusedWeapon");

            Weapon.StartWeapon(fbb);
            Weapon.AddName(fbb, nameOffset);
            Weapon.AddDamage(fbb, 200);
            var weaponOffset = Weapon.EndWeapon(fbb);
            fbb.Finish(weaponOffset.Value);

            var weapon = Weapon.GetRootAsWeapon(fbb.DataBuffer);

            var existingWeaponT = new WeaponT();
            existingWeaponT.Name = "OldName";
            existingWeaponT.Damage = 50;

            weapon.UnPackTo(existingWeaponT);

            Assert.AreEqual("ReusedWeapon", existingWeaponT.Name);
            Assert.AreEqual(200, existingWeaponT.Damage);
        }

        [FlatBuffersTestMethod]
        public void UnPackTo_WithVectors_ReusesLists()
        {
            var fbb1 = CreateBuilder();
            var name1 = fbb1.CreateString("Weapon1");
            Span<int> damages1 = stackalloc int[] { 1, 2, 3, 4, 5 };
            var damagesOffset1 = Weapon.CreateDamageValuesVectorBlock(fbb1, damages1);

            Weapon.StartWeapon(fbb1);
            Weapon.AddName(fbb1, name1);
            Weapon.AddDamageValues(fbb1, damagesOffset1);
            var weaponOffset1 = Weapon.EndWeapon(fbb1);
            fbb1.Finish(weaponOffset1.Value);

            var weapon1 = Weapon.GetRootAsWeapon(fbb1.DataBuffer);
            var weaponT = weapon1.UnPack();

            var originalList = weaponT.DamageValues;

            var fbb2 = CreateBuilder();
            var name2 = fbb2.CreateString("Weapon2");
            Span<int> damages2 = stackalloc int[] { 10, 20, 30 };
            var damagesOffset2 = Weapon.CreateDamageValuesVectorBlock(fbb2, damages2);

            Weapon.StartWeapon(fbb2);
            Weapon.AddName(fbb2, name2);
            Weapon.AddDamageValues(fbb2, damagesOffset2);
            var weaponOffset2 = Weapon.EndWeapon(fbb2);
            fbb2.Finish(weaponOffset2.Value);

            var weapon2 = Weapon.GetRootAsWeapon(fbb2.DataBuffer);

            weapon2.UnPackTo(weaponT);

            Assert.IsTrue(ReferenceEquals(originalList, weaponT.DamageValues));
            Assert.AreEqual(3, weaponT.DamageValues.Count);
            Assert.AreEqual(10, weaponT.DamageValues[0]);
            Assert.AreEqual(30, weaponT.DamageValues[2]);
        }

        [FlatBuffersTestMethod]
        public void Pack_BasicTable_CreatesCorrectBuffer()
        {
            var weaponT = new WeaponT
            {
                Name = "PackedSword",
                Damage = 175,
                Durability = 88.5f,
                Enchanted = true,
                Rarity = Color.Green,
                Transform = null,
                FixedStats = null,
                PositionHistory = null
            };

            var fbb = CreateBuilder();
            var offset = Weapon.Pack(fbb, weaponT);
            fbb.Finish(offset.Value);

            var weapon = Weapon.GetRootAsWeapon(fbb.DataBuffer);

            // Assert
            Assert.AreEqual("PackedSword", weapon.Name);
            Assert.AreEqual(175, weapon.Damage);
            Assert.AreEqual(88.5f, weapon.Durability, 6);
            Assert.AreEqual(true, weapon.Enchanted);
            Assert.AreEqual(Color.Green, weapon.Rarity);
        }

        [FlatBuffersTestMethod]
        public void Pack_WithVectorOfScalars_CreatesCorrectBuffer()
        {
            var weaponT = new WeaponT
            {
                Name = "VectorWeapon",
                DamageValues = new List<int> { 5, 10, 15, 20 },
                Modifiers = new List<float> { 0.5f, 1.0f, 1.5f },
                Transform = null,
                FixedStats = null,
                PositionHistory = null
            };

            var fbb = CreateBuilder();
            var offset = Weapon.Pack(fbb, weaponT);
            fbb.Finish(offset.Value);

            var weapon = Weapon.GetRootAsWeapon(fbb.DataBuffer);

            Assert.AreEqual(4, weapon.DamageValues.Value.Length);
            Assert.AreEqual(5, weapon.DamageValues.Value[0]);
            Assert.AreEqual(20, weapon.DamageValues.Value[3]);

            Assert.AreEqual(3, weapon.Modifiers.Value.Length);
            Assert.AreEqual(0.5f, weapon.Modifiers.Value[0], 6);
            Assert.AreEqual(1.5f, weapon.Modifiers.Value[2], 6);
        }

        [FlatBuffersTestMethod]
        public void Pack_WithVectorOfStrings_CreatesCorrectBuffer()
        {
            var weaponT = new WeaponT
            {
                Name = "TaggedPacked",
                Tags = new List<string> { "epic", "frost", "blessed" },
                Transform = null,
                FixedStats = null,
                PositionHistory = null
            };

            var fbb = CreateBuilder();
            var offset = Weapon.Pack(fbb, weaponT);
            fbb.Finish(offset.Value);

            var weapon = Weapon.GetRootAsWeapon(fbb.DataBuffer);

            var tags = weapon.Tags;
            Assert.IsTrue(tags.HasValue);
            var tagsVec = tags.Value;
            Assert.AreEqual(3, tagsVec.Length);
            Assert.AreEqual("epic", tagsVec[0]);
            Assert.AreEqual("frost", tagsVec[1]);
            Assert.AreEqual("blessed", tagsVec[2]);
        }

        [FlatBuffersTestMethod]
        public void Pack_WithVectorOfEnums_CreatesCorrectBuffer()
        {
            var weaponT = new WeaponT
            {
                Name = "EnumWeapon",
                ValidColors = new List<Color> { Color.Blue, Color.Red },
                Transform = null,
                FixedStats = null,
                PositionHistory = null
            };

            var fbb = CreateBuilder();
            var offset = Weapon.Pack(fbb, weaponT);
            fbb.Finish(offset.Value);

            var weapon = Weapon.GetRootAsWeapon(fbb.DataBuffer);

            Assert.AreEqual(2, weapon.ValidColors.Value.Length);
            Assert.AreEqual(Color.Blue, weapon.ValidColors.Value[0]);
            Assert.AreEqual(Color.Red, weapon.ValidColors.Value[1]);
        }

        [FlatBuffersTestMethod]
        public void Pack_NullObject_ReturnsDefaultOffset()
        {
            var fbb = CreateBuilder();
            var offset = Weapon.Pack(fbb, null);

            Assert.AreEqual(0, offset.Value);
        }

        [FlatBuffersTestMethod]
        public void ObjectApi_DefaultValuesInConstructor()
        {
            var weaponT = new WeaponT();

            Assert.IsNull(weaponT.Name);
            Assert.AreEqual(10, weaponT.Damage);  // default = 10
            Assert.AreEqual(100.0f, weaponT.Durability, 6);  // default = 100.0
            Assert.AreEqual(false, weaponT.Enchanted);  // default = false
            Assert.AreEqual(Color.Red, weaponT.Rarity);  // default = Red
        }

        [FlatBuffersTestMethod]
        public void ObjectApi_DefaultValuesPreservedAfterRoundTrip()
        {
            var weaponT = new WeaponT
            {
                Name = "DefaultsWeapon",
                // Leave Damage, Durability, Enchanted, Rarity at defaults
                Transform = null,
                FixedStats = null,
                PositionHistory = null
            };

            var fbb = CreateBuilder();
            var offset = Weapon.Pack(fbb, weaponT);
            fbb.Finish(offset.Value);

            var weapon = Weapon.GetRootAsWeapon(fbb.DataBuffer);
            var unpacked = weapon.UnPack();

            Assert.AreEqual("DefaultsWeapon", unpacked.Name);
            Assert.AreEqual(10, unpacked.Damage);
            Assert.AreEqual(100.0f, unpacked.Durability, 6);
            Assert.AreEqual(false, unpacked.Enchanted);
            Assert.AreEqual(Color.Red, unpacked.Rarity);
        }

        [FlatBuffersTestMethod]
        public void RoundTrip_ComplexObject_PreservesAllData()
        {
            var originalWeapon = new WeaponT
            {
                Name = "ComplexWeapon",
                Damage = 999,
                Durability = 50.25f,
                Enchanted = true,
                Rarity = Color.Blue,
                Tags = new List<string> { "tag1", "tag2", "tag3" },
                DamageValues = new List<int> { 100, 200, 300 },
                Modifiers = new List<float> { 1.5f, 2.0f, 2.5f },
                ValidColors = new List<Color> { Color.Red, Color.Green, Color.Blue },
                Transform = null,
                FixedStats = null,
                PositionHistory = null
            };

            var fbb = CreateBuilder(1024);
            var offset = Weapon.Pack(fbb, originalWeapon);
            fbb.Finish(offset.Value);

            var weapon = Weapon.GetRootAsWeapon(fbb.DataBuffer);
            var unpacked = weapon.UnPack();

            Assert.AreEqual(originalWeapon.Name, unpacked.Name);
            Assert.AreEqual(originalWeapon.Damage, unpacked.Damage);
            Assert.AreEqual(originalWeapon.Durability, unpacked.Durability, 6);
            Assert.AreEqual(originalWeapon.Enchanted, unpacked.Enchanted);
            Assert.AreEqual(originalWeapon.Rarity, unpacked.Rarity);

            Assert.AreEqual(originalWeapon.Tags.Count, unpacked.Tags.Count);
            for (int i = 0; i < originalWeapon.Tags.Count; i++)
            {
                Assert.AreEqual(originalWeapon.Tags[i], unpacked.Tags[i]);
            }

            Assert.AreEqual(originalWeapon.DamageValues.Count, unpacked.DamageValues.Count);
            for (int i = 0; i < originalWeapon.DamageValues.Count; i++)
            {
                Assert.AreEqual(originalWeapon.DamageValues[i], unpacked.DamageValues[i]);
            }

            Assert.AreEqual(originalWeapon.Modifiers.Count, unpacked.Modifiers.Count);
            for (int i = 0; i < originalWeapon.Modifiers.Count; i++)
            {
                Assert.AreEqual(originalWeapon.Modifiers[i], unpacked.Modifiers[i], 6);
            }

            Assert.AreEqual(originalWeapon.ValidColors.Count, unpacked.ValidColors.Count);
            for (int i = 0; i < originalWeapon.ValidColors.Count; i++)
            {
                Assert.AreEqual(originalWeapon.ValidColors[i], unpacked.ValidColors[i]);
            }
        }

        [FlatBuffersTestMethod]
        public void RoundTrip_MultiplePackUnpack_DataIntegrity()
        {
            var weapon1 = new WeaponT
            {
                Name = "Weapon1",
                Damage = 100,
                Transform = null,
                FixedStats = null,
                PositionHistory = null
            };

            var fbb1 = CreateBuilder();
            var offset1 = Weapon.Pack(fbb1, weapon1);
            fbb1.Finish(offset1.Value);

            var readWeapon1 = Weapon.GetRootAsWeapon(fbb1.DataBuffer);
            var weapon2 = readWeapon1.UnPack();

            weapon2.Damage = 200;
            weapon2.Transform = null;
            weapon2.FixedStats = null;
            weapon2.PositionHistory = null;

            var fbb2 = CreateBuilder();
            var offset2 = Weapon.Pack(fbb2, weapon2);
            fbb2.Finish(offset2.Value);

            var readWeapon2 = Weapon.GetRootAsWeapon(fbb2.DataBuffer);
            var weapon3 = readWeapon2.UnPack();

            Assert.AreEqual("Weapon1", weapon3.Name);
            Assert.AreEqual(200, weapon3.Damage);
        }

        [FlatBuffersTestMethod]
        public void Player_Pack_WithDefaultValues()
        {
            var playerT = new PlayerT
            {
                Id = 1,
                Name = "TestPlayer"
                // Leave other fields at defaults
            };

            var fbb = CreateBuilder(512);
            var offset = Player.Pack(fbb, playerT);
            fbb.Finish(offset.Value);

            var player = Player.GetRootAsPlayer(fbb.DataBuffer);

            Assert.AreEqual(1, player.Id);
            Assert.AreEqual("TestPlayer", player.Name);
            Assert.AreEqual(1, player.Level);  // default = 1
            Assert.AreEqual(0L, player.Experience);  // default = 0
            Assert.AreEqual(100.0f, player.Health, 6);  // default = 100.0
            Assert.AreEqual(50.0f, player.Mana, 6);  // default = 50.0
            Assert.AreEqual(Status.Pending, player.Status);  // default = Pending
        }

        [FlatBuffersTestMethod]
        public void Player_UnPack_PreservesDefaultValues()
        {
            var playerT = new PlayerT();

            Assert.AreEqual(0, playerT.Id);
            Assert.IsNull(playerT.Name);
            Assert.AreEqual(1, playerT.Level);  // default = 1
            Assert.AreEqual(0L, playerT.Experience);
            Assert.AreEqual(100.0f, playerT.Health, 6);  // default = 100.0
            Assert.AreEqual(50.0f, playerT.Mana, 6);  // default = 50.0
            Assert.AreEqual(Status.Pending, playerT.Status);  // default = Pending
        }

        [FlatBuffersTestMethod]
        public void Player_RoundTrip_WithVectorsAndStats()
        {
            var playerT = new PlayerT
            {
                Id = 42,
                Name = "HeroPlayer",
                Level = 50,
                Experience = 1000000L,
                Health = 500.0f,
                Mana = 300.0f,
                Skills = new List<string> { "Fireball", "Shield", "Heal" },
                Stats = new List<int> { 100, 80, 60, 40, 20 },
                Status = Status.Active
            };

            var fbb = CreateBuilder(1024);
            var offset = Player.Pack(fbb, playerT);
            fbb.Finish(offset.Value);

            var player = Player.GetRootAsPlayer(fbb.DataBuffer);
            var unpacked = player.UnPack();

            Assert.AreEqual(playerT.Id, unpacked.Id);
            Assert.AreEqual(playerT.Name, unpacked.Name);
            Assert.AreEqual(playerT.Level, unpacked.Level);
            Assert.AreEqual(playerT.Experience, unpacked.Experience);
            Assert.AreEqual(playerT.Health, unpacked.Health, 6);
            Assert.AreEqual(playerT.Mana, unpacked.Mana, 6);
            Assert.AreEqual(playerT.Status, unpacked.Status);

            Assert.AreEqual(3, unpacked.Skills.Count);
            Assert.AreEqual("Fireball", unpacked.Skills[0]);
            Assert.AreEqual("Shield", unpacked.Skills[1]);
            Assert.AreEqual("Heal", unpacked.Skills[2]);

            Assert.AreEqual(5, unpacked.Stats.Count);
            Assert.AreEqual(100, unpacked.Stats[0]);
            Assert.AreEqual(20, unpacked.Stats[4]);
        }

        [FlatBuffersTestMethod]
        public void Player_WithStruct_SpawnPoint()
        {
            var playerT = new PlayerT
            {
                Id = 1,
                Name = "SpawnPlayer",
                SpawnPoint = new Vec3T
                {
                    X = 10.0f,
                    Y = 20.0f,
                    Z = 30.0f
                }
            };

            var fbb = CreateBuilder(512);
            var offset = Player.Pack(fbb, playerT);
            fbb.Finish(offset.Value);

            var player = Player.GetRootAsPlayer(fbb.DataBuffer);
            var unpacked = player.UnPack();

            Assert.IsNotNull(unpacked.SpawnPoint);
            Assert.AreEqual(10.0f, unpacked.SpawnPoint.X, 6);
            Assert.AreEqual(20.0f, unpacked.SpawnPoint.Y, 6);
            Assert.AreEqual(30.0f, unpacked.SpawnPoint.Z, 6);
        }

        [FlatBuffersTestMethod]
        public void Player_WithUnion_EquippedWeapon()
        {
            var weaponT = new WeaponT
            {
                Name = "HeroSword",
                Damage = 500,
                Transform = null,
                FixedStats = null,
                PositionHistory = null
            };

            var playerT = new PlayerT
            {
                Id = 1,
                Name = "UnionPlayer",
                EquippedType = new EquipmentUnion
                {
                    Type = Equipment.Weapon,
                    Value = weaponT
                }
            };

            var fbb = CreateBuilder(1024);
            var offset = Player.Pack(fbb, playerT);
            fbb.Finish(offset.Value);

            var player = Player.GetRootAsPlayer(fbb.DataBuffer);
            var unpacked = player.UnPack();

            Assert.IsNotNull(unpacked.EquippedType);
            Assert.AreEqual(Equipment.Weapon, unpacked.EquippedType.Type);
            var equippedWeapon = unpacked.EquippedType.Value as WeaponT;
            Assert.IsNotNull(equippedWeapon);
            Assert.AreEqual("HeroSword", equippedWeapon.Name);
            Assert.AreEqual(500, equippedWeapon.Damage);
        }

        [FlatBuffersTestMethod]
        public void Player_WithVectorOfUnions_Inventory()
        {
            var sword = new WeaponT { Name = "Sword", Damage = 100, Transform = null, FixedStats = null, PositionHistory = null };
            var shield = new ArmorT { Name = "Shield", Defense = 50, Transform = null };
            var bow = new WeaponT { Name = "Bow", Damage = 75, Transform = null, FixedStats = null, PositionHistory = null };

            var playerT = new PlayerT
            {
                Id = 1,
                Name = "InventoryPlayer",
                Inventory = new List<EquipmentUnion>
                {
                    new EquipmentUnion { Type = Equipment.Weapon, Value = sword },
                    new EquipmentUnion { Type = Equipment.Armor, Value = shield },
                    new EquipmentUnion { Type = Equipment.Weapon, Value = bow }
                }
            };

            var fbb = CreateBuilder(2048);
            var offset = Player.Pack(fbb, playerT);
            fbb.Finish(offset.Value);

            var player = Player.GetRootAsPlayer(fbb.DataBuffer);
            var unpacked = player.UnPack();

            Assert.AreEqual(3, unpacked.Inventory.Count);

            Assert.AreEqual(Equipment.Weapon, unpacked.Inventory[0].Type);
            var unpackedSword = unpacked.Inventory[0].Value as WeaponT;
            Assert.IsNotNull(unpackedSword);
            Assert.AreEqual("Sword", unpackedSword.Name);
            Assert.AreEqual(100, unpackedSword.Damage);

            Assert.AreEqual(Equipment.Armor, unpacked.Inventory[1].Type);
            var unpackedShield = unpacked.Inventory[1].Value as ArmorT;
            Assert.IsNotNull(unpackedShield);
            Assert.AreEqual("Shield", unpackedShield.Name);
            Assert.AreEqual(50, unpackedShield.Defense);

            Assert.AreEqual(Equipment.Weapon, unpacked.Inventory[2].Type);
            var unpackedBow = unpacked.Inventory[2].Value as WeaponT;
            Assert.IsNotNull(unpackedBow);
            Assert.AreEqual("Bow", unpackedBow.Name);
            Assert.AreEqual(75, unpackedBow.Damage);
        }

        [FlatBuffersTestMethod]
        public void Pack_NullVectors_HandledCorrectly()
        {
            var weaponT = new WeaponT
            {
                Name = "NullVectorWeapon",
                Tags = null,
                DamageValues = null,
                Modifiers = null,
                Transform = null,
                FixedStats = null,
                PositionHistory = null
            };

            var fbb = CreateBuilder();
            var offset = Weapon.Pack(fbb, weaponT);
            fbb.Finish(offset.Value);

            var weapon = Weapon.GetRootAsWeapon(fbb.DataBuffer);

            Assert.IsFalse(weapon.Tags.HasValue);
            Assert.IsFalse(weapon.DamageValues.HasValue);
            Assert.IsFalse(weapon.Modifiers.HasValue);
        }

        [FlatBuffersTestMethod]
        public void Pack_EmptyVectors_HandledCorrectly()
        {
            var weaponT = new WeaponT
            {
                Name = "EmptyVectorWeapon",
                Tags = new List<string>(),
                DamageValues = new List<int>(),
                Modifiers = new List<float>(),
                Transform = null,
                FixedStats = null,
                PositionHistory = null
            };

            var fbb = CreateBuilder();
            var offset = Weapon.Pack(fbb, weaponT);
            fbb.Finish(offset.Value);

            var weapon = Weapon.GetRootAsWeapon(fbb.DataBuffer);

            var tags = weapon.Tags;
            var tagsLen = tags.HasValue ? tags.Value.Length : 0;
            Assert.AreEqual(0, tagsLen);
            var damageValsLen = weapon.DamageValues.HasValue ? weapon.DamageValues.Value.Length : 0;
            Assert.AreEqual(0, damageValsLen);
            var modifiersLen = weapon.Modifiers.HasValue ? weapon.Modifiers.Value.Length : 0;
            Assert.AreEqual(0, modifiersLen);
        }

        [FlatBuffersTestMethod]
        public void UnPack_MissingVectors_ReturnsEmptyLists()
        {
            var fbb = CreateBuilder();
            var nameOffset = fbb.CreateString("MinimalWeapon");

            Weapon.StartWeapon(fbb);
            Weapon.AddName(fbb, nameOffset);
            var weaponOffset = Weapon.EndWeapon(fbb);
            fbb.Finish(weaponOffset.Value);

            var weapon = Weapon.GetRootAsWeapon(fbb.DataBuffer);
            var weaponT = weapon.UnPack();

            Assert.IsNotNull(weaponT.Tags);
            Assert.AreEqual(0, weaponT.Tags.Count);
            Assert.IsNotNull(weaponT.DamageValues);
            Assert.AreEqual(0, weaponT.DamageValues.Count);
            Assert.IsNotNull(weaponT.Modifiers);
            Assert.AreEqual(0, weaponT.Modifiers.Count);
        }

        [FlatBuffersTestMethod]
        public void Armor_Pack_And_UnPack()
        {
            var armorT = new ArmorT
            {
                Name = "DragonScaleArmor",
                Defense = 150,
                Weight = 25.5f
            };

            var fbb = CreateBuilder();
            var offset = Armor.Pack(fbb, armorT);
            fbb.Finish(offset.Value);

            var armor = Armor.GetRootAsArmor(fbb.DataBuffer);
            var unpacked = armor.UnPack();

            Assert.AreEqual("DragonScaleArmor", unpacked.Name);
            Assert.AreEqual(150, unpacked.Defense);
            Assert.AreEqual(25.5f, unpacked.Weight, 6);
        }

        [FlatBuffersTestMethod]
        public void Armor_DefaultValues()
        {
            var armorT = new ArmorT();

            Assert.IsNull(armorT.Name);
            Assert.AreEqual(5, armorT.Defense);  // default = 5
            Assert.AreEqual(1.0f, armorT.Weight, 6);  // default = 1.0
        }

        [FlatBuffersTestMethod]
        public void Armor_WithTransform()
        {
            var armorT = new ArmorT
            {
                Name = "TransformArmor",
                Defense = 100,
                Transform = new TransformT
                {
                    Position = new Vec3T { X = 1.0f, Y = 2.0f, Z = 3.0f },
                    Scale = new Vec3T { X = 1.0f, Y = 1.0f, Z = 1.0f }
                }
            };

            var fbb = CreateBuilder(512);
            var offset = Armor.Pack(fbb, armorT);
            fbb.Finish(offset.Value);

            var armor = Armor.GetRootAsArmor(fbb.DataBuffer);
            var unpacked = armor.UnPack();

            Assert.AreEqual("TransformArmor", unpacked.Name);
            Assert.AreEqual(100, unpacked.Defense);
            Assert.IsNotNull(unpacked.Transform);
            Assert.AreEqual(1.0f, unpacked.Transform.Position.X, 6);
            Assert.AreEqual(2.0f, unpacked.Transform.Position.Y, 6);
            Assert.AreEqual(3.0f, unpacked.Transform.Position.Z, 6);
        }

        [FlatBuffersTestMethod]
        public void UnPackTo_VectorOfUnions_ReusesExistingObjects()
        {
            var sword1 = new WeaponT { Name = "Sword1", Damage = 100, Transform = null, FixedStats = null, PositionHistory = null };
            var shield1 = new ArmorT { Name = "Shield1", Defense = 50, Transform = null };

            var playerT1 = new PlayerT
            {
                Id = 1,
                Name = "Player1",
                Inventory = new List<EquipmentUnion>
                {
                    new EquipmentUnion { Type = Equipment.Weapon, Value = sword1 },
                    new EquipmentUnion { Type = Equipment.Armor, Value = shield1 }
                }
            };

            var fbb1 = CreateBuilder(2048);
            var offset1 = Player.Pack(fbb1, playerT1);
            fbb1.Finish(offset1.Value);

            var player1 = Player.GetRootAsPlayer(fbb1.DataBuffer);
            var unpacked = player1.UnPack();

            var originalInventory = unpacked.Inventory;
            var originalUnion0 = unpacked.Inventory[0];
            var originalUnion1 = unpacked.Inventory[1];
            var originalSword = unpacked.Inventory[0].Value as WeaponT;
            var originalShield = unpacked.Inventory[1].Value as ArmorT;

            var sword2 = new WeaponT { Name = "Sword2", Damage = 200, Transform = null, FixedStats = null, PositionHistory = null };
            var shield2 = new ArmorT { Name = "Shield2", Defense = 100, Transform = null };

            var playerT2 = new PlayerT
            {
                Id = 2,
                Name = "Player2",
                Inventory = new List<EquipmentUnion>
                {
                    new EquipmentUnion { Type = Equipment.Weapon, Value = sword2 },
                    new EquipmentUnion { Type = Equipment.Armor, Value = shield2 }
                }
            };

            var fbb2 = CreateBuilder(2048);
            var offset2 = Player.Pack(fbb2, playerT2);
            fbb2.Finish(offset2.Value);

            var player2 = Player.GetRootAsPlayer(fbb2.DataBuffer);

            player2.UnPackTo(unpacked);

            Assert.IsTrue(ReferenceEquals(originalInventory, unpacked.Inventory));
            Assert.AreEqual(2, unpacked.Inventory.Count);

            Assert.IsTrue(ReferenceEquals(originalUnion0, unpacked.Inventory[0]));
            Assert.IsTrue(ReferenceEquals(originalUnion1, unpacked.Inventory[1]));

            Assert.IsTrue(ReferenceEquals(originalSword, unpacked.Inventory[0].Value));
            Assert.IsTrue(ReferenceEquals(originalShield, unpacked.Inventory[1].Value));

            var updatedSword = unpacked.Inventory[0].Value as WeaponT;
            Assert.AreEqual("Sword2", updatedSword.Name);
            Assert.AreEqual(200, updatedSword.Damage);

            var updatedShield = unpacked.Inventory[1].Value as ArmorT;
            Assert.AreEqual("Shield2", updatedShield.Name);
            Assert.AreEqual(100, updatedShield.Defense);
        }

        [FlatBuffersTestMethod]
        public void UnPackTo_VectorOfUnions_TypeChange_CreatesNewObjects()
        {
            var sword = new WeaponT { Name = "Sword", Damage = 100, Transform = null, FixedStats = null, PositionHistory = null };
            var shield = new ArmorT { Name = "Shield", Defense = 50, Transform = null };

            var playerT1 = new PlayerT
            {
                Id = 1,
                Name = "Player1",
                Inventory = new List<EquipmentUnion>
                {
                    new EquipmentUnion { Type = Equipment.Weapon, Value = sword },
                    new EquipmentUnion { Type = Equipment.Armor, Value = shield }
                }
            };

            var fbb1 = CreateBuilder(2048);
            var offset1 = Player.Pack(fbb1, playerT1);
            fbb1.Finish(offset1.Value);

            var player1 = Player.GetRootAsPlayer(fbb1.DataBuffer);
            var unpacked = player1.UnPack();

            var originalSword = unpacked.Inventory[0].Value as WeaponT;
            var originalShield = unpacked.Inventory[1].Value as ArmorT;

            var armor2 = new ArmorT { Name = "Armor2", Defense = 75, Transform = null };
            var weapon2 = new WeaponT { Name = "Weapon2", Damage = 150, Transform = null, FixedStats = null, PositionHistory = null };

            var playerT2 = new PlayerT
            {
                Id = 2,
                Name = "Player2",
                Inventory = new List<EquipmentUnion>
                {
                    new EquipmentUnion { Type = Equipment.Armor, Value = armor2 },
                    new EquipmentUnion { Type = Equipment.Weapon, Value = weapon2 }
                }
            };

            var fbb2 = CreateBuilder(2048);
            var offset2 = Player.Pack(fbb2, playerT2);
            fbb2.Finish(offset2.Value);

            var player2 = Player.GetRootAsPlayer(fbb2.DataBuffer);

            player2.UnPackTo(unpacked);

            Assert.AreEqual(Equipment.Armor, unpacked.Inventory[0].Type);
            Assert.AreEqual(Equipment.Weapon, unpacked.Inventory[1].Type);

            Assert.IsFalse(ReferenceEquals(originalSword, unpacked.Inventory[0].Value));
            Assert.IsFalse(ReferenceEquals(originalShield, unpacked.Inventory[1].Value));

            var newArmor = unpacked.Inventory[0].Value as ArmorT;
            Assert.AreEqual("Armor2", newArmor.Name);
            Assert.AreEqual(75, newArmor.Defense);

            var newWeapon = unpacked.Inventory[1].Value as WeaponT;
            Assert.AreEqual("Weapon2", newWeapon.Name);
            Assert.AreEqual(150, newWeapon.Damage);
        }

        [FlatBuffersTestMethod]
        public void UnPackTo_VectorOfUnions_Shrinking_TruncatesList()
        {
            var sword1 = new WeaponT { Name = "Sword1", Damage = 100, Transform = null, FixedStats = null, PositionHistory = null };
            var shield1 = new ArmorT { Name = "Shield1", Defense = 50, Transform = null };
            var bow1 = new WeaponT { Name = "Bow1", Damage = 75, Transform = null, FixedStats = null, PositionHistory = null };

            var playerT1 = new PlayerT
            {
                Id = 1,
                Name = "Player1",
                Inventory = new List<EquipmentUnion>
                {
                    new EquipmentUnion { Type = Equipment.Weapon, Value = sword1 },
                    new EquipmentUnion { Type = Equipment.Armor, Value = shield1 },
                    new EquipmentUnion { Type = Equipment.Weapon, Value = bow1 }
                }
            };

            var fbb1 = CreateBuilder(2048);
            var offset1 = Player.Pack(fbb1, playerT1);
            fbb1.Finish(offset1.Value);

            var player1 = Player.GetRootAsPlayer(fbb1.DataBuffer);
            var unpacked = player1.UnPack();

            var originalList = unpacked.Inventory;
            Assert.AreEqual(3, unpacked.Inventory.Count);

            var sword2 = new WeaponT { Name = "Sword2", Damage = 200, Transform = null, FixedStats = null, PositionHistory = null };

            var playerT2 = new PlayerT
            {
                Id = 2,
                Name = "Player2",
                Inventory = new List<EquipmentUnion>
                {
                    new EquipmentUnion { Type = Equipment.Weapon, Value = sword2 }
                }
            };

            var fbb2 = CreateBuilder(2048);
            var offset2 = Player.Pack(fbb2, playerT2);
            fbb2.Finish(offset2.Value);

            var player2 = Player.GetRootAsPlayer(fbb2.DataBuffer);

            player2.UnPackTo(unpacked);

            Assert.IsTrue(ReferenceEquals(originalList, unpacked.Inventory));
            Assert.AreEqual(1, unpacked.Inventory.Count);

            var updatedSword = unpacked.Inventory[0].Value as WeaponT;
            Assert.AreEqual("Sword2", updatedSword.Name);
            Assert.AreEqual(200, updatedSword.Damage);
        }

        [FlatBuffersTestMethod]
        public void UnPackTo_VectorOfUnions_Growing_AddsNewItems()
        {
            var sword1 = new WeaponT { Name = "Sword1", Damage = 100, Transform = null, FixedStats = null, PositionHistory = null };

            var playerT1 = new PlayerT
            {
                Id = 1,
                Name = "Player1",
                Inventory = new List<EquipmentUnion>
                {
                    new EquipmentUnion { Type = Equipment.Weapon, Value = sword1 }
                }
            };

            var fbb1 = CreateBuilder(2048);
            var offset1 = Player.Pack(fbb1, playerT1);
            fbb1.Finish(offset1.Value);

            var player1 = Player.GetRootAsPlayer(fbb1.DataBuffer);
            var unpacked = player1.UnPack();

            var originalList = unpacked.Inventory;
            var originalUnion0 = unpacked.Inventory[0];
            var originalSword = unpacked.Inventory[0].Value as WeaponT;

            var sword2 = new WeaponT { Name = "Sword2", Damage = 200, Transform = null, FixedStats = null, PositionHistory = null };
            var shield2 = new ArmorT { Name = "Shield2", Defense = 100, Transform = null };
            var bow2 = new WeaponT { Name = "Bow2", Damage = 150, Transform = null, FixedStats = null, PositionHistory = null };

            var playerT2 = new PlayerT
            {
                Id = 2,
                Name = "Player2",
                Inventory = new List<EquipmentUnion>
                {
                    new EquipmentUnion { Type = Equipment.Weapon, Value = sword2 },
                    new EquipmentUnion { Type = Equipment.Armor, Value = shield2 },
                    new EquipmentUnion { Type = Equipment.Weapon, Value = bow2 }
                }
            };

            var fbb2 = CreateBuilder(2048);
            var offset2 = Player.Pack(fbb2, playerT2);
            fbb2.Finish(offset2.Value);

            var player2 = Player.GetRootAsPlayer(fbb2.DataBuffer);

            player2.UnPackTo(unpacked);

            Assert.IsTrue(ReferenceEquals(originalList, unpacked.Inventory));
            Assert.AreEqual(3, unpacked.Inventory.Count);

            Assert.IsTrue(ReferenceEquals(originalUnion0, unpacked.Inventory[0]));
            Assert.IsTrue(ReferenceEquals(originalSword, unpacked.Inventory[0].Value));

            var updatedSword = unpacked.Inventory[0].Value as WeaponT;
            Assert.AreEqual("Sword2", updatedSword.Name);
            Assert.AreEqual(200, updatedSword.Damage);

            Assert.AreEqual(Equipment.Armor, unpacked.Inventory[1].Type);
            var newShield = unpacked.Inventory[1].Value as ArmorT;
            Assert.AreEqual("Shield2", newShield.Name);

            Assert.AreEqual(Equipment.Weapon, unpacked.Inventory[2].Type);
            var newBow = unpacked.Inventory[2].Value as WeaponT;
            Assert.AreEqual("Bow2", newBow.Name);
        }

    }
}
