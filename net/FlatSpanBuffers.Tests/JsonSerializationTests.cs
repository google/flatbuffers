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
using JsonTest;

namespace Google.FlatSpanBuffers.Tests
{
    [FlatBuffersTestClass]
    public class JsonSerializationTests
    {
        [FlatBuffersTestMethod]
        public void BasicJsonRoundTrip()
        {
            var gameState = new GameStateT
            {
                Version = "1.0.0",
                ActivePlayerId = 42,
                Timestamp = 1234567890,
                Players = new List<PlayerT>
                {
                    new PlayerT
                    {
                        Id = 1,
                        Name = "Player1",
                        Level = 10,
                        Health = 95.5f,
                        Status = Status.Active,
                    }
                }
            };

            var json = gameState.SerializeToJson();

            Assert.IsTrue(json.Contains("\"version\": \"1.0.0\""));
            Assert.IsTrue(json.Contains("\"active_player_id\": 42"));
            Assert.IsTrue(json.Contains("\"Player1\""));

            var deserialized = GameStateT.DeserializeFromJson(json);

            Assert.AreEqual(gameState.Version, deserialized.Version);
            Assert.AreEqual(gameState.ActivePlayerId, deserialized.ActivePlayerId);
            Assert.AreEqual(gameState.Timestamp, deserialized.Timestamp);
            Assert.AreEqual(gameState.Players.Count, deserialized.Players.Count);
            Assert.AreEqual(gameState.Players[0].Name, deserialized.Players[0].Name);
            Assert.AreEqual(gameState.Players[0].Level, deserialized.Players[0].Level);
            Assert.AreEqual(gameState.Players[0].Health, deserialized.Players[0].Health, 6);
        }

        [FlatBuffersTestMethod]
        public void EnumSerializesAsString()
        {
            var player = new PlayerT
            {
                Id = 1,
                Name = "TestPlayer",
                Status = Status.Suspended,
            };

            var gameState = new GameStateT
            {
                Version = "1.0",
                Players = new List<PlayerT> { player }
            };

            var json = gameState.SerializeToJson();

            // Enums should serialize as strings due to JsonStringEnumConverter
            Assert.IsTrue(json.Contains("\"status\": \"Suspended\""));

            var deserialized = GameStateT.DeserializeFromJson(json);
            Assert.AreEqual(Status.Suspended, deserialized.Players[0].Status);
        }

        [FlatBuffersTestMethod]
        public void BitFlagsEnumSerializesCorrectly()
        {
            var player = new PlayerT
            {
                Id = 1,
                Name = "FlagPlayer",
                Priorities = Priority.High | Priority.Critical,
            };

            var gameState = new GameStateT
            {
                Version = "1.0",
                Players = new List<PlayerT> { player }
            };

            var json = gameState.SerializeToJson();

            var deserialized = GameStateT.DeserializeFromJson(json);
            Assert.AreEqual(player.Priorities, deserialized.Players[0].Priorities);
        }

        [FlatBuffersTestMethod]
        public void StructSerializesCorrectly()
        {
            var player = new PlayerT
            {
                Id = 1,
                Name = "StructPlayer",
                Position = new Vec2T { X = 10.5f, Y = 20.5f },
                Color = new ColorT { R = 255, G = 128, B = 64, A = 255 },
            };

            var gameState = new GameStateT
            {
                Version = "1.0",
                Players = new List<PlayerT> { player }
            };

            var json = gameState.SerializeToJson();

            Assert.IsTrue(json.Contains("\"position\":"));
            Assert.IsTrue(json.Contains("\"x\":"));
            Assert.IsTrue(json.Contains("\"color\":"));

            var deserialized = GameStateT.DeserializeFromJson(json);
            Assert.AreEqual(player.Position.X, deserialized.Players[0].Position.X, 6);
            Assert.AreEqual(player.Position.Y, deserialized.Players[0].Position.Y, 6);
            Assert.AreEqual(player.Color.R, deserialized.Players[0].Color.R);
            Assert.AreEqual(player.Color.G, deserialized.Players[0].Color.G);
            Assert.AreEqual(player.Color.B, deserialized.Players[0].Color.B);
        }

        [FlatBuffersTestMethod]
        public void VectorOfScalarsSerializesCorrectly()
        {
            var player = new PlayerT
            {
                Id = 1,
                Name = "VectorPlayer",
                Scores = new List<int> { 100, 200, 300, 400 },
                Tags = new List<string> { "tag1", "tag2", "tag3" },
            };

            var gameState = new GameStateT
            {
                Version = "1.0",
                Players = new List<PlayerT> { player }
            };

            var json = gameState.SerializeToJson();

            var deserialized = GameStateT.DeserializeFromJson(json);
            Assert.AreEqual(player.Scores.Count, deserialized.Players[0].Scores.Count);
            for (int i = 0; i < player.Scores.Count; i++)
            {
                Assert.AreEqual(player.Scores[i], deserialized.Players[0].Scores[i]);
            }
            Assert.AreEqual(player.Tags.Count, deserialized.Players[0].Tags.Count);
            for (int i = 0; i < player.Tags.Count; i++)
            {
                Assert.AreEqual(player.Tags[i], deserialized.Players[0].Tags[i]);
            }
        }

        [FlatBuffersTestMethod]
        public void VectorOfTablesSerializesCorrectly()
        {
            var player = new PlayerT
            {
                Id = 1,
                Name = "InventoryPlayer",
                Inventory = new List<ItemT>
                {
                    new ItemT { Id = 1, Name = "Sword", Value = 100.0f },
                    new ItemT { Id = 2, Name = "Shield", Value = 50.0f },
                    new ItemT { Id = 3, Name = "Potion", Value = 25.0f },
                },
            };

            var gameState = new GameStateT
            {
                Version = "1.0",
                Players = new List<PlayerT> { player }
            };

            var json = gameState.SerializeToJson();

            var deserialized = GameStateT.DeserializeFromJson(json);
            Assert.AreEqual(player.Inventory.Count, deserialized.Players[0].Inventory.Count);
            for (int i = 0; i < player.Inventory.Count; i++)
            {
                Assert.AreEqual(player.Inventory[i].Id, deserialized.Players[0].Inventory[i].Id);
                Assert.AreEqual(player.Inventory[i].Name, deserialized.Players[0].Inventory[i].Name);
                Assert.AreEqual(player.Inventory[i].Value, deserialized.Players[0].Inventory[i].Value, 6);
            }
        }

        [FlatBuffersTestMethod]
        public void UnionSerializesCorrectly()
        {
            // Note: Union deserialization is a known limitation with System.Text.Json
            // The union converter requires type context which is not available during deserialization.
            // This test verifies serialization works correctly.
            
            var weaponPlayer = new PlayerT
            {
                Id = 1,
                Name = "WeaponPlayer",
                Equipped = new EquipmentUnion { Type = Equipment.Weapon, Value = new WeaponT { Name = "Excalibur", Damage = 999 } },
            };

            var armorPlayer = new PlayerT
            {
                Id = 2,
                Name = "ArmorPlayer",
                Equipped = new EquipmentUnion { Type = Equipment.Armor, Value = new ArmorT { Name = "DragonScale", Defense = 500 } },
            };

            var gameState = new GameStateT
            {
                Version = "1.0",
                Players = new List<PlayerT> { weaponPlayer, armorPlayer }
            };

            var json = gameState.SerializeToJson();

            // Verify union values are serialized to JSON
            Assert.IsTrue(json.Contains("\"equipped\":"));
            Assert.IsTrue(json.Contains("\"Excalibur\""));
            Assert.IsTrue(json.Contains("\"DragonScale\""));
            Assert.IsTrue(json.Contains("\"damage\": 999"));
            Assert.IsTrue(json.Contains("\"defense\": 500"));
        }

        [FlatBuffersTestMethod]
        public void NullValuesHandledCorrectly()
        {
            var player = new PlayerT
            {
                Id = 1,
                Name = "NullPlayer",
                // Inventory, Tags, Scores, Position, etc. are null
            };

            var gameState = new GameStateT
            {
                Version = "1.0",
                Players = new List<PlayerT> { player }
            };

            var json = gameState.SerializeToJson();
            var deserialized = GameStateT.DeserializeFromJson(json);

            Assert.AreEqual("NullPlayer", deserialized.Players[0].Name);
            Assert.IsNull(deserialized.Players[0].Inventory);
            Assert.IsNull(deserialized.Players[0].Tags);
            Assert.IsNull(deserialized.Players[0].Scores);
        }

        [FlatBuffersTestMethod]
        public void JsonToBinaryRoundTrip()
        {
            var gameState = new GameStateT
            {
                Version = "2.0.0",
                ActivePlayerId = 100,
                Timestamp = 9999999999,
                Players = new List<PlayerT>
                {
                    new PlayerT
                    {
                        Id = 1,
                        Name = "BinaryPlayer",
                        Level = 50,
                        Health = 100.0f,
                        Status = Status.Active,
                        Position = new Vec2T { X = 1.0f, Y = 2.0f },
                    }
                }
            };

            // JSON round trip
            var json = gameState.SerializeToJson();
            var fromJson = GameStateT.DeserializeFromJson(json);

            // Verify JSON round trip
            Assert.AreEqual(gameState.Version, fromJson.Version);
            Assert.AreEqual(gameState.Players[0].Name, fromJson.Players[0].Name);

            // Binary round trip
            var binary = fromJson.SerializeToBinary();
            var fromBinary = GameStateT.DeserializeFromBinary(binary);

            // Verify final result
            Assert.AreEqual(gameState.Version, fromBinary.Version);
            Assert.AreEqual(gameState.ActivePlayerId, fromBinary.ActivePlayerId);
            Assert.AreEqual(gameState.Timestamp, fromBinary.Timestamp);
            Assert.AreEqual(gameState.Players[0].Name, fromBinary.Players[0].Name);
            Assert.AreEqual(gameState.Players[0].Level, fromBinary.Players[0].Level);
            Assert.AreEqual(gameState.Players[0].Health, fromBinary.Players[0].Health, 6);
        }

        [FlatBuffersTestMethod]
        public void MultiplePlayersSerializeCorrectly()
        {
            var gameState = new GameStateT
            {
                Version = "3.0",
                ActivePlayerId = 2,
                Players = new List<PlayerT>
                {
                    new PlayerT { Id = 1, Name = "Alice", Level = 10 },
                    new PlayerT { Id = 2, Name = "Bob", Level = 20 },
                    new PlayerT { Id = 3, Name = "Charlie", Level = 30 },
                }
            };

            var json = gameState.SerializeToJson();
            var deserialized = GameStateT.DeserializeFromJson(json);

            Assert.AreEqual(3, deserialized.Players.Count);
            Assert.AreEqual("Alice", deserialized.Players[0].Name);
            Assert.AreEqual("Bob", deserialized.Players[1].Name);
            Assert.AreEqual("Charlie", deserialized.Players[2].Name);
            Assert.AreEqual(10, deserialized.Players[0].Level);
            Assert.AreEqual(20, deserialized.Players[1].Level);
            Assert.AreEqual(30, deserialized.Players[2].Level);
        }
    }
}
