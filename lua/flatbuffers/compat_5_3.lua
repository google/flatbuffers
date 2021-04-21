-- We need to put it into a separate file to avoid syntax error like `unexpected symbol near '~'`
local m = {}


m.GetAlignSize = function(k, size)
    return ((~k) + 1) & (size - 1)
end


m.spack = string.pack
m.sunpack = string.unpack


return m
