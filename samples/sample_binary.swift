/*
 * Copyright 2021 Google Inc. All rights reserved.
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

import FlatBuffers

typealias Monster = MyGame_Sample_Monster
typealias Weapon = MyGame_Sample_Weapon
typealias Color = MyGame_Sample_Color
typealias Vec3 = MyGame_Sample_Vec3

func main() {
  let expectedDMG: [Int16] = [3, 5]
  let expectedNames = ["Sword", "Axe"]

  var builder = FlatBufferBuilder(initialSize: 1024)
  let weapon1Name = builder.create(string: expectedNames[0])
  let weapon2Name = builder.create(string: expectedNames[1])

  let weapon1Start = Weapon.startWeapon(&builder)
  Weapon.add(name: weapon1Name, &builder)
  Weapon.add(damage: expectedDMG[0], &builder)
  let sword = Weapon.endWeapon(&builder, start: weapon1Start)
  let weapon2Start = Weapon.startWeapon(&builder)
  Weapon.add(name: weapon2Name, &builder)
  Weapon.add(damage: expectedDMG[1], &builder)
  let axe = Weapon.endWeapon(&builder, start: weapon2Start)

  let name = builder.create(string: "Orc")
  let inventory: [Byte] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
  let inventoryOffset = builder.createVector(inventory)

  let weaponsOffset = builder.createVector(ofOffsets: [sword, axe])

  let orc = Monster.createMonster(
    &builder,
    pos: MyGame_Sample_Vec3(x: 1, y: 2, z: 3),
    hp: 300,
    nameOffset: name,
    inventoryVectorOffset: inventoryOffset,
    color: .red,
    weaponsVectorOffset: weaponsOffset,
    equippedType: .weapon,
    equippedOffset: axe)
  builder.finish(offset: orc)

  let buf = builder.sizedByteArray
  let monster = Monster.getRootAsMonster(bb: ByteBuffer(bytes: buf))

  assert(monster.mana == 150)
  assert(monster.hp == 300)
  assert(monster.name == "Orc")
  assert(monster.color == MyGame.Sample.Color.red)
  assert(monster.pos != nil)
  assert(monster.mutablePos != nil)
  for i in 0..<monster.inventoryCount {
    assert(i == monster.inventory(at: i))
  }

  for i in 0..<monster.weaponsCount {
    let weap = monster.weapons(at: i)
    let index = Int(i)
    assert(weap?.damage == expectedDMG[index])
    assert(weap?.name == expectedNames[index])
  }
  assert(monster.equippedType == .weapon)
  let equipped = monster.equipped(type: Weapon.self)
  assert(equipped?.name == "Axe")
  assert(equipped?.damage == 5)
  print("Monster Object is Verified")
}
