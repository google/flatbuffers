local m = {}

local mt = {}

local function generateArray(size, default)
    if size <= 0 then return "" end
    default = default or 0x0
    local t = {}
    for i=1,size do
        t[i] = string.char(default)
    end
    return table.concat(t), t
end

function mt:__len()
    return string.len(self.bytes)
end

function m.New(size)
    local o = {}
    setmetatable(o, {__index = mt, __len = mt.__len})
    o.bytes, o.bytes2 = generateArray(size)
    return o
end

function m.FromString(str)
    local o = {}
    setmetatable(o, {__index = mt, __len = mt.__len})
    o.bytes = str
    return o    
end

function mt:Slice(startPos, endPos)
    -- n.b start/endPos are 0-based incoming, so need to convert
    --     correctly. in python a slice includes start -> end - 1
    local r = self.bytes:sub(startPos+1, endPos)
    return r
end

function mt:Grow(newsize)
    if #self.bytes == 0 then
        self.bytes = generateArray(newsize)
    else        
        self.bytes = generateArray(newsize - #self.bytes) .. self.bytes
    end
    assert(#self.bytes == newsize)
end

function mt:Set(value, startPos, endPos)
    -- n.b. startPos is 0-based as it assume C-like array indexing.
    --      So to insert at startPos, we get the substring from
    --      1 to startPos and then append the value to it.
    --      The endPos is also 0-based, and if not provided,
    --      we need to calculate it from the startPos and the len
    --      of the inserted string. The second index is ommitted 
    --      and assume the -1 value, which is the end of the string
    local l = #value
    if l <= 0 then
        return -- no op
    end
    
    local csize = #self.bytes
    
    endPos = (endPos or startPos + l)+1
    
    -- separated for debugging purposes
    -- todo: combine and optimize if possible
    local pre = self.bytes:sub(1,startPos)
    local post = self.bytes:sub(endPos)
    if post then
        self.bytes = pre .. value .. post  
    else 
        self.bytes = pre .. value
    end
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