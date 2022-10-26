--[[ MyGame.Example.Vec3

  Automatically generated by the FlatBuffers compiler, do not modify.
  Or modify. I'm a message, not a cop.

  flatc version: 22.10.25

  Declared by  : //monster_test.fbs
  Rooting type : MyGame.Example.Monster (//monster_test.fbs)

--]]

local flatbuffers = require('flatbuffers')

local Vec3 = {}
local mt = {}

function Vec3.New()
  local o = {}
  setmetatable(o, {__index = mt})
  return o
end

function mt:Init(buf, pos)
  self.view = flatbuffers.view.New(buf, pos)
end

function mt:X()
  return self.view:Get(flatbuffers.N.Float32, self.view.pos + 0)
end

function mt:Y()
  return self.view:Get(flatbuffers.N.Float32, self.view.pos + 4)
end

function mt:Z()
  return self.view:Get(flatbuffers.N.Float32, self.view.pos + 8)
end

function mt:Test1()
  return self.view:Get(flatbuffers.N.Float64, self.view.pos + 16)
end

function mt:Test2()
  return self.view:Get(flatbuffers.N.Uint8, self.view.pos + 24)
end

function mt:Test3(obj)
  obj:Init(self.view.bytes, self.view.pos + 26)
  return obj
end

function Vec3.CreateVec3(builder, x, y, z, test1, test2, test3_a, test3_b)
  builder:Prep(8, 32)
  builder:Pad(2)
  builder:Prep(2, 4)
  builder:Pad(1)
  builder:PrependInt8(test3_b)
  builder:PrependInt16(test3_a)
  builder:Pad(1)
  builder:PrependUint8(test2)
  builder:PrependFloat64(test1)
  builder:Pad(4)
  builder:PrependFloat32(z)
  builder:PrependFloat32(y)
  builder:PrependFloat32(x)
  return builder:Offset()
end

return Vec3