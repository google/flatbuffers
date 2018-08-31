Use in Julia    {#flatbuffers_guide_use_julia}
==============
## Before you get started
Before diving into the FlatBuffers usage in Julia, it should be noted that the
[Tutorial](@ref flatbuffers_guide_tutorial) page has a complete guide to general
FlatBuffers usage in all of the supported languages (including Julia). This
page is designed to cover the nuances of FlatBuffers usage, specific to
Julia.

You should also have read the [Building](@ref flatbuffers_guide_building)
documentation to build `flatc` and should be familiar with
[Using the schema compiler](@ref flatbuffers_guide_using_schema_compiler) and
[Writing a schema](@ref flatbuffers_guide_writing_schema).

## FlatBuffers Julia package
The `flatc` backend for Julia makes heavy use of the [FlatBuffers.jl package](https://github.com/JuliaData/FlatBuffers.jl).
Documentation for this package may be found [here](http://juliadata.github.io/FlatBuffers.jl/stable).
You can also browse the source code on the [FlatBuffers.jl GitHub page](https://github.com/JuliaData/FlatBuffers.jl).
 
## Testing the FlatBuffers Julia library
The code to test the Julia library can be found at [FlatBuffers.jl/test/flatc.jl](https://github.com/JuliaData/FlatBuffers.jl/tree/master/test/flatc.jl). To run the tests, run `julia test/flatc.jl` in the top-level directory.
To obtain Julia itself, go to the [Julia homepage](http://julialang.org)
for binaries or source code for your platform.

## Using the FlatBuffers Julia library
*Note: See [Tutorial](@ref flatbuffers_guide_tutorial) for a more in-depth
example of how to use FlatBuffers in Julia.*

There is support for both reading and writing FlatBuffers in Julia.
To use FlatBuffers in your own code, first generate Julia modules from your
schema with the `--julia` option to `flatc`. Then you can include both
FlatBuffers and the generated code to read or write a FlatBuffer.

 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.julia}
    import FlatBuffers

    include("MyGame/MyGame.jl")
    import .MyGame.Example.Monster

    monster = open("monsterdata_test.mon", "r") do f Monster(f) end
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 Now you can access values like this:
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.julia}
    hp = monster.hp
    pos = monster.pos
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
More detailed documentation for using the FlatBuffers Julia package
may be found [here](http://juliadata.github.io/FlatBuffers.jl/stable).

## Speed
The FlatBuffers Julia package has not been thoroughly optimized for speed.
For example, instead of referring to memory in-place, values are instead
loaded with `unsafe_load`. It does however support deduplication of vtables, which
saves roughly 30% of time spent writing, when compared to writing objects
with duplicate vtables.

 <br>