local m = {}

local function replace_char3(pos, str

function m.Get(flag, buf, head)
   return flag:Unpack(buf, head) 
end

function m.Write(flag, buf, head, n)
    local packedData = flag:Pack(n)
    
end

return m