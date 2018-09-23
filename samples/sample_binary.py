#!/usr/bin/python
# Copyright 2015 Google Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# To run this file, use `python_sample.sh`.

# Append paths to the `flatbuffers` and `MyGame` modules. This is necessary
# to facilitate executing this script in the `samples` folder, and to root
# folder (where it gets placed when using `cmake`).
import os
import sys

sys.path.append(os.path.join(os.path.dirname(__file__), '../python'))

import flatbuffers
import monster_generated


# Example of how to use FlatBuffers to create and read binary buffers.

def main():
    builder = flatbuffers.Builder(1024)

    # Create some weapons for our Monster ('Sword' and 'Axe').
    weapon_one = builder.CreateString('Sword')
    weapon_two = builder.CreateString('Axe')

    weapon_builder = monster_generated.WeaponBuilder(builder)

    weapon_builder.add_name(weapon_one)
    weapon_builder.add_damage(3)
    sword = weapon_builder.finish()

    weapon_builder = monster_generated.WeaponBuilder(builder)
    weapon_builder.add_name(weapon_two)
    weapon_builder.add_damage(5)
    axe = weapon_builder.finish()

    # Serialize the FlatBuffer data.
    name = builder.CreateString('Orc')

    monster_generated.MonsterBuilder.start_inventory(builder, 10)
    # Note: Since we prepend the bytes, this loop iterates in reverse order.
    for i in reversed(range(0, 10)):
        builder.PrependByte(i)
    inv = monster_generated.MonsterBuilder.end_inventory(builder, 10)

    monster_generated.MonsterBuilder.start_weapons(builder, 2)
    # Note: Since we prepend the data, prepend the weapons in reverse order.
    builder.PrependUOffsetTRelative(axe)
    builder.PrependUOffsetTRelative(sword)
    weapons = monster_generated.MonsterBuilder.end_weapons(builder, 2)

    monster_builder = monster_generated.MonsterBuilder(builder)
    monster_builder.add_pos(1.0, 2.0, 3.0)
    monster_builder.add_hp(300)
    monster_builder.add_name(name)
    monster_builder.add_inventory(inv)
    monster_builder.add_color(monster_generated.Color.Red)
    monster_builder.add_weapons(weapons)
    monster_builder.add_equipped_type(monster_generated.Equipment.Weapon)
    monster_builder.add_equipped(axe)
    orc = monster_builder.finish()

    builder.Finish(orc)

    # We now have a FlatBuffer that we could store on disk or send over a network.

    # ...Saving to file or sending over a network code goes here...

    # Instead, we are going to access this buffer right away (as if we just
    # received it).

    buf = builder.Output()

    # Note: We use `0` for the offset here, since we got the data using the
    # `builder.Output()` method. This simulates the data you would store/receive
    # in your FlatBuffer. If you wanted to read from the `builder.Bytes` directly,
    # you would need to pass in the offset of `builder.Head()`, as the builder
    # actually constructs the buffer backwards.
    monster = monster_generated.Monster.from_root(buf)

    # Note: We did not set the `Mana` field explicitly, so we get a default value.
    assert monster.mana == 150
    assert monster.hp == 300
    assert monster.name == 'Orc'
    assert monster.color == monster_generated.Color.Red
    assert monster.pos.x == 1.0
    assert monster.pos.z == 3.0
    assert monster.pos.y == 2.0

    # Get and test the `inventory` FlatBuffer `vector`.
    for i in xrange(monster.inventory_length):
        assert monster.inventory_item(i) == i

    # Get and test the `weapons` FlatBuffer `vector` of `table`s.
    expected_weapon_names = ['Sword', 'Axe']
    expected_weapon_damages = [3, 5]
    for i in xrange(monster.weapons_length):
        assert monster.weapons_item(i).name == expected_weapon_names[i]
        assert monster.weapons_item(i).damage == expected_weapon_damages[i]

    # Get and test the `equipped` FlatBuffer `union`.
    assert monster.equipped_type == monster_generated.Equipment.Weapon

    assert isinstance(monster.equipped, monster_generated.Weapon)
    assert monster.equipped.name == "Axe"
    assert monster.equipped.damage == 5

    print 'The FlatBuffer was successfully created and verified!'


if __name__ == '__main__':
    main()
