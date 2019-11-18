using FlatBuffers
using Test
import Parameters

# test default fields
@with_kw mutable struct UltimateAnswer
    answer::Int32 = 42
    question::String
    highwaysbuilt::Int32 = 7
end

x = UltimateAnswer(;question="How many roads must a man walk down?")
@test x.answer == 42
@test FlatBuffers.default(UltimateAnswer, Int32, :answer) == 42
@test FlatBuffers.default(UltimateAnswer, Int32, :highwaysbuilt) == 7
b = FlatBuffers.Builder(UltimateAnswer)
FlatBuffers.build!(b, x)
xbytes = FlatBuffers.bytes(b)
y = FlatBuffers.read(UltimateAnswer, xbytes)

@test y.answer == x.answer
@test y.question == x.question
@test y.highwaysbuilt == x.highwaysbuilt
@test x.highwaysbuilt == 7

y = Parameters.reconstruct(x, highwaysbuilt = 0)
b = FlatBuffers.Builder(UltimateAnswer)
FlatBuffers.build!(b, y)
ybytes = FlatBuffers.bytes(b)

# check that we save bytes with default integer values
@test length(ybytes) > length(xbytes)

@test y.answer == x.answer
@test y.question == x.question
@test y.highwaysbuilt == 0

y = FlatBuffers.read(UltimateAnswer, ybytes)
@test y.highwaysbuilt == 0

