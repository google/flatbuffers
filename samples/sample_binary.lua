-- need to update the Lua path to point to the local flatbuffers implementation
package.path = string.format("../lua/?.lua;%s",package.path)

-- require the library
local flatbuffers = require("flatbuffers")

-- get access to the builder, providing an array of size 0
local builder = flatbuffers.Builder(0)

local weaponOne = builder.CreateString("Sword")
local weaponTwo = builder.CreateString("Axe")

