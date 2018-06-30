-- need to update the Lua path to point to the local flatbuffers implementation
package.path = string.format("../lua/?.lua;%s",package.path)
package.path = string.format("./lua/?.lua;%s",package.path)

-- require the library
local flatbuffers = require("flatbuffers")
local binaryArray = require("binaryarray")

-- require the files generated from the schema
local weapon = require("MyGame.Sample.Weapon")

-- get access to the builder, providing an array of size 0
local builder = flatbuffers.Builder(0)

local weaponOne = builder:CreateString("Sword")
local weaponTwo = builder:CreateString("Axe")

weapon.Start(builder)
weapon.AddName(builder, weaponOne)
weapon.AddDamage(builder, 3)
local sword = weapon.End(builder)

builder:Finish(sword)

local buf = builder:Output()

-- print the buffer to console
binaryArray.DumpHex(buf)
