local m = {}

local mt = {}

local function generateArray(size, default)
    if size <= 0 then return "" end
    default = default or 0x0
    local t = {}
    for i=1,size do
        t[i] = string.char(default)
    end
    return table.concat(t)
end

function mt:__len()
    return string.len(self.bytes)
end

function m.New(size)
    local o = {}
    setmetatable(o, {__index = mt, __len = mt.__len})
    o.bytes = generateArray(size)
    return o
end

function mt:Slice(startPos, endPos)
    endPos = endPos or #self
    local r = self.bytes:sub(startPos, endPos)
    return r
end

function mt:Grow(newsize)
    if #self.bytes == 0 then
        self.bytes = generateArray(newsize)
    else
        self.bytes = generateArray(#self.bytes) .. self.bytes
    end
    assert(#self.bytes == newsize)
end

function mt:Set(value, startPos, endPos)
    endPos = (endPos or startPos + #value)+1
    self.bytes = self.bytes:sub(1,startPos) .. value .. self.bytes:sub(endPos)
end

function m.Pack(fmt, ...)
    return string.pack(fmt, ...)
end

function m.Unpack(fmt, s, pos)
    return string.unpack(fmt, s, pos+1)
end

function m.DumpHex(buf)
    -- from: http://lua-users.org/wiki/HexDump
    for byte=1, #buf, 16 do
        local chunk = buf:sub(byte, byte+15)
        io.write(string.format('%08X  ',byte-1))
        chunk:gsub('.', function (c) io.write(string.format('%02X ',string.byte(c))) end)
        io.write(string.rep(' ',3*(16-#chunk)))
        io.write(' ',chunk:gsub('%c','.'),"\n") 
    end
end

return m