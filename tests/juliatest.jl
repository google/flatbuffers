import Pkg
Pkg.activate(joinpath(@__DIR__, "..", "julia"))
Pkg.test("FlatBuffers")
