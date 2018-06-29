local m = {}

local mt = {}

local function generateArray(size, default)
    default = default or 0x0
    return string.rep(string.char(default, size))
end

function m.New(size)
    local o = {}
    setmetatable(o, {__index = o})
    o.len = size
    o.bytes = generateArray(size)
    return o
end

function mt:Slice(startPos, endPos)
    endPos = endPos or #self
    return self.bytes:byte(startPos, endPos)
end

function mt:Grow(newsize)
    self.bytes = generateArray(self.len) .. self.bytes
    assert(#self.bytes == newsize)
end

function mt:Set(value, startPos, endPos)
    
    
end

function m.Pack(fmt, ...)
    return string.pack(fmt, ...)
end

function m.Unpack(fmt, s, pos)
    return string.unpack(fmt, s, pos)
end

return m