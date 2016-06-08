/*
 * Copyright 2016 Google Inc. All rights reserved.
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

// Run this file with the `dlang_sample.sh` script.

import std.stdio;
import MyGame.Sample;
import std.conv;

// Example how to use FlatBuffers to create and read binary buffers.
void main()
{
    auto builder = new FlatBufferBuilder(512);

    // Create a string for our Monster as name.
    auto name = builder.createString("MyMonster");

    // Create some weapons for our Monster 
    int[] weaps;
    foreach (i; 0 .. 3)
    {
        weaps ~= Weapon.createWeapon(builder,
            builder.createString("Weapon." ~ i.to!string()), cast(short) i); // Use the `createWeapon()` helper function to create the weapons, since we set every field.
    }
    int t = Monster.createWeaponsVector(builder, weaps);

    // Serialize the FlatBuffer data.
    ubyte[] invData = cast(ubyte[])("MyMonster");
    auto inventory = Monster.createInventoryVector(builder, invData);
    // Create monster:
    Monster.startMonster(builder);
    Monster.addPos(builder, Vec3.createVec3(builder, 1, 2, 3));
    Monster.addMana(builder, 150);
    Monster.addHp(builder, 80);
    Monster.addName(builder, name);
    Monster.addInventory(builder, inventory);
    Monster.addColor(builder, Color.Blue);
    Monster.addWeapons(builder, t);
    auto mloc = Monster.endMonster(builder);

    builder.finish(mloc);
    // We now have a FlatBuffer we can store or send somewhere.

    //** file/network code goes here :) **
    // access builder.sizedByteArray() for builder.sizedByteArray().length bytes

    // Instead, we're going to access it straight away.
    // Get access to the root:
    auto data = builder.sizedByteArray();

    // Get access to the root:
    auto monster = Monster.getRootAsMonster(new ByteBuffer(data));

    assert(monster.hp == 80);
    assert(monster.mana == 150); // default
    assert(monster.name == "MyMonster");

    assert(monster.color == Color.Blue);

    auto pos = monster.pos();
    assert(!pos.isNull());
    assert(pos.z == 3);

    auto hh = monster.inventory();
    assert(hh.length == 9);
    assert(hh[2] == 'M');

    // Get and test the `Weapons` FlatBuffer `vector` of `table`s.
    auto weapons = monster.weapons();
    assert(weapons.length == 3);
    Nullable!Weapon weap = weapons[1];
    assert(!weap.isNull());
    assert(weap.damage == 1);
    assert(weap.name == "Weapon.1");
    writeln("deserialized data!");
}
