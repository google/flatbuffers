import Pkg
Pkg.instantiate()
using Documenter, FlatBuffers

makedocs(
    modules = [FlatBuffers],
    format = :html,
    sitename = "FlatBuffers.jl",
    pages = ["Home" => "index.md"]
)

deploydocs(
    repo = "github.com/JuliaData/FlatBuffers.jl.git",
    target = "build",
    deps = nothing,
    make = nothing
)
