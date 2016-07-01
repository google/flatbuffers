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

// To run, use the `csharp_sample.sh` script.

using System;
using FlatBuffers;
using MyGame.Sample;

class SampleBinary
{
  // Example how to use FlatBuffers to create and read binary buffers.
  static void Main()
  {
    var builder = new FlatBufferBuilder(1);

    // Create some weapons for our Monster ('Sword' and 'Axe').
    var weapon1Name = builder.CreateString("Sword");
    var weapon1Damage = 3;
    var weapon2Name = builder.CreateString("Axe");
    var weapon2Damage = 5;

    // Use the `CreateWeapon()` helper function to create the weapons, since we set every field.
    var weaps = new Offset<Weapon>[2];
    weaps[0] = Weapon.CreateWeapon(builder, weapon1Name, (short)weapon1Damage);
    weaps[1] = Weapon.CreateWeapon(builder, weapon2Name, (short)weapon2Damage);

    // Serialize the FlatBuffer data.
    var name = builder.CreateString("Orc");
    var treasure =  new byte[] {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    var inv = Monster.CreateInventoryVector(builder, treasure);
    var weapons = Monster.CreateWeaponsVector(builder, weaps);
    var pos = Vec3.CreateVec3(builder, 1.0f, 2.0f, 3.0f);

    Monster.StartMonster(builder);
    Monster.AddPos(builder, pos);
    Monster.AddHp(builder, (short)300);
    Monster.AddName(builder, name);
    Monster.AddInventory(builder, inv);
    Monster.AddColor(builder, Color.Red);
    Monster.AddWeapons(builder, weapons);
    Monster.AddEquippedType(builder, Equipment.Weapon);
    Monster.AddEquipped(builder, weaps[1].Value);
    var orc = Monster.EndMonster(builder);

    builder.Finish(orc.Value); // You could also call `Monster.FinishMonsterBuffer(builder, orc);`.

    // We now have a FlatBuffer that we could store on disk or send over a network.

    // ...Code to store to disk or send over a network goes here...

    // Instead, we are going to access it right away, as if we just received it.

    var buf = builder.DataBuffer;

    // Get access to the root:
    var monster = Monster.GetRootAsMonster(buf);

    // For C#, unlike other languages, most values (except for vectors and unions) are available as
    // properties instead of accessor methods.

    // Note: We did not set the `Mana` field explicitly, so we get back the default value.
    Assert(monster.Mana == 150, "monster.Mana", Convert.ToString(monster.Mana),
           Convert.ToString(150));
    Assert(monster.Hp == 300, "monster.Hp", Convert.ToString(monster.Hp), Convert.ToString(30));
    Assert(monster.Name.Equals("Orc", StringComparison.Ordinal), "monster.Name", monster.Name,
           "Orc");
    Assert(monster.Color == Color.Red, "monster.Color", Convert.ToString(monster.Color),
           Convert.ToString(Color.Red));

    // C# also allows you to use performance-enhanced methods to fill an object that has already
    // been created. These functions are prefixed with "Get". For example: `monster.GetPos()`.
    var myAlreadyCreatedVector = new Vec3();
    monster.GetPos(myAlreadyCreatedVector); // Instead of `var myNewVec3 = monster.Pos`.
    Assert(myAlreadyCreatedVector.X == 1.0f, "myAlreadyCreatedVector.X",
           Convert.ToString(myAlreadyCreatedVector.X), Convert.ToString(1.0f));
    Assert(myAlreadyCreatedVector.Y == 2.0f, "myAlreadyCreatedVector.Y",
           Convert.ToString(myAlreadyCreatedVector.Y), Convert.ToString(2.0f));
    Assert(myAlreadyCreatedVector.Z == 3.0f, "myAlreadyCreatedVector.Z",
           Convert.ToString(myAlreadyCreatedVector.Z), Convert.ToString(3.0f));

    // Get and test the `Inventory` FlatBuffer `vector`.
    for (int i = 0; i < monster.InventoryLength; i++)
    {
      Assert(monster.GetInventory(i) == i, "monster.GetInventory",
             Convert.ToString(monster.GetInventory(i)), Convert.ToString(i));
    }

    // Get and test the `Weapons` FlatBuffer `vector` of `table`s.
    var expectedWeaponNames = new string[] {"Sword", "Axe"};
    var expectedWeaponDamages = new short[] {3, 5};
    for (int i = 0; i < monster.WeaponsLength; i++)
    {
      Assert(monster.GetWeapons(i).Name.Equals(expectedWeaponNames[i], StringComparison.Ordinal),
             "monster.GetWeapons", monster.GetWeapons(i).Name, expectedWeaponNames[i]);
      Assert(monster.GetWeapons(i).Damage == expectedWeaponDamages[i], "monster.GetWeapons",
             Convert.ToString(monster.GetWeapons(i).Damage),
             Convert.ToString(expectedWeaponDamages[i]));
    }

    // Get and test the `Equipped` FlatBuffer `union`.
    Assert(monster.EquippedType == Equipment.Weapon, "monster.EquippedType",
           Convert.ToString(monster.EquippedType), Convert.ToString(Equipment.Weapon));
    var equipped = (Weapon)monster.GetEquipped(new Weapon());
    Assert(equipped.Name.Equals("Axe", StringComparison.Ordinal), "equipped.Name", equipped.Name,
           "Axe");
    Assert(equipped.Damage == 5, "equipped.Damage", Convert.ToString(equipped.Damage),
           Convert.ToString(5));

    Console.WriteLine("The FlatBuffer was successfully created and verified!");
  }

  // A helper function to handle assertions.
  static void Assert(bool assertPassed, string codeExecuted, string actualValue,
                     string expectedValue)
  {
    if (assertPassed == false)
    {
      Console.WriteLine("Assert failed! " + codeExecuted + " (" + actualValue +
          ") was not equal to " + expectedValue + ".");
      System.Environment.Exit(1);
    }
  }
}
