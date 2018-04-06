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

import '../dart/lib/flat_buffers.dart' as fb;
import './monster_my_game.sample_generated.dart' as myGame;

// Example how to use FlatBuffers to create and read binary buffers.

void main() {
  // Create the builder here so we can use it for both weapons and equipped
  // the actual data will only be written to the buffer once.
  var axe = new myGame.WeaponBuilder(name: 'Axe', damage: 5);

  var monsterBuilder = new myGame.MonsterBuilder(
    pos: new myGame.Vec3Builder(x: 1.0,y: 2.0, z: 3.0),
    mana: 150,
    hp: 80,
    name: 'MyMonster',
    inventory: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9],
    color: myGame.Color.Red,
    weapons: [new myGame.WeaponBuilder(name: 'Sword', damage: 3), axe],
    equippedType: myGame.EquipmentTypeId.Weapon,
    equipped: axe,
  );

  var buffer = monsterBuilder.toBytes();

  // We now have a FlatBuffer we can store on disk or send over a network.

  // ** file/network code goes here :) **

  // Instead, we're going to access it right away (as if we just received it).

  // Get access to the root:

  var monster = new myGame.Monster(buffer);

  // Get and test some scalar types from the FlatBuffer.
  assert(monster.hp == 80);
  assert(monster.mana == 150);  // default
  assert(monster.name == "MyMonster");

  // Get and test a field of the FlatBuffer's `struct`.
  var pos = monster.pos;
  assert(pos != null);
  assert(pos.z == 3.0);

  // Get a test an element from the `inventory` FlatBuffer's `vector`.
  var inv = monster.inventory;
  assert(inv != null);
  assert(inv.length == 10);
  assert(inv[9] == 9);

  // Get and test the `weapons` FlatBuffers's `vector`.
  var expected_weapon_names = ["Sword", "Axe"];
  var expected_weapon_damages = [ 3, 5 ];
  var weps = monster.weapons;
  for (int i = 0; i < weps.length; i++) {
    assert(weps[i].name == expected_weapon_names[i]);
    assert(weps[i].damage == expected_weapon_damages[i]);
  }

  // Get and test the `Equipment` union (`equipped` field).
  assert(monster.equippedType.value == myGame.EquipmentTypeId.Weapon.value);
  assert(monster.equippedType == myGame.EquipmentTypeId.Weapon);

  assert(monster.equipped is myGame.Weapon);
  var equipped = monster.equipped as myGame.Weapon;
  assert(equipped.name == "Axe");
  assert(equipped.damage == 5);


  print("The FlatBuffer was successfully created and verified!");
}
