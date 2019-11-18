using Test
import FlatBuffers

# generated code
include(joinpath(@__DIR__, "..", "..", "tests", "monster_test_generated.jl"))
import .MyGame
import .MyGame.Example
import .MyGame.Example2
import .MyGame.Example.Any_
import .MyGame.Example.Monster
import .MyGame.Example.TestSimpleTableWithEnum

loadmonsterfile(filename) = open(joinpath(@__DIR__, filename), "r") do f Monster(f) end

function checkmonster(monster)
    @test monster.hp == 80
    @test monster.mana == 150
    @test monster.name == "MyMonster"

    vec = monster.pos

    @test vec.x == 1.0
    @test vec.y == 2.0
    @test vec.z == 3.0
    @test vec.test1 == 3.0
    @test vec.test2 == MyGame.Example.ColorGreen
    @test vec.test3_a == 5
    @test vec.test3_b == 6

    monster2 = monster.test
    @test monster2.name == "Fred"

    @test length(monster.inventory) == 5
    @test sum(monster.inventory) == 10

    @test monster.vector_of_longs == [10 ^ (2*i) for i = 0:4]
    @test monster.vector_of_doubles == [-1.7976931348623157e+308, 0, 1.7976931348623157e+308]

    @test length(monster.test4) == 2

    (test0, test1) = monster.test4
    @test sum([test0.a, test0.b, test1.a, test1.b]) == 100

    @test monster.testarrayofstring == ["test1", "test2"]
    @test monster.testarrayoftables == []
    @test monster.testf == 3.14159f0
end

function checkpassthrough(monster)
    b = FlatBuffers.Builder(Monster)
    FlatBuffers.build!(b, monster)
    bytes = FlatBuffers.bytes(b)
    @test FlatBuffers.has_identifier(Monster, bytes)
    newmonster = FlatBuffers.read(Monster, bytes)
    checkmonster(newmonster)
end

function checkserialize(monster)
    io = IOBuffer()
    FlatBuffers.serialize(io, monster)
    bytes = take!(io)
    newmonster = FlatBuffers.deserialize(IOBuffer(bytes), Monster)
    checkmonster(newmonster)
end

@test FlatBuffers.root_type(Monster) == true
@test FlatBuffers.file_identifier(Monster) == "MONS"
@test FlatBuffers.file_extension(Monster) == "mon"

for testcase in ["test", "python_wire"]
    mon = loadmonsterfile("monsterdata_$testcase.mon")
    checkmonster(mon)
    checkpassthrough(mon)
    checkserialize(mon)
end

# test printing
mon = loadmonsterfile("monsterdata_test.mon")
b = FlatBuffers.Builder(Monster)
FlatBuffers.build!(b, mon)
io = IOBuffer()
show(io, b)
output = String(take!(io))
@test occursin("deprecated field", split(output, "\n")[9])

