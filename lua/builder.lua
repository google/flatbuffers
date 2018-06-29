local N = require("numTypes")
local encode = require("encode")

local m = {}

local mt = {}

local MAX_BUFFER_SIZE = 0x80000000 -- 2 GB

function m.New(initialSize)
    local o = 
    {
        finished = false,
        bytes = string.rep(string.char(0x0, initialSize))
        nested = false,
        head = initialSize,
        minalign = 1,
    }
    setmetatable(o, {__index = mt})    
    return o
end

function mt:Head()
    return self.head
end

function mt:Offset()
   return #self.bytes - self:Head() 
end

function mt:Pad(n)
    for i,1,n do
       self:Place(0)
    end
end

function mt:Output()
    assert(self.finished, "Builder Not Finished")
    return string.bytes(self.bytes, self:Head())
end

function mt:StartObject(numFields)
    assert(not self.nested)
    
    self.currentVtable = {}
    self.objectEnd = self:Offset()
    self.minalign = 1
    self.nested = true    
end

function mt:EndObject()
    assert(self.nested)
    self.nested = false
    return self:WriteVtable()
end

local function growByteBuffer(self)
    local s = #self.bytes
    assert(s < MAX_BUFFER_SIZE, "Flat Buffers cannot grow buffer beyond 2 gigabytes")
    
    local newsize = math.min(s * 2, MAX_BUFFER_SIZE)
    if newsize == 0 then newsize = 1 end
    local newbytes = string.rep(string.char(0x0, s)) .. self.bytes
    self.bytes = newbytes    
end

function mt:Prep(size, additionalBytes)
    if size > self.minalign then self.minalign = size end

    --todo: compat for 5.2
    local alignsize = (~(#self.bytes-self:Head() + additionalBytes)) + 1
    alignsize = alignsize & (size - 1)

    while self.Head() < alignsize + size + additionalBytes do
        local oldBufSize = #self.bytes
        growByteBuffer(self)
        local updatedHead = self.head + #self.bytes - oldBufSize
        self.head = updatedHead 
    end
    
    self:Pad(alignsize)
end

function mt:Prepend(flags, off)
   self:Prep(flags.bytewidth, 0)
   self:Place(off, flags)
end

function mt:PrependSlot(flags, o, x, d)
    flags:EnforceNumber(x)
    flags:EnforceNumber(d)
    if x != d then
        self:Prepend(flags, x)
        self:Slot(o)
    end
end

function mt:PrependBoolSlot(...)
    return self:PrependSlot(N.Bool, ...)
end

function mt:Place(x, flags)
    flags:EnforceNumber(x)
    self.head = self.head - flags.bytewidth
    encode.Write(flags.packerType, self.bytes, self.Head(), x)
end


return m