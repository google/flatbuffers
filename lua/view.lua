local m = {}
local mt = {}

local N = require("numTypes")
local encode = require("encode")

function m.New(buf, pos)
    N.UOffsetT:EnforceNumber(pos)
    local o = {
        bytes = buf,
        pos = pos
    }
    setmetatable(o, {__index = mt})
    return o
end

function mt:Offset(vtableOffset)
    local vtable = self.pos - self:Get(N.SOffsetT, self.pos)
    local vtableEnd = self:Get(N.VOffsetT, vtable)
    if vtableOffset < vtableEnd then
        return self:Get(N.VOffsetT, vtable + vtableOffset)
    end
    return 0
end

function mt:Indirect(off)
    N.UOffsetT:EnforceNumber(off)
    return off + encode.Get(N.UOffsetTFlags, self.bytes, off)
end

function mt:String(off)
    N.UOffsetT:EnforceNumber(off)
    off = off + encode.Get(N.UOffsetT, self.bytes, off)
    local start = off + N.UOffsetT.bytewidth
    local length = encode.Get(N.UOffsetT, self.bytes, off)
    return self.bytes:Slice(start, start+length)
end

function mt:VectorLen(off)
    N.UOffsetT:EnforceNumber(off)
    off = off + self.pos
    off = off + encode.Get(N.OffsetT, self.bytes, off)
    return encode.Get(N.UOffsetT, self.bytes, off)
end

function mt:Vector(off)
    N.UOffsetT:EnforceNumber(off)
    
    off = off + self.pos
    local x = off + self:Get(N.UOffsetT, off)
    x = x + N.UOffsetT.bytewidth
    return x
end

function mt:Union(t2, off)
    assert(getmetatable(t2) == mt)
    N.UOffsetT:EnforceNumber(off)
    
    off = off + self.pos
    t2.pos = off + self.Get(N.UOffsetT, off)
    t2.bytes = self.bytes
end

function mt:Get(flags, off)
    N.UOffsetT:EnforceNumber(off)
    return encode.Get(flags, self.bytes, off)
end

function mt:GetSlot(slot, d, validatorFlags)
    N.VOffsetT:EnforceNumber(slot)
    if validatorFlags then
        validatorFlags:EnforceNumber(d)
    end
    local off = self:Offset(slot)
    if off == 0 then
        return d
    end
    return self:Get(validatorFlags, self.pos + off)
end

function mt:GetVOffsetTSlot(slot, d)
    N.VOffsetT:EnforceNumber(slot)
    N.VOffsetT:EnforceNumber(d)
    local off = self:Offset(slot)
    if off == 0 then
        return d
    end
    return off
end

return m