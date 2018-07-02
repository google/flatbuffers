local m = {}

local ba = require("flatbuffers.binaryarray")

function m.Get(flag, buf, head)
   return flag:Unpack(buf, head)
end

function m.Write(flag, buf, head, n)
    buf:Set(flag:Pack(n), head)
end

return m