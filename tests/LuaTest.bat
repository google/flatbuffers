set buildtype=Release
if "%1"=="-b" set buildtype=%2

..\%buildtype%\flatc.exe --lua -I include_test monster_test.fbs

echo Run with LuaJIT:
luajit.exe luatest.lua
echo Run with Lua 5.3:
lua53.exe luatest.lua