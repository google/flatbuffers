local m = {}

local flag_mt = 
{
    
}

function flag_mt:Pack(value)
    return string.pack(self.packFmt, value)
end

function flag_mt:Unpack(s, pos)
    return string.unpack(self.packFmt, s, pos)
end

function flag_mt:EnforceNumber(n)
   if not self.min_value and not self.max_value then return nil end
   assert(self.min_value <= n and n <= self.max_value, "Number is not in the valid range")
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

setmetatable(bool_mt, flag_mt)

m.Bool = bool_mt

return m