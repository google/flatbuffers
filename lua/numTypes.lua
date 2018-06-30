local m = {}

local ba = require("binaryarray")

local type_mt =  {}

function type_mt:Pack(value)
    return ba.Pack(self.packFmt, value)
end

function type_mt:Unpack(s, pos)
    return ba.Unpack(self.packFmt, s, pos)
end

function type_mt:ValidNumber(n)
    if not self.min_value and not self.max_value then return true end
    return self.min_value <= n and n <= self.max_value
end

function type_mt:EnforceNumber(n)
    assert(self:ValidNumber(n), "Number is not in the valid range")
end

function type_mt:ConvertType(n, otherType)
    assert(self.bytewidth == otherType.bytewidth, "Cannot convert between types of different widths")
    if self == otherType then
        return n
    end
    return otherType:Unpack(self:Pack(n))
end

local bool_mt =
{
    bytewidth = 1,
    min_value = false,
    max_value = true,
    lua_type = type(true),
    name = "bool",
    packFmt = "<b"
}

local uint8_mt = 
{
    bytewidth = 1,
    min_value = 0,
    max_value = math.pow(2,8)-1,
    lua_type = type(1),
    name = "uint8",
    packFmt = "<I1"
}

local uint16_mt = 
{
    bytewidth = 2,
    min_value = 0,
    max_value = math.pow(2,16)-1,
    lua_type = type(1),
    name = "uint16",
    packFmt = "<I2"
}

local uint32_mt = 
{
    bytewidth = 4,
    min_value = 0,
    max_value = math.pow(2,32)-1,
    lua_type = type(1),
    name = "uint32",
    packFmt = "<I4"
}

local uint64_mt = 
{
    bytewidth = 8,
    min_value = 0,
    max_value = math.pow(2,64)-1,
    lua_type = type(1),
    name = "uint64",
    packFmt = "<I8"
}

local int8_mt = 
{
    bytewidth = 1,
    min_value = -math.pow(2,7),
    max_value = math.pow(2,7)-1,
    lua_type = type(1),
    name = "int8",
    packFmt = "<i1"
}

local int16_mt = 
{
    bytewidth = 2,
    min_value = -math.pow(2,15),
    max_value = math.pow(2,15)-1,
    lua_type = type(1),
    name = "int16",
    packFmt = "<i2"
}

local int32_mt = 
{
    bytewidth = 4,
    min_value = -math.pow(2,15),
    max_value = math.pow(2,15)-1,
    lua_type = type(1),
    name = "int32",
    packFmt = "<i4"
}

local int64_mt = 
{
    bytewidth = 8,
    min_value = -math.pow(2,63),
    max_value = math.pow(2,63)-1,
    lua_type = type(1),
    name = "int64",
    packFmt = "<i8"
}

local float32_mt = 
{
    bytewidth = 4,
    min_value = nil,
    max_value = nil,
    lua_type = type(1.0),
    name = "float32",
    packFmt = "<f"
}

local float64_mt = 
{
    bytewidth = 8,
    min_value = nil,
    max_value = nil,
    lua_type = type(1.0),
    name = "float64",
    packFmt = "<d"
}

-- register the base class
setmetatable(bool_mt, {__index = type_mt})
setmetatable(uint8_mt, {__index = type_mt})
setmetatable(uint16_mt, {__index = type_mt})
setmetatable(uint32_mt, {__index = type_mt})
setmetatable(uint64_mt, {__index = type_mt})
setmetatable(int8_mt, {__index = type_mt})
setmetatable(int16_mt, {__index = type_mt})
setmetatable(int32_mt, {__index = type_mt})
setmetatable(int64_mt, {__index = type_mt})
setmetatable(float32_mt, {__index = type_mt})
setmetatable(float64_mt, {__index = type_mt})

m.Bool      = bool_mt
m.Uint8     = uint8_mt
m.Uint16    = uint16_mt
m.Uint32    = uint32_mt
m.Uint64    = uint64_mt
m.Int8      = int8_mt
m.Int16     = int16_mt
m.Int32     = int32_mt
m.Int64     = int64_mt
m.Float32   = float32_mt
m.Float64   = float64_mt

m.UOffsetT  = uint32_mt
m.VOffsetT  = uint16_mt
m.SOffsetT  = int32_mt

return m