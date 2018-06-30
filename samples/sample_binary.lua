-- need to update the Lua path to point to the local flatbuffers implementation
package.path = string.format("../lua/?.lua;%s",package.path)
package.path = string.format("./lua/?.lua;%s",package.path)

-- require the library
local flatbuffers = require("flatbuffers")

local binaryArray = require("binaryarray") -- for hex dump utility

-- require the files generated from the schema
local weapon = require("MyGame.Sample.Weapon")
local monster = require("MyGame.Sample.Monster")
local vec3 = require("MyGame.Sample.Vec3")
local color = require("MyGame.Sample.Color")
local equipment = require("MyGame.Sample.Equipment")

-- get access to the builder, providing an array of size 0
local builder = flatbuffers.Builder(20)

local weaponOne = builder:CreateString("Sword")
local weaponTwo = builder:CreateString("Axe")

weapon.Start(builder)
weapon.AddName(builder, weaponOne)
weapon.AddDamage(builder, 3)
local sword = weapon.End(builder)

weapon.Start(builder)
weapon.AddName(builder, weaponTwo)
weapon.AddDamage(builder, 5)
local axe = weapon.End(builder)

local name = builder:CreateString("Orc")

monster.StartInventoryVector(builder, 10)
for i=10,1,-1 do
    builder:PrependByte(i)
end
local inv = builder:EndVector(10)

monster.StartWeaponsVector(builder, 2)
builder:PrependUOffsetTRelative(axe)
builder:PrependUOffsetTRelative(sword)
local weapons = builder:EndVector(2)

local pos = vec3.CreateVec3(builder, 1.0, 2.0, 3.0)

monster.Start(builder)
monster.AddPos(builder, pos)
monster.AddHp(builder, 300)
monster.AddName(builder, name)
monster.AddInventory(builder, inv)
monster.AddColor(builder, color.Red)
monster.AddWeapons(builder, weapons)
monster.AddEquippedType(builder, equipment.Weapon)
monster.AddEquipped(builder, axe)
local orc = monster.End(builder)

builder:Finish(orc)

local buf = builder:Output()

-- print the buffer to console
binaryArray.DumpHex(buf)

local mon = monster.GetRootAsMonster(buf, 0)
assert(mon:Mana() == 150)