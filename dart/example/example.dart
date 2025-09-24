/*
 * Copyright 2018 Dan Field. All rights reserved.
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

import 'package:flat_buffers/flat_buffers.dart' as fb;

import './monster_my_game.sample_generated.dart' as my_game;

// Example how to use FlatBuffers to create and read binary buffers.

void main() {
  builderTest();
  objectBuilderTest();
}

void builderTest() {
  final builder = fb.Builder(initialSize: 1024);
  final int? weaponOneName = builder.writeString("Sword");
  final int weaponOneDamage = 3;

  final int? weaponTwoName = builder.writeString("Axe");
  final int weaponTwoDamage = 5;

  final swordBuilder = my_game.WeaponBuilder(builder)
    ..begin()
    ..addNameOffset(weaponOneName)
    ..addDamage(weaponOneDamage);
  final int sword = swordBuilder.finish();

  final axeBuilder = my_game.WeaponBuilder(builder)
    ..begin()
    ..addNameOffset(weaponTwoName)
    ..addDamage(weaponTwoDamage);
  final int axe = axeBuilder.finish();

  // Serialize a name for our monster, called "Orc".
  final int? name = builder.writeString('Orc');

  // Create a list representing the inventory of the Orc. Each number
  // could correspond to an item that can be claimed after he is slain.
  final List<int> treasure = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];
  final inventory = builder.writeListUint8(treasure);
  final weapons = builder.writeList([sword, axe]);

  // Struct builders are very easy to reuse.
  final vec3Builder = my_game.Vec3Builder(builder);

  vec3Builder.finish(4.0, 5.0, 6.0);
  vec3Builder.finish(1.0, 2.0, 3.0);
  // Set his hit points to 300 and his mana to 150.
  final int hp = 300;
  final int mana = 150;

  final monster = my_game.MonsterBuilder(builder)
    ..begin()
    ..addNameOffset(name)
    ..addInventoryOffset(inventory)
    ..addWeaponsOffset(weapons)
    ..addEquippedType(my_game.EquipmentTypeId.Weapon)
    ..addEquippedOffset(axe)
    ..addHp(hp)
    ..addMana(mana)
    ..addPos(vec3Builder.finish(1.0, 2.0, 3.0))
    ..addColor(my_game.Color.Red);

  final int monsteroff = monster.finish();
  builder.finish(monsteroff);
  if (verify(builder.buffer)) {
    print(
      "The FlatBuffer was successfully created with a builder and verified!",
    );
  }
}

void objectBuilderTest() {
  // Create the builder here so we can use it for both weapons and equipped
  // the actual data will only be written to the buffer once.
  var axe = my_game.WeaponObjectBuilder(name: 'Axe', damage: 5);

  var monsterBuilder = my_game.MonsterObjectBuilder(
    pos: my_game.Vec3ObjectBuilder(x: 1.0, y: 2.0, z: 3.0),
    mana: 150,
    hp: 300,
    name: 'Orc',
    inventory: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9],
    color: my_game.Color.Red,
    weapons: [
      my_game.WeaponObjectBuilder(name: 'Sword', damage: 3),
      axe,
    ],
    equippedType: my_game.EquipmentTypeId.Weapon,
    equipped: axe,
  );

  var buffer = monsterBuilder.toBytes();

  // We now have a FlatBuffer we can store on disk or send over a network.

  // ** file/network code goes here :) **

  // Instead, we're going to access it right away (as if we just received it).
  if (verify(buffer)) {
    print(
      "The FlatBuffer was successfully created with an object builder and verified!",
    );
  }
}

bool verify(List<int> buffer) {
  // Get access to the root:
  var monster = my_game.Monster(buffer);

  // Get and test some scalar types from the FlatBuffer.
  assert(monster.hp == 80);
  assert(monster.mana == 150); // default
  assert(monster.name == "MyMonster");

  // Get and test a field of the FlatBuffer's `struct`.
  var pos = monster.pos!;
  assert(pos.z == 3.0);

  // Get a test an element from the `inventory` FlatBuffer's `vector`.
  var inv = monster.inventory!;
  assert(inv.length == 10);
  assert(inv[9] == 9);

  // Get and test the `weapons` FlatBuffers's `vector`.
  var expectedWeaponNames = ["Sword", "Axe"];
  var expectedWeaponDamages = [3, 5];
  var weps = monster.weapons!;
  for (int i = 0; i < weps.length; i++) {
    assert(weps[i].name == expectedWeaponNames[i]);
    assert(weps[i].damage == expectedWeaponDamages[i]);
  }

  // Get and test the `Equipment` union (`equipped` field).
  assert(monster.equippedType!.value == my_game.EquipmentTypeId.Weapon.value);
  assert(monster.equippedType == my_game.EquipmentTypeId.Weapon);

  assert(monster.equipped is my_game.Weapon);
  var equipped = monster.equipped as my_game.Weapon;
  assert(equipped.name == "Axe");
  assert(equipped.damage == 5);

  print(monster);
  return true;
}
