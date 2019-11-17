# FlatBuffers.jl Documentation

#### Usage

FlatBuffers.jl provides native Julia support for reading and writing binary structures following the google flatbuffer schema (see [here](https://google.github.io/flatbuffers/flatbuffers_internals.html) for a more in-depth review of the binary format).

The typical language support for flatbuffers involves utilizing the `flatc` compiler to translate a flatbuffer schema file (.fbs) into a langugage-specific set of types/classes and methods. See [here](https://google.github.io/flatbuffers/flatbuffers_guide_writing_schema.html) for the official guide on writing schemas.

Currently in Julia, the `flatc` compiler isn't supported, but FlatBuffers.jl provides a native implementation of reading/writing the binary format directly with native Julia types. What does this mean exactly? Basically you can take a schema like:

```
namespace example;

table SimpleType {
  x: int = 1;
}

root_type SimpleType;
```

and do a straightforward Julia translation like:

```julia
module Example

using FlatBuffers

mutable struct SimpleType
    x::Int32
end

@DEFAULT SimpleType x=1

end
```

A couple of things to point out:
* `using FlatBuffers` was included near the top to bring in the FlatBuffers module; this defines the necessary FlatBuffers.jl machinery for making the schema definitions easier
* `int` translates to a Julia `Int32`, see more info on flatbuffer types [here](https://google.github.io/flatbuffers/md__schemas.html)
* A default value for the `x` field in `SimpleType` was declared after the type with the `@DEFAULT` macro
* No `root_type` definition is necessary in Julia; basically any type defined with `type` (i.e. not abstract or immutable) can be a valid root table type in Julia.

So let's see how we can actually use a flatbuffer in Julia:

```julia
using FlatBuffers, Example # the schema module we defined above

val = Example.SimpleType(2) # create an instance of our type

flatbuffer = FlatBuffers.build!(val) # start and build a flatbuffer for our SimpleType
val2 = FlatBuffers.read(flatbuffer) # now we can deserialize the value from our flatbuffer, `val2` == `val`
flatbytes = FlatBuffers.bytes(flatbuffer) # get the serialized bytes of the flatbuffer
val3 = Flatbuffers.read(Example.SimpleType, flatbytes) # now we can deserialize directly from flatbytes
```

For more involved examples, see the test suite [here](https://github.com/dmbates/FlatBuffers.jl/tree/master/test).

#### Reference

Documentation is included inline for each type/method, these can be accessed at the REPL by type `?foo` where `foo` is the name of the type or method you'd like more information on.

List of types/methods:

* `FlatBuffers.Table{T}`: type for deserializing a Julia type `T` from a flatbuffer
* `FlatBuffers.Builder{T}`: type for serializing a Julia type `T` to a flatbuffer
* `FlatBuffers.read`: performs the actual deserializing on a `FlatBuffer.Table`
* `FlatBuffers.build!`: performs the actual serializing on a `FlatBuffer.Builder`
* `@ALIGN T size_in_bytes`: convenience macro for forcing a flatbuffer alignment on the Julia type `T` to `size_in_bytes`
* `@DEFAULT T field1=val1 field2=val2 ...`: convenience macro for defining default field values for Julia type `T`
* `@UNION T Union{T1,T2,...}`: convenience macro for defining a flatbuffer union type `T`
* `@STRUCT immutable T fields... end`: convenience macro for defining flatbuffer struct types, ensuring any necessary padding gets added to the type definition
