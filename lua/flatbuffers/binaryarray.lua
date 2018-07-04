local m = {}

local mt = {}

function mt:__len()
    return self.size
end

function m.New(size)
    local o = 
    {
        data = {},
        size = size
    }
    setmetatable(o, {__index = mt, __len = mt.__len})
    return o
end

function m.FromString(str)
    local o = 
    {
        str = str,
        size = #str
    }
    setmetatable(o, {__index = mt, __len = mt.__len})
    return o    
end

function mt:Slice(startPos, endPos)
    startPos = startPos or 0
    endPos = endPos or self.size
    local d = self.data
    if d then      
        
        -- new table to store the slice components
        local b = {}        
        
        -- starting with the startPos, put all
        -- values into the new table to be concat later
        -- updated the startPos based on the size of the
        -- value
        while startPos < endPos do
            local v = d[startPos] or '/0'
            table.insert(b, v)
            startPos = startPos + #v
        end

        -- combine the table of strings into one string
        return table.concat(b)   
    else
        -- n.b start/endPos are 0-based incoming, so need to convert
        --     correctly. in python a slice includes start -> end - 1
        return self.str:sub(startPos+1, endPos)
    end
end

function mt:Grow(newsize)
    -- the new table to store the data
    local newT = {}
    
    -- the offset to be applied to existing entries
    local offset = newsize - self.size
    
    -- loop over all the current entries and
    -- add them to the new table at the correct
    -- offset location
    local d = self.data    
    for i,data in pairs(d) do       
        newT[i + offset] = data
    end
    
    -- update this storage with the new table and size
    self.data = newT    
    self.size = newsize
end

-- memorization
local pads = {}
function mt:Pad(n, startPos)   
    local s = pads[n] 
    if not s then
        s = string.rep('\0', n)
        pads[n] = s
    end
    self.data[startPos] = s    
end

function mt:Set(value, startPos, endPos)    
    self.data[startPos] = value    
end

-- locals for slightly faster access
local sunpack = string.unpack
local spack = string.pack

function m.Pack(fmt, ...)
    return spack(fmt, ...)
end

function m.Unpack(fmt, s, pos)
    return sunpack(fmt, s.str, pos + 1)
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