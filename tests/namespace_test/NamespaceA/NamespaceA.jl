module NamespaceA
include("NamespaceB/NamespaceB.jl")
include("TableInFirstNS.jl")
include("SecondTableInA.jl")
end
