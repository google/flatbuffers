--[[ MyGame.Example.Monster

  Automatically generated by the FlatBuffers compiler, do not modify.
  Or modify. I'm a message, not a cop.

  flatc version: 23.1.20

  Declared by  : //monster_test.fbs
  Rooting type : MyGame.Example.Monster (//monster_test.fbs)

--]]

local __MyGame_Example_Ability = require('MyGame.Example.Ability')
local __MyGame_Example_Referrable = require('MyGame.Example.Referrable')
local __MyGame_Example_Stat = require('MyGame.Example.Stat')
local __MyGame_Example_Test = require('MyGame.Example.Test')
local __MyGame_Example_Vec3 = require('MyGame.Example.Vec3')
local __MyGame_InParentNamespace = require('MyGame.InParentNamespace')
local flatbuffers = require('flatbuffers')

-- an example documentation comment: "monster object"
local Monster = {}
local mt = {}

function Monster.New()
  local o = {}
  setmetatable(o, {__index = mt})
  return o
end

function Monster.GetRootAsMonster(buf, offset)
  if type(buf) == "string" then
    buf = flatbuffers.binaryArray.New(buf)
  end

  local n = flatbuffers.N.UOffsetT:Unpack(buf, offset)
  local o = Monster.New()
  o:Init(buf, n + offset)
  return o
end

function mt:Init(buf, pos)
  self.view = flatbuffers.view.New(buf, pos)
end

function mt:Pos()
  local o = self.view:Offset(4)
  if o ~= 0 then
    local x = self.view.pos + o
    local obj = __MyGame_Example_Vec3.New()
    obj:Init(self.view.bytes, x)
    return obj
  end
end

function mt:Mana()
  local o = self.view:Offset(6)
  if o ~= 0 then
    return self.view:Get(flatbuffers.N.Int16, self.view.pos + o)
  end
  return 150
end

function mt:Hp()
  local o = self.view:Offset(8)
  if o ~= 0 then
    return self.view:Get(flatbuffers.N.Int16, self.view.pos + o)
  end
  return 100
end

function mt:Name()
  local o = self.view:Offset(10)
  if o ~= 0 then
    return self.view:String(self.view.pos + o)
  end
end

function mt:Inventory(j)
  local o = self.view:Offset(14)
  if o ~= 0 then
    local a = self.view:Vector(o)
    return self.view:Get(flatbuffers.N.Uint8, a + ((j-1) * 1))
  end
  return 0
end

function mt:InventoryAsString(start, stop)
  return self.view:VectorAsString(14, start, stop)
end

function mt:InventoryLength()
  local o = self.view:Offset(14)
  if o ~= 0 then
    return self.view:VectorLen(o)
  end
  return 0
end

function mt:Color()
  local o = self.view:Offset(16)
  if o ~= 0 then
    return self.view:Get(flatbuffers.N.Uint8, self.view.pos + o)
  end
  return 8
end

function mt:TestType()
  local o = self.view:Offset(18)
  if o ~= 0 then
    return self.view:Get(flatbuffers.N.Uint8, self.view.pos + o)
  end
  return 0
end

function mt:Test()
  local o = self.view:Offset(20)
  if o ~= 0 then
   local obj = flatbuffers.view.New(flatbuffers.binaryArray.New(0), 0)
    self.view:Union(obj, o)
    return obj
  end
end

function mt:Test4(j)
  local o = self.view:Offset(22)
  if o ~= 0 then
    local x = self.view:Vector(o)
    x = x + ((j-1) * 4)
    local obj = __MyGame_Example_Test.New()
    obj:Init(self.view.bytes, x)
    return obj
  end
end

function mt:Test4Length()
  local o = self.view:Offset(22)
  if o ~= 0 then
    return self.view:VectorLen(o)
  end
  return 0
end

function mt:Testarrayofstring(j)
  local o = self.view:Offset(24)
  if o ~= 0 then
    local a = self.view:Vector(o)
    return self.view:String(a + ((j-1) * 4))
  end
  return ''
end

function mt:TestarrayofstringLength()
  local o = self.view:Offset(24)
  if o ~= 0 then
    return self.view:VectorLen(o)
  end
  return 0
end

-- an example documentation comment: this will end up in the generated code
-- multiline too
function mt:Testarrayoftables(j)
  local o = self.view:Offset(26)
  if o ~= 0 then
    local x = self.view:Vector(o)
    x = x + ((j-1) * 4)
    x = self.view:Indirect(x)
    local obj = Monster.New()
    obj:Init(self.view.bytes, x)
    return obj
  end
end

function mt:TestarrayoftablesLength()
  local o = self.view:Offset(26)
  if o ~= 0 then
    return self.view:VectorLen(o)
  end
  return 0
end

function mt:Enemy()
  local o = self.view:Offset(28)
  if o ~= 0 then
    local x = self.view:Indirect(self.view.pos + o)
    local obj = Monster.New()
    obj:Init(self.view.bytes, x)
    return obj
  end
end

function mt:Testnestedflatbuffer(j)
  local o = self.view:Offset(30)
  if o ~= 0 then
    local a = self.view:Vector(o)
    return self.view:Get(flatbuffers.N.Uint8, a + ((j-1) * 1))
  end
  return 0
end

function mt:TestnestedflatbufferAsString(start, stop)
  return self.view:VectorAsString(30, start, stop)
end

function mt:TestnestedflatbufferLength()
  local o = self.view:Offset(30)
  if o ~= 0 then
    return self.view:VectorLen(o)
  end
  return 0
end

function mt:Testempty()
  local o = self.view:Offset(32)
  if o ~= 0 then
    local x = self.view:Indirect(self.view.pos + o)
    local obj = __MyGame_Example_Stat.New()
    obj:Init(self.view.bytes, x)
    return obj
  end
end

function mt:Testbool()
  local o = self.view:Offset(34)
  if o ~= 0 then
    return (self.view:Get(flatbuffers.N.Bool, self.view.pos + o) ~=0)
  end
  return false
end

function mt:Testhashs32Fnv1()
  local o = self.view:Offset(36)
  if o ~= 0 then
    return self.view:Get(flatbuffers.N.Int32, self.view.pos + o)
  end
  return 0
end

function mt:Testhashu32Fnv1()
  local o = self.view:Offset(38)
  if o ~= 0 then
    return self.view:Get(flatbuffers.N.Uint32, self.view.pos + o)
  end
  return 0
end

function mt:Testhashs64Fnv1()
  local o = self.view:Offset(40)
  if o ~= 0 then
    return self.view:Get(flatbuffers.N.Int64, self.view.pos + o)
  end
  return 0
end

function mt:Testhashu64Fnv1()
  local o = self.view:Offset(42)
  if o ~= 0 then
    return self.view:Get(flatbuffers.N.Uint64, self.view.pos + o)
  end
  return 0
end

function mt:Testhashs32Fnv1a()
  local o = self.view:Offset(44)
  if o ~= 0 then
    return self.view:Get(flatbuffers.N.Int32, self.view.pos + o)
  end
  return 0
end

function mt:Testhashu32Fnv1a()
  local o = self.view:Offset(46)
  if o ~= 0 then
    return self.view:Get(flatbuffers.N.Uint32, self.view.pos + o)
  end
  return 0
end

function mt:Testhashs64Fnv1a()
  local o = self.view:Offset(48)
  if o ~= 0 then
    return self.view:Get(flatbuffers.N.Int64, self.view.pos + o)
  end
  return 0
end

function mt:Testhashu64Fnv1a()
  local o = self.view:Offset(50)
  if o ~= 0 then
    return self.view:Get(flatbuffers.N.Uint64, self.view.pos + o)
  end
  return 0
end

function mt:Testarrayofbools(j)
  local o = self.view:Offset(52)
  if o ~= 0 then
    local a = self.view:Vector(o)
    return self.view:Get(flatbuffers.N.Bool, a + ((j-1) * 1))
  end
  return 0
end

function mt:TestarrayofboolsAsString(start, stop)
  return self.view:VectorAsString(52, start, stop)
end

function mt:TestarrayofboolsLength()
  local o = self.view:Offset(52)
  if o ~= 0 then
    return self.view:VectorLen(o)
  end
  return 0
end

function mt:Testf()
  local o = self.view:Offset(54)
  if o ~= 0 then
    return self.view:Get(flatbuffers.N.Float32, self.view.pos + o)
  end
  return 3.14159
end

function mt:Testf2()
  local o = self.view:Offset(56)
  if o ~= 0 then
    return self.view:Get(flatbuffers.N.Float32, self.view.pos + o)
  end
  return 3.0
end

function mt:Testf3()
  local o = self.view:Offset(58)
  if o ~= 0 then
    return self.view:Get(flatbuffers.N.Float32, self.view.pos + o)
  end
  return 0.0
end

function mt:Testarrayofstring2(j)
  local o = self.view:Offset(60)
  if o ~= 0 then
    local a = self.view:Vector(o)
    return self.view:String(a + ((j-1) * 4))
  end
  return ''
end

function mt:Testarrayofstring2Length()
  local o = self.view:Offset(60)
  if o ~= 0 then
    return self.view:VectorLen(o)
  end
  return 0
end

function mt:Testarrayofsortedstruct(j)
  local o = self.view:Offset(62)
  if o ~= 0 then
    local x = self.view:Vector(o)
    x = x + ((j-1) * 8)
    local obj = __MyGame_Example_Ability.New()
    obj:Init(self.view.bytes, x)
    return obj
  end
end

function mt:TestarrayofsortedstructLength()
  local o = self.view:Offset(62)
  if o ~= 0 then
    return self.view:VectorLen(o)
  end
  return 0
end

function mt:Flex(j)
  local o = self.view:Offset(64)
  if o ~= 0 then
    local a = self.view:Vector(o)
    return self.view:Get(flatbuffers.N.Uint8, a + ((j-1) * 1))
  end
  return 0
end

function mt:FlexAsString(start, stop)
  return self.view:VectorAsString(64, start, stop)
end

function mt:FlexLength()
  local o = self.view:Offset(64)
  if o ~= 0 then
    return self.view:VectorLen(o)
  end
  return 0
end

function mt:Test5(j)
  local o = self.view:Offset(66)
  if o ~= 0 then
    local x = self.view:Vector(o)
    x = x + ((j-1) * 4)
    local obj = __MyGame_Example_Test.New()
    obj:Init(self.view.bytes, x)
    return obj
  end
end

function mt:Test5Length()
  local o = self.view:Offset(66)
  if o ~= 0 then
    return self.view:VectorLen(o)
  end
  return 0
end

function mt:VectorOfLongs(j)
  local o = self.view:Offset(68)
  if o ~= 0 then
    local a = self.view:Vector(o)
    return self.view:Get(flatbuffers.N.Int64, a + ((j-1) * 8))
  end
  return 0
end

function mt:VectorOfLongsLength()
  local o = self.view:Offset(68)
  if o ~= 0 then
    return self.view:VectorLen(o)
  end
  return 0
end

function mt:VectorOfDoubles(j)
  local o = self.view:Offset(70)
  if o ~= 0 then
    local a = self.view:Vector(o)
    return self.view:Get(flatbuffers.N.Float64, a + ((j-1) * 8))
  end
  return 0
end

function mt:VectorOfDoublesLength()
  local o = self.view:Offset(70)
  if o ~= 0 then
    return self.view:VectorLen(o)
  end
  return 0
end

function mt:ParentNamespaceTest()
  local o = self.view:Offset(72)
  if o ~= 0 then
    local x = self.view:Indirect(self.view.pos + o)
    local obj = __MyGame_InParentNamespace.New()
    obj:Init(self.view.bytes, x)
    return obj
  end
end

function mt:VectorOfReferrables(j)
  local o = self.view:Offset(74)
  if o ~= 0 then
    local x = self.view:Vector(o)
    x = x + ((j-1) * 4)
    x = self.view:Indirect(x)
    local obj = __MyGame_Example_Referrable.New()
    obj:Init(self.view.bytes, x)
    return obj
  end
end

function mt:VectorOfReferrablesLength()
  local o = self.view:Offset(74)
  if o ~= 0 then
    return self.view:VectorLen(o)
  end
  return 0
end

function mt:SingleWeakReference()
  local o = self.view:Offset(76)
  if o ~= 0 then
    return self.view:Get(flatbuffers.N.Uint64, self.view.pos + o)
  end
  return 0
end

function mt:VectorOfWeakReferences(j)
  local o = self.view:Offset(78)
  if o ~= 0 then
    local a = self.view:Vector(o)
    return self.view:Get(flatbuffers.N.Uint64, a + ((j-1) * 8))
  end
  return 0
end

function mt:VectorOfWeakReferencesLength()
  local o = self.view:Offset(78)
  if o ~= 0 then
    return self.view:VectorLen(o)
  end
  return 0
end

function mt:VectorOfStrongReferrables(j)
  local o = self.view:Offset(80)
  if o ~= 0 then
    local x = self.view:Vector(o)
    x = x + ((j-1) * 4)
    x = self.view:Indirect(x)
    local obj = __MyGame_Example_Referrable.New()
    obj:Init(self.view.bytes, x)
    return obj
  end
end

function mt:VectorOfStrongReferrablesLength()
  local o = self.view:Offset(80)
  if o ~= 0 then
    return self.view:VectorLen(o)
  end
  return 0
end

function mt:CoOwningReference()
  local o = self.view:Offset(82)
  if o ~= 0 then
    return self.view:Get(flatbuffers.N.Uint64, self.view.pos + o)
  end
  return 0
end

function mt:VectorOfCoOwningReferences(j)
  local o = self.view:Offset(84)
  if o ~= 0 then
    local a = self.view:Vector(o)
    return self.view:Get(flatbuffers.N.Uint64, a + ((j-1) * 8))
  end
  return 0
end

function mt:VectorOfCoOwningReferencesLength()
  local o = self.view:Offset(84)
  if o ~= 0 then
    return self.view:VectorLen(o)
  end
  return 0
end

function mt:NonOwningReference()
  local o = self.view:Offset(86)
  if o ~= 0 then
    return self.view:Get(flatbuffers.N.Uint64, self.view.pos + o)
  end
  return 0
end

function mt:VectorOfNonOwningReferences(j)
  local o = self.view:Offset(88)
  if o ~= 0 then
    local a = self.view:Vector(o)
    return self.view:Get(flatbuffers.N.Uint64, a + ((j-1) * 8))
  end
  return 0
end

function mt:VectorOfNonOwningReferencesLength()
  local o = self.view:Offset(88)
  if o ~= 0 then
    return self.view:VectorLen(o)
  end
  return 0
end

function mt:AnyUniqueType()
  local o = self.view:Offset(90)
  if o ~= 0 then
    return self.view:Get(flatbuffers.N.Uint8, self.view.pos + o)
  end
  return 0
end

function mt:AnyUnique()
  local o = self.view:Offset(92)
  if o ~= 0 then
   local obj = flatbuffers.view.New(flatbuffers.binaryArray.New(0), 0)
    self.view:Union(obj, o)
    return obj
  end
end

function mt:AnyAmbiguousType()
  local o = self.view:Offset(94)
  if o ~= 0 then
    return self.view:Get(flatbuffers.N.Uint8, self.view.pos + o)
  end
  return 0
end

function mt:AnyAmbiguous()
  local o = self.view:Offset(96)
  if o ~= 0 then
   local obj = flatbuffers.view.New(flatbuffers.binaryArray.New(0), 0)
    self.view:Union(obj, o)
    return obj
  end
end

function mt:VectorOfEnums(j)
  local o = self.view:Offset(98)
  if o ~= 0 then
    local a = self.view:Vector(o)
    return self.view:Get(flatbuffers.N.Uint8, a + ((j-1) * 1))
  end
  return 0
end

function mt:VectorOfEnumsAsString(start, stop)
  return self.view:VectorAsString(98, start, stop)
end

function mt:VectorOfEnumsLength()
  local o = self.view:Offset(98)
  if o ~= 0 then
    return self.view:VectorLen(o)
  end
  return 0
end

function mt:SignedEnum()
  local o = self.view:Offset(100)
  if o ~= 0 then
    return self.view:Get(flatbuffers.N.Int8, self.view.pos + o)
  end
  return -1
end

function mt:Testrequirednestedflatbuffer(j)
  local o = self.view:Offset(102)
  if o ~= 0 then
    local a = self.view:Vector(o)
    return self.view:Get(flatbuffers.N.Uint8, a + ((j-1) * 1))
  end
  return 0
end

function mt:TestrequirednestedflatbufferAsString(start, stop)
  return self.view:VectorAsString(102, start, stop)
end

function mt:TestrequirednestedflatbufferLength()
  local o = self.view:Offset(102)
  if o ~= 0 then
    return self.view:VectorLen(o)
  end
  return 0
end

function mt:ScalarKeySortedTables(j)
  local o = self.view:Offset(104)
  if o ~= 0 then
    local x = self.view:Vector(o)
    x = x + ((j-1) * 4)
    x = self.view:Indirect(x)
    local obj = __MyGame_Example_Stat.New()
    obj:Init(self.view.bytes, x)
    return obj
  end
end

function mt:ScalarKeySortedTablesLength()
  local o = self.view:Offset(104)
  if o ~= 0 then
    return self.view:VectorLen(o)
  end
  return 0
end

function mt:NativeInline()
  local o = self.view:Offset(106)
  if o ~= 0 then
    local x = self.view.pos + o
    local obj = __MyGame_Example_Test.New()
    obj:Init(self.view.bytes, x)
    return obj
  end
end

function mt:LongEnumNonEnumDefault()
  local o = self.view:Offset(108)
  if o ~= 0 then
    return self.view:Get(flatbuffers.N.Uint64, self.view.pos + o)
  end
  return 0
end

function mt:LongEnumNormalDefault()
  local o = self.view:Offset(110)
  if o ~= 0 then
    return self.view:Get(flatbuffers.N.Uint64, self.view.pos + o)
  end
  return 2
end

function mt:NanDefault()
  local o = self.view:Offset(112)
  if o ~= 0 then
    return self.view:Get(flatbuffers.N.Float32, self.view.pos + o)
  end
  return nan
end

function mt:InfDefault()
  local o = self.view:Offset(114)
  if o ~= 0 then
    return self.view:Get(flatbuffers.N.Float32, self.view.pos + o)
  end
  return inf
end

function mt:PositiveInfDefault()
  local o = self.view:Offset(116)
  if o ~= 0 then
    return self.view:Get(flatbuffers.N.Float32, self.view.pos + o)
  end
  return inf
end

function mt:InfinityDefault()
  local o = self.view:Offset(118)
  if o ~= 0 then
    return self.view:Get(flatbuffers.N.Float32, self.view.pos + o)
  end
  return inf
end

function mt:PositiveInfinityDefault()
  local o = self.view:Offset(120)
  if o ~= 0 then
    return self.view:Get(flatbuffers.N.Float32, self.view.pos + o)
  end
  return inf
end

function mt:NegativeInfDefault()
  local o = self.view:Offset(122)
  if o ~= 0 then
    return self.view:Get(flatbuffers.N.Float32, self.view.pos + o)
  end
  return -inf
end

function mt:NegativeInfinityDefault()
  local o = self.view:Offset(124)
  if o ~= 0 then
    return self.view:Get(flatbuffers.N.Float32, self.view.pos + o)
  end
  return -inf
end

function mt:DoubleInfDefault()
  local o = self.view:Offset(126)
  if o ~= 0 then
    return self.view:Get(flatbuffers.N.Float64, self.view.pos + o)
  end
  return inf
end

function Monster.Start(builder)
  builder:StartObject(62)
end

function Monster.AddPos(builder, pos)
  builder:PrependStructSlot(0, pos, 0)
end

function Monster.AddMana(builder, mana)
  builder:PrependInt16Slot(1, mana, 150)
end

function Monster.AddHp(builder, hp)
  builder:PrependInt16Slot(2, hp, 100)
end

function Monster.AddName(builder, name)
  builder:PrependUOffsetTRelativeSlot(3, name, 0)
end

function Monster.AddInventory(builder, inventory)
  builder:PrependUOffsetTRelativeSlot(5, inventory, 0)
end

function Monster.StartInventoryVector(builder, numElems)
  return builder:StartVector(1, numElems, 1)
end

function Monster.AddColor(builder, color)
  builder:PrependUint8Slot(6, color, 8)
end

function Monster.AddTestType(builder, testType)
  builder:PrependUint8Slot(7, testType, 0)
end

function Monster.AddTest(builder, test)
  builder:PrependUOffsetTRelativeSlot(8, test, 0)
end

function Monster.AddTest4(builder, test4)
  builder:PrependUOffsetTRelativeSlot(9, test4, 0)
end

function Monster.StartTest4Vector(builder, numElems)
  return builder:StartVector(4, numElems, 2)
end

function Monster.AddTestarrayofstring(builder, testarrayofstring)
  builder:PrependUOffsetTRelativeSlot(10, testarrayofstring, 0)
end

function Monster.StartTestarrayofstringVector(builder, numElems)
  return builder:StartVector(4, numElems, 4)
end

function Monster.AddTestarrayoftables(builder, testarrayoftables)
  builder:PrependUOffsetTRelativeSlot(11, testarrayoftables, 0)
end

function Monster.StartTestarrayoftablesVector(builder, numElems)
  return builder:StartVector(4, numElems, 4)
end

function Monster.AddEnemy(builder, enemy)
  builder:PrependStructSlot(12, enemy, 0)
end

function Monster.AddTestnestedflatbuffer(builder, testnestedflatbuffer)
  builder:PrependUOffsetTRelativeSlot(13, testnestedflatbuffer, 0)
end

function Monster.StartTestnestedflatbufferVector(builder, numElems)
  return builder:StartVector(1, numElems, 1)
end

function Monster.AddTestempty(builder, testempty)
  builder:PrependStructSlot(14, testempty, 0)
end

function Monster.AddTestbool(builder, testbool)
  builder:PrependBoolSlot(15, testbool, false)
end

function Monster.AddTesthashs32Fnv1(builder, testhashs32Fnv1)
  builder:PrependInt32Slot(16, testhashs32Fnv1, 0)
end

function Monster.AddTesthashu32Fnv1(builder, testhashu32Fnv1)
  builder:PrependUint32Slot(17, testhashu32Fnv1, 0)
end

function Monster.AddTesthashs64Fnv1(builder, testhashs64Fnv1)
  builder:PrependInt64Slot(18, testhashs64Fnv1, 0)
end

function Monster.AddTesthashu64Fnv1(builder, testhashu64Fnv1)
  builder:PrependUint64Slot(19, testhashu64Fnv1, 0)
end

function Monster.AddTesthashs32Fnv1a(builder, testhashs32Fnv1a)
  builder:PrependInt32Slot(20, testhashs32Fnv1a, 0)
end

function Monster.AddTesthashu32Fnv1a(builder, testhashu32Fnv1a)
  builder:PrependUint32Slot(21, testhashu32Fnv1a, 0)
end

function Monster.AddTesthashs64Fnv1a(builder, testhashs64Fnv1a)
  builder:PrependInt64Slot(22, testhashs64Fnv1a, 0)
end

function Monster.AddTesthashu64Fnv1a(builder, testhashu64Fnv1a)
  builder:PrependUint64Slot(23, testhashu64Fnv1a, 0)
end

function Monster.AddTestarrayofbools(builder, testarrayofbools)
  builder:PrependUOffsetTRelativeSlot(24, testarrayofbools, 0)
end

function Monster.StartTestarrayofboolsVector(builder, numElems)
  return builder:StartVector(1, numElems, 1)
end

function Monster.AddTestf(builder, testf)
  builder:PrependFloat32Slot(25, testf, 3.14159)
end

function Monster.AddTestf2(builder, testf2)
  builder:PrependFloat32Slot(26, testf2, 3.0)
end

function Monster.AddTestf3(builder, testf3)
  builder:PrependFloat32Slot(27, testf3, 0.0)
end

function Monster.AddTestarrayofstring2(builder, testarrayofstring2)
  builder:PrependUOffsetTRelativeSlot(28, testarrayofstring2, 0)
end

function Monster.StartTestarrayofstring2Vector(builder, numElems)
  return builder:StartVector(4, numElems, 4)
end

function Monster.AddTestarrayofsortedstruct(builder, testarrayofsortedstruct)
  builder:PrependUOffsetTRelativeSlot(29, testarrayofsortedstruct, 0)
end

function Monster.StartTestarrayofsortedstructVector(builder, numElems)
  return builder:StartVector(8, numElems, 4)
end

function Monster.AddFlex(builder, flex)
  builder:PrependUOffsetTRelativeSlot(30, flex, 0)
end

function Monster.StartFlexVector(builder, numElems)
  return builder:StartVector(1, numElems, 1)
end

function Monster.AddTest5(builder, test5)
  builder:PrependUOffsetTRelativeSlot(31, test5, 0)
end

function Monster.StartTest5Vector(builder, numElems)
  return builder:StartVector(4, numElems, 2)
end

function Monster.AddVectorOfLongs(builder, vectorOfLongs)
  builder:PrependUOffsetTRelativeSlot(32, vectorOfLongs, 0)
end

function Monster.StartVectorOfLongsVector(builder, numElems)
  return builder:StartVector(8, numElems, 8)
end

function Monster.AddVectorOfDoubles(builder, vectorOfDoubles)
  builder:PrependUOffsetTRelativeSlot(33, vectorOfDoubles, 0)
end

function Monster.StartVectorOfDoublesVector(builder, numElems)
  return builder:StartVector(8, numElems, 8)
end

function Monster.AddParentNamespaceTest(builder, parentNamespaceTest)
  builder:PrependStructSlot(34, parentNamespaceTest, 0)
end

function Monster.AddVectorOfReferrables(builder, vectorOfReferrables)
  builder:PrependUOffsetTRelativeSlot(35, vectorOfReferrables, 0)
end

function Monster.StartVectorOfReferrablesVector(builder, numElems)
  return builder:StartVector(4, numElems, 4)
end

function Monster.AddSingleWeakReference(builder, singleWeakReference)
  builder:PrependUint64Slot(36, singleWeakReference, 0)
end

function Monster.AddVectorOfWeakReferences(builder, vectorOfWeakReferences)
  builder:PrependUOffsetTRelativeSlot(37, vectorOfWeakReferences, 0)
end

function Monster.StartVectorOfWeakReferencesVector(builder, numElems)
  return builder:StartVector(8, numElems, 8)
end

function Monster.AddVectorOfStrongReferrables(builder, vectorOfStrongReferrables)
  builder:PrependUOffsetTRelativeSlot(38, vectorOfStrongReferrables, 0)
end

function Monster.StartVectorOfStrongReferrablesVector(builder, numElems)
  return builder:StartVector(4, numElems, 4)
end

function Monster.AddCoOwningReference(builder, coOwningReference)
  builder:PrependUint64Slot(39, coOwningReference, 0)
end

function Monster.AddVectorOfCoOwningReferences(builder, vectorOfCoOwningReferences)
  builder:PrependUOffsetTRelativeSlot(40, vectorOfCoOwningReferences, 0)
end

function Monster.StartVectorOfCoOwningReferencesVector(builder, numElems)
  return builder:StartVector(8, numElems, 8)
end

function Monster.AddNonOwningReference(builder, nonOwningReference)
  builder:PrependUint64Slot(41, nonOwningReference, 0)
end

function Monster.AddVectorOfNonOwningReferences(builder, vectorOfNonOwningReferences)
  builder:PrependUOffsetTRelativeSlot(42, vectorOfNonOwningReferences, 0)
end

function Monster.StartVectorOfNonOwningReferencesVector(builder, numElems)
  return builder:StartVector(8, numElems, 8)
end

function Monster.AddAnyUniqueType(builder, anyUniqueType)
  builder:PrependUint8Slot(43, anyUniqueType, 0)
end

function Monster.AddAnyUnique(builder, anyUnique)
  builder:PrependUOffsetTRelativeSlot(44, anyUnique, 0)
end

function Monster.AddAnyAmbiguousType(builder, anyAmbiguousType)
  builder:PrependUint8Slot(45, anyAmbiguousType, 0)
end

function Monster.AddAnyAmbiguous(builder, anyAmbiguous)
  builder:PrependUOffsetTRelativeSlot(46, anyAmbiguous, 0)
end

function Monster.AddVectorOfEnums(builder, vectorOfEnums)
  builder:PrependUOffsetTRelativeSlot(47, vectorOfEnums, 0)
end

function Monster.StartVectorOfEnumsVector(builder, numElems)
  return builder:StartVector(1, numElems, 1)
end

function Monster.AddSignedEnum(builder, signedEnum)
  builder:PrependInt8Slot(48, signedEnum, -1)
end

function Monster.AddTestrequirednestedflatbuffer(builder, testrequirednestedflatbuffer)
  builder:PrependUOffsetTRelativeSlot(49, testrequirednestedflatbuffer, 0)
end

function Monster.StartTestrequirednestedflatbufferVector(builder, numElems)
  return builder:StartVector(1, numElems, 1)
end

function Monster.AddScalarKeySortedTables(builder, scalarKeySortedTables)
  builder:PrependUOffsetTRelativeSlot(50, scalarKeySortedTables, 0)
end

function Monster.StartScalarKeySortedTablesVector(builder, numElems)
  return builder:StartVector(4, numElems, 4)
end

function Monster.AddNativeInline(builder, nativeInline)
  builder:PrependStructSlot(51, nativeInline, 0)
end

function Monster.AddLongEnumNonEnumDefault(builder, longEnumNonEnumDefault)
  builder:PrependUint64Slot(52, longEnumNonEnumDefault, 0)
end

function Monster.AddLongEnumNormalDefault(builder, longEnumNormalDefault)
  builder:PrependUint64Slot(53, longEnumNormalDefault, 2)
end

function Monster.AddNanDefault(builder, nanDefault)
  builder:PrependFloat32Slot(54, nanDefault, nan)
end

function Monster.AddInfDefault(builder, infDefault)
  builder:PrependFloat32Slot(55, infDefault, inf)
end

function Monster.AddPositiveInfDefault(builder, positiveInfDefault)
  builder:PrependFloat32Slot(56, positiveInfDefault, inf)
end

function Monster.AddInfinityDefault(builder, infinityDefault)
  builder:PrependFloat32Slot(57, infinityDefault, inf)
end

function Monster.AddPositiveInfinityDefault(builder, positiveInfinityDefault)
  builder:PrependFloat32Slot(58, positiveInfinityDefault, inf)
end

function Monster.AddNegativeInfDefault(builder, negativeInfDefault)
  builder:PrependFloat32Slot(59, negativeInfDefault, -inf)
end

function Monster.AddNegativeInfinityDefault(builder, negativeInfinityDefault)
  builder:PrependFloat32Slot(60, negativeInfinityDefault, -inf)
end

function Monster.AddDoubleInfDefault(builder, doubleInfDefault)
  builder:PrependFloat64Slot(61, doubleInfDefault, inf)
end

function Monster.End(builder)
  return builder:EndObject()
end

return Monster