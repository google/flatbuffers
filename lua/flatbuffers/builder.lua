local N = require("flatbuffers.numTypes")
local encode = require("flatbuffers.encode")
local ba = require("flatbuffers.binaryarray")
local compat = require("flatbuffers.compat")

local m = {}

local mt = {}

local MAX_BUFFER_SIZE = 0x80000000 -- 2 GB
local VtableMetadataFields = 2

local getAlignSize = compat.GetAlignSize

local function vtableEqual(a, objectStart, b)
    N.UOffsetT:EnforceNumber(objectStart)
    if (#a * N.VOffsetT.bytewidth) ~= #b then
        return false
    end
    
    for i, elem in ipairs(a) do
        local x = encode.Get(N.VOffsetT, b, i * N.VOffsetT.bytewidth)
        if x == 0 and elem == 0 then
            --no-op
        else
            local y = objectStart - elem
            if x ~= y then
                return false
            end
        end
    end
    return true
end

function m.New(initialSize)
    assert(0 <= initialSize and initialSize < MAX_BUFFER_SIZE)
    local o = 
    {
        finished = false,
        bytes = ba.New(initialSize),
        nested = false,
        head = initialSize,
        minalign = 1,
        vtables = {}
    }
    setmetatable(o, {__index = mt})
    return o
end

function mt:Output()
    assert(self.finished, "Builder Not Finished")
    return self.bytes:Slice(self:Head())
end

function mt:StartObject(numFields)
    assert(not self.nested)
    
    local vtable = {}

    for _=1,numFields do
        table.insert(vtable, 0)
    end
            
    self.currentVTable = vtable
    self.objectEnd = self:Offset()
    self.minalign = 1
    self.nested = true
end

function mt:WriteVtable()
    self:PrependSOffsetTRelative(0)
    local objectOffset = self:Offset()
    
    local exisitingVTable
    local i = #self.vtables
    while i >= 1 do
        if self.vtables[i] == 0 then
            table.remove(self.vtables,i)
        end
        i = i - 1
    end    
   
    while i >= 1 do
        
        local vt2Offset = self.vtables[i]
        local vt2Start = #self.bytes - vt2Offset
        local vt2Len = encode.Get(N.VOffsetT, self.bytes, vt2Start)
        
        local metadata = VtableMetadataFields * N.VOffsetT.bytewidth
        local vt2End = vt2Start + vt2Len
        local vt2 = self.bytes:Slice(vt2Start+metadata,vt2End)
        
        if vtableEqual(self.currentVTable, objectOffset, vt2) then
            exisitingVTable = vt2Offset
            break
        end
        
        i = i - 1
    end
    
    if not exisitingVTable then
        i = #self.currentVTable
        while i >= 1 do
            local off = 0
            local a = self.currentVTable[i]
            if a and a ~= 0 then
                off = objectOffset - a
            end
            self:PrependVOffsetT(off)
            
            i = i - 1
        end
        
        local objectSize = objectOffset - self.objectEnd
        self:PrependVOffsetT(objectSize)
        
        local vBytes = #self.currentVTable + VtableMetadataFields
        vBytes = vBytes * N.VOffsetT.bytewidth
        self:PrependVOffsetT(vBytes)
        
        local objectStart = #self.bytes - objectOffset
        encode.Write(N.SOffsetT, self.bytes, objectStart, self:Offset() - objectOffset)
        
        table.insert(self.vtables, self:Offset())
    else
        local objectStart = #self.bytes - objectOffset
        self.head = objectStart
        
        encode.Write(N.SOffsetT, self.bytes, self:Head(), exisitingVTable - objectOffset)
    end
    
    self.currentVTable = nil
    return objectOffset
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
    
    self.bytes:Grow(newsize)
end

function mt:Head()
    return self.head
end

function mt:Offset()
   return #self.bytes - self:Head()
end

function mt:Pad(n)
    for _=1,n do
       self:Place(0, N.Uint8)
    end
end

function mt:Prep(size, additionalBytes)
    if size > self.minalign then
        self.minalign = size
    end

    local alignsize = getAlignSize(self, size, additionalBytes)

    while self:Head() < alignsize + size + additionalBytes do
        local oldBufSize = #self.bytes
        growByteBuffer(self)
        local updatedHead = self.head + #self.bytes - oldBufSize
        self.head = updatedHead
    end
    
    self:Pad(alignsize)
end

function mt:PrependSOffsetTRelative(off)
    self:Prep(N.SOffsetT.bytewidth, 0)
    assert(off <= self:Offset(), "Offset arithmetic error: ".. off .. " > ".. self:Offset())
    local off2 = self:Offset() - off + N.SOffsetT.bytewidth
    self:PlaceSOffsetT(off2)
end

function mt:PrependUOffsetTRelative(off)
    self:Prep(N.UOffsetT.bytewidth, 0)
    assert(off <= self:Offset(), "Offset arithmetic error: ".. off .. " > ".. self:Offset())
    local off2 = self:Offset() - off + N.UOffsetT.bytewidth
    self:PlaceUOffsetT(off2)
end

function mt:StartVector(elemSize, numElements, alignment)
    assert(not self.nested)
    self.nested = true
    self:Prep(N.Uint32.bytewidth, elemSize * numElements)
    self:Prep(alignment, elemSize * numElements)
    return self:Offset()
end

function mt:EndVector(vectorNumElements)
    assert(self.nested)
    self.nested = false
    self:PlaceUOffsetT(vectorNumElements)
    return self:Offset()
end

function mt:CreateString(s)
    assert(not self.nested)
    self.nested = true
    
    assert(type(s) == "string")
    
    self:Prep(N.UOffsetT.bytewidth, (#s + 1)*N.Uint8.bytewidth)
    self:Place(0, N.Uint8)
    
    local l = #s
    self.head = self:Head() - l
    
    self.bytes:Set(s, self:Head(), self:Head() + l)
    
    return self:EndVector(#s)
end

function mt:CreateByteVector(x)
    assert(not self.nested)
    self.nested = true
    self:Prep(N.UOffsetT.bytewidth, #x*N.Uint8.bytewidth)
    
    local l = #x
    self.head = self:Head() - l
    
    self.bytes:Set(x, self:Head(), self:Head() + l)
    
    return self:EndVector(#x)
end

function mt:Slot(slotnum)
    assert(self.nested)
    -- n.b. slot number is 0-based
    self.currentVTable[slotnum+1] = self:Offset()
end

local function finish(self, rootTable, sizePrefix)
    N.UOffsetT:EnforceNumber(rootTable)
    local prepSize = N.UOffsetT.bytewidth
    if sizePrefix then
        prepSize = prepSize + N.Int32.bytewidth
    end
    
    self:Prep(self.minalign, prepSize)
    self:PrependUOffsetTRelative(rootTable)
    if sizePrefix then
        local size = #self.bytes - self:Head()
        N.Int32:EnforceNumber(size)
        self:PrependInt32(size)
    end
    self.finished = true
    return self:Head()
end

function mt:Finish(rootTable)
    return finish(self, rootTable, false)
end

function mt:FinishSizePrefixed(rootTable)
    return finish(self, rootTable, true)
end

function mt:Prepend(flags, off)
    self:Prep(flags.bytewidth, 0)
    self:Place(off, flags)
end

function mt:PrependSlot(flags, o, x, d)
    flags:EnforceNumber(x)
    flags:EnforceNumber(d)
    if x ~= d then
        self:Prepend(flags, x)
        self:Slot(o)
    end
end

function mt:PrependBoolSlot(...)    self:PrependSlot(N.Bool, ...) end
function mt:PrependByteSlot(...)    self:PrependSlot(N.Uint8, ...) end
function mt:PrependUint8Slot(...)   self:PrependSlot(N.Uint8, ...) end
function mt:PrependUint16Slot(...)  self:PrependSlot(N.Uint16, ...) end
function mt:PrependUint32Slot(...)  self:PrependSlot(N.Uint32, ...) end
function mt:PrependUint64Slot(...)  self:PrependSlot(N.Uint64, ...) end
function mt:PrependInt8Slot(...)    self:PrependSlot(N.Int8, ...) end
function mt:PrependInt16Slot(...)   self:PrependSlot(N.Int16, ...) end
function mt:PrependInt32Slot(...)   self:PrependSlot(N.Int32, ...) end
function mt:PrependInt64Slot(...)   self:PrependSlot(N.Int64, ...) end
function mt:PrependFloat32Slot(...) self:PrependSlot(N.Float32, ...) end
function mt:PrependFloat64Slot(...) self:PrependSlot(N.Float64, ...) end

function mt:PrependUOffsetTRelativeSlot(o,x,d)
    if x~=d then
        self:PrependUOffsetTRelative(x)
        self:Slot(o)
    end
end

function mt:PrependStructSlot(v,x,d)
    N.UOffsetT:EnforceNumber(d)
    if x~=d then
        N.UOffsetT:EnforceNumber(x)
        assert(x == self:Offset(), "Tried to write a Struct at an Offset that is different from the current Offset of the Builder.")
        self:Slot(v)
    end
end

function mt:PrependBool(x)      self:Prepend(N.Bool, x) end
function mt:PrependByte(x)      self:Prepend(N.Uint8, x) end
function mt:PrependUint8(x)     self:Prepend(N.Uint8, x) end
function mt:PrependUint16(x)    self:Prepend(N.Uint16, x) end
function mt:PrependUint32(x)    self:Prepend(N.Uint32, x) end
function mt:PrependUint64(x)    self:Prepend(N.Uint64, x) end
function mt:PrependInt8(x)      self:Prepend(N.Int8, x) end
function mt:PrependInt16(x)     self:Prepend(N.Int16, x) end
function mt:PrependInt32(x)     self:Prepend(N.Int32, x) end
function mt:PrependInt64(x)     self:Prepend(N.Int64, x) end
function mt:PrependFloat32(x)   self:Prepend(N.Float32, x) end
function mt:PrependFloat64(x)   self:Prepend(N.Float64, x) end
function mt:PrependVOffsetT(x)  self:Prepend(N.VOffsetT, x) end

function mt:Place(x, flags)
    flags:EnforceNumber(x)
    self.head = self:Head() - flags.bytewidth
    encode.Write(flags, self.bytes, self.head, x)
end

function mt:PlaceVOffsetT(x) self:Place(x, N.VOffsetT) end
function mt:PlaceSOffsetT(x) self:Place(x, N.SOffsetT) end
function mt:PlaceUOffsetT(x) self:Place(x, N.UOffsetT) end

return m