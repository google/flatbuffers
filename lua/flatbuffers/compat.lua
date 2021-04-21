local m
if _VERSION == "Lua 5.3" then
    m = require("flatbuffers.compat_5_3")
else
    local ok = pcall(require, "jit")
    if not ok then
        error("Only Lua 5.3 or LuaJIT is supported")
    else
        m = require("flatbuffers.compat_luajit")
    end
end
return m