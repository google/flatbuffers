os.execute([[.\..\Debug\flatc.exe --lua -I include_test monster_test.fbs]])

package.path = string.format("../lua/?.lua;./?.lua;%s",package.path)

local function checkReadBuffer(buf, offset)
    offset = offset or 0
    
    local monster = require("MyGame.Example.Monster")
    
    local mon = monster.GetRootAsMonster(buf, offset)
    assert(mon:Hp() == 80, "Monster Hp is not 80")
    assert(mon:Mana() == 150, "Monster Mana is not 150")
    assert(mon:Name() == "MyMonster", "Monster Name is not MyMonster")
    
    local vec = assert(mon:Pos(), "Monster Position is nil")
    assert(vec:X() == 1.0)
    assert(vec:Y() == 2.0)
    assert(vec:Z() == 3.0)
    assert(vec:Test1() == 3.0)
    assert(vec:Test2() == 2)
    
    local t = require("MyGame.Example.Test").New()
    t = assert(vec:Test3(t))
    
    assert(t:A() == 5)
    assert(t:B() == 6)
end

local function testWire(flatbuffers)
    local f = assert(io.open('monsterdata_test.mon', 'rb'))
    local wireData = f:read("*a")
    f:close()
    
    checkReadBuffer(wireData)  
end
    
    
local tests = {
    testWire,
}

local result, err = xpcall(function()
    local flatbuffers = require("flatbuffers")

    assert(flatbuffers, "Cannot load flatbuffer Lua module")
    
    local testsPassed, testsFailed = 0,0
    for _,testFunc in ipairs(tests) do
        local results, err = pcall(testFunc, flatbuffers)        
        if results then
            testsPassed = testsPassed + 1
        else
			testsFailed = testsFailed + 1
            print(string.format(" Test failed: %s", err)) 
        end
    end
    
    local totalTests = testsPassed + testsFailed
    print(string.format("# of test passed: %d / %d (%.2f%%)",
        testsPassed, 
        totalTests, 
        totalTests ~= 0 
            and 100 * (testsPassed / totalTests) 
            or 0)
        )
    
    return 0
end, debug.traceback)

if not result then
    print("Unable to run tests due to test framework error: ",err)
end

os.exit(result or -1)
