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
    make = nothing,
    julia = "0.5",
    osname = "linux"
)
