__precompile__(true)
module FlatBuffers

# utils
struct UndefinedType end
const Undefined = UndefinedType()

getfieldvalue(obj::T, i) where {T} = isdefined(obj, i) ? getfield(obj, i) : Undefined

"""
     Scalar
A Union of the Julia types `T <: Number` that are allowed in FlatBuffers schema
"""
const Scalar = Union{UndefinedType, Bool,
                        Int8, Int16, Int32, Int64,
                        UInt8, UInt16, UInt32, UInt64,
                        Float32, Float64}

if VERSION < v"0.7-DEV"
    isconcrete = isleaftype
else
    isconcrete = isconcretetype
end

isstruct(T) = !T.mutable && isconcrete(T)
isbitstype(T) = fieldcount(T) == 0

default(T, TT, sym) = default(TT)
default(::Type{UndefinedType}) = Undefined
default(::Type{T}) where {T <: Scalar} = zero(T)
default(::Type{T}) where {T <: AbstractString} = ""
default(::Type{T}) where {T <: Enum} = enumtype(T)(T(0))
default(::Type{Vector{T}}) where {T} = T[]
# fallback that recursively builds a default; for structs/tables
default(::Type{T}) where {T} = isa(T, Union) ? nothing : T(map(TT->TT == T ? TT() : default(TT),T.types)...)

function typeorder end

enumtype(::Type{<:Enum}) = UInt8

# Types
"""
    Table

The object containing the flatbuffer and positional information specific to the table.
The `vtable` containing the offsets for specific members precedes `pos`.
The actual values in the table follow `pos` offset and size of the vtable.

- `bytes::Vector{UInt8}`: the flatbuffer itself
- `pos::Int`:  the base position in `bytes` of the table
"""
mutable struct Table{T}
    bytes::Vector{UInt8}
    pos::Int
end

"""
Builder is a state machine for creating FlatBuffer objects.
Use a Builder to construct object(s) starting from leaf nodes.

A Builder constructs byte buffers in a last-first manner for simplicity and
performance.
"""
mutable struct Builder{T}
    bytes::Vector{UInt8}
    minalign::Int
    vtable::Vector{Int}
	objectend::Int
	vtables::Vector{Int}
	head::Int
	nested::Bool
	finished::Bool
end

function Base.show(io::IO, x::Union{Builder{T},Table{T}}) where {T}
    println(io, "FlatBuffers.$(typeof(x)): ")
    buffer = typeof(x) <: Table ? x.bytes : x.bytes[x.head+1:end]
    if isempty(buffer)
        print(io, " (empty flatbuffer)")
    else
        pos = Int(typeof(x) <: Table ? x.pos : readbuffer(buffer, 0, Int32))
        # print vtable offset
        syms = T.name.names
        maxpad = max(length(" vtable rel. start pos: "), maximum(map(x->length(string(x)), syms)))
        stringify(buf, x, y, msg) = replace(string(rpad(string(lpad("$(x): ", 6, ' '),lpad(msg, maxpad, ' ')),maxpad+6,' '),string(map(z->lpad(string(Int(z)), 4, ' '),buf[x:y]))[9:end-1]),'"',"")
        println(io, stringify(buffer, 1, 4, " root position: "))
        vtaboff = readbuffer(buffer, pos, Int32)
        vtabstart = pos - vtaboff + 5

        println(io, stringify(buffer, 5, 6, " vtable size: "))
        println(io, stringify(buffer, 7, 8, " data size: "))
        i = vtabstart
        x = 1
        for y = 1:length(syms)
            println(io, stringify(buffer, i, i+1, "$(syms[x]): "))
            i += 2
            x += 1
        end
        # print rel pos. of vtable
        println(io, stringify(buffer, i, i+3, " vtable rel. start pos: "))
        i += 4
        # now we're pointing at data
        while i < length(buffer)
            println(io, stringify(buffer, i, i+3, " "))
            i += 4
        end
    end
end

include("internals.jl")

function Table(::Type{T}, buffer::Vector{UInt8}, pos::Integer) where {T}
    return Table{T}(buffer, pos)
end

Table(b::Builder{T}) where {T} = Table(T, b.bytes[b.head+1:end], get(b, b.head, Int32))

function untilindex(func, itr)
    for (i,x) in enumerate(itr)
        func(x) && return i
    end
    return 0
end

getvalue(t, o, ::Type{Nothing}) = nothing
getvalue(t, o, ::Type{T}) where {T <: Scalar} = get(t, t.pos + o, T)
getvalue(t, o, ::Type{T}) where {T <: Enum} = T(get(t, t.pos + o, enumtype(T)))
function getvalue(t, o, ::Type{T}) where {T <: AbstractString}
    o += get(t, t.pos + o, Int32)
    strlen = get(t, t.pos + o, Int32)
    o += t.pos + sizeof(Int32)
    return String(t.bytes[o + 1:o + strlen])
end
function getvalue(t, o, ::Type{Vector{UInt8}})
    o += get(t, t.pos + o, Int32)
    len = get(t, t.pos + o, Int32)
    o += t.pos + sizeof(Int32)
    return t.bytes[o + 1:o + len] #TODO: maybe not make copy here?
end

getarray(t, vp, len, ::Type{T}) where {T <: Scalar} = (ptr = convert(Ptr{T}, pointer(t.bytes, vp + 1)); return [unsafe_load(ptr, i) for i = 1:len])
getarray(t, vp, len, ::Type{T}) where {T <: Enum} = (ptr = convert(Ptr{enumtype(T)}, pointer(t.bytes, vp + 1)); return [unsafe_load(ptr, i) for i = 1:len])
function getarray(t, vp, len, ::Type{T}) where {T <: Union{AbstractString, Vector{UInt8}}}
    A = Vector{T}(undef, len)
    for i = 1:len
        A[i] = getvalue(t, vp - t.pos, T)
        vp += sizeof(Int32)
    end
    return A
end
function getarray(t, vp, len, ::Type{T}) where {T}
    if isstruct(T)
        ptr = convert(Ptr{T}, pointer(t.bytes, vp + 1))
        return [unsafe_load(ptr, i) for i = 1:len]
    else
        A = Vector{T}(undef, len)
        for i = 1:len
            A[i] = getvalue(t, vp - t.pos, T)
            vp += sizeof(Int32)
        end
        return A
    end
end

function getvalue(t, o, ::Type{Vector{T}}) where {T}
    vl = vectorlen(t, o)
    vp = vector(t, o)
    return getarray(t, vp, vl, T)
end

Base.convert(::Type{T}, e::Integer) where {T <: Enum} = T(e)

# fallback which recursively calls read
function getvalue(t, o, ::Type{T}) where {T}
    if isstruct(T)
        if any(x-> x <: Enum, T.types)
            args = []
            o = t.pos + o + 1
            for typ in T.types
                val = unsafe_load(convert(Ptr{typ <: Enum ? enumtype(typ) : typ}, pointer(view(t.bytes, o:length(t.bytes)))))
                push!(args, val)
                o += sizeof(typ <: Enum ? enumtype(typ) : typ)
            end
            return T(args...)
        else
            return unsafe_load(convert(Ptr{T}, pointer(view(t.bytes, (t.pos + o + 1):length(t.bytes)))))
        end
    else
        newt = Table{T}(t.bytes, t.pos + o + get(t, t.pos + o, Int32))
        return FlatBuffers.read(newt, T)
    end
end

"""
`FlatBuffers.read` parses a `T` at `t.pos` in Table `t`.
Will recurse as necessary for nested types (Arrays, Tables, etc.)
"""
function FlatBuffers.read(t::Table{T1}, ::Type{T}=T1) where {T1, T}
    args = []
    numfields = length(T.types)
    for i = 1:numfields
        TT = T.types[i]
        # if it's a Union type, use the previous arg to figure out the true type that was serialized
        if isa(TT, Union)
            TT = typeorder(TT, args[end])
        end
        o = offset(t, 4 + ((i - 1) * 2))
        if o == 0
            push!(args, default(T, TT, T.name.names[i]))
        else
            push!(args, getvalue(t, o, TT))
        end
    end
    return T(args...)
end

FlatBuffers.read(::Type{T}, buffer::Vector{UInt8}, pos::Integer) where {T} = FlatBuffers.read(Table(T, buffer, pos))
FlatBuffers.read(b::Builder{T}) where {T} = FlatBuffers.read(Table(T, b.bytes[b.head+1:end], get(b, b.head, Int32)))
# assume `bytes` is a pure flatbuffer buffer where we can read the root position at the beginning
FlatBuffers.read(::Type{T}, bytes) where {T} = FlatBuffers.read(T, bytes, read(IOBuffer(bytes), Int32))

"""
    flat_bytes = bytes(b)

`flat_bytes` are the serialized bytes for the FlatBuffer.  This discards the Julia specific `head`.
"""
bytes(b::Builder) = unsafe_wrap(Array{UInt8,1}, pointer(b.bytes, b.head+1), (length(b.bytes)-b.head))

function Builder(::Type{T}=Any, size=0) where {T}
    objectend = 0
    vtables = zeros(Int, 0)
    head = size
    nested = false
    bytes = zeros(UInt8, size)
    minalign = 1
    vtable = zeros(Int, 0)
    finished = false
    b = Builder{T}(bytes, minalign, vtable, objectend,
                vtables, head, nested, finished)
    return b
end

# build!
"`alignment` looks for the largest scalar member of `T` that represents a flatbuffer Struct"
function alignment(::Type{T}) where {T}
    largest = 0
    for typ in T.types
        largest = isbitstype(typ) ? max(largest,sizeof(typ)) : alignment(typ)
    end
    return largest
end

"""
`buildvector!` is for building vectors with all kinds of element types,
even building its elements recursively if needed (Array of Arrays, Array of tables, etc.).
"""
function buildvector! end

# empty vector
function buildvector!(b, A::Vector{Nothing}, len)
    startvector(b, 1, 0, 1)
    return endvector(b, 0)
end
# scalar type vector
function buildvector!(b, A::Vector{T}, len) where {T <: Scalar}
    startvector(b, sizeof(T), len, sizeof(T))
    foreach(x->prepend!(b, A[x]), len:-1:1)
    return endvector(b, len)
end
function buildvector!(b, A::Vector{T}, len) where {T <: Enum}
    startvector(b, sizeof(enumtype(T)), len, sizeof(enumtype(T)))
    foreach(x->prepend!(b, enumtype(T)(A[x])), len:-1:1)
    return endvector(b, len)
end

function putoffsetvector!(b, offsets, len)
    startvector(b, 4, len, 4) #TODO: elsize/alignment correct here?
    foreach(x->prependoffset!(b, offsets[x]), len:-1:1)
    return endvector(b, len)
end
# byte vector vector
function buildvector!(b, A::Vector{Vector{UInt8}}, len)
    offsets = map(x->createbytevector(b, A[x]), 1:len)
    return putoffsetvector!(b, offsets, len)
end
# string vector
function buildvector!(b, A::Vector{T}, len) where {T <: AbstractString}
    offsets = map(x->createstring(b, A[x]), 1:len)
    return putoffsetvector!(b, offsets, len)
end
# array vector
function buildvector!(b, A::Vector{Vector{T}}, len) where {T}
    offsets = map(x->buildbuffer!(b, A[x]), 1:len)
    return putoffsetvector!(b, offsets, len)
end
# struct or table/object vector
function buildvector!(b, A::Vector{T}, len) where {T}
    if isstruct(T)
        # struct
        startvector(b, sizeof(T), len, alignment(T)) #TODO: forced elsize/alignment correct here?
        foreach(x->buildbuffer!(b, A[x]), len:-1:1)
        return endvector(b, len)
    else # table/object
        offsets = map(x->buildbuffer!(b, A[x]), 1:len)
        return putoffsetvector!(b, offsets, len)
    end
end

"""
`getoffset` checks if a given field argument needs to be built
offset (Arrays, Strings, other tables) or can be inlined (Scalar or Struct types).
`getoffset` has a recursive nature in that it will build offset types
down to their last leaf scalar types before returning the highest-level offset.
"""
function getoffset end

getoffset(b, arg::Nothing) = 0
getoffset(b, arg::T) where {T <: Scalar} = 0
getoffset(b, arg::T) where {T <: Enum} = 0
getoffset(b, arg::Vector{UInt8}) = createbytevector(b, arg)
getoffset(b, arg::AbstractString) = createstring(b, arg)
getoffset(b, arg::Vector{T}) where {T} = buildbuffer!(b, arg)
# structs or table/object
getoffset(b, arg::T) where {T} = isstruct(T) ? 0 : buildbuffer!(b, arg)

"""
`putslot!` is one of the final steps in building a flatbuffer.
It puts the final value in the "data" section of the flatbuffer,
whether that be an actual value (for `Scalar`, Struct types) or an offset
to the actual data (Arrays, Strings, other tables)
"""
function putslot! end

putslot!(b, i, arg::T, off) where {T <: Scalar} = prependslot!(b, i, arg, default(T))
putslot!(b, i, arg::T, off) where {T <: Enum} = prependslot!(b, i, enumtype(T)(arg), default(T))
putslot!(b, i, arg::AbstractString, off) = prependoffsetslot!(b, i, off, 0)
putslot!(b, i, arg::Vector{T}, off) where {T} = prependoffsetslot!(b, i, off, 0)
# structs or table/object
putslot!(b, i, arg::T, off) where {T} =
    isstruct(T) ? prependstructslot!(b, i, buildbuffer!(b, arg), 0) : prependoffsetslot!(b, i, off, 0)

function buildbuffer!(b::Builder{T1}, arg::T) where {T1, T}
    if T <: Array
        # array of things
        n = buildvector!(b, arg, length(arg))
    elseif isstruct(T)
        # build a struct type with provided `arg`
        all(isstruct, T.types) || throw(ArgumentError("can't seralize flatbuffer, $T is not a pure struct"))
        align = alignment(T)
        prep!(b, align, 2align)
        for i = length(T.types):-1:1
            typ = T.types[i]
            if typ <: Enum
                prepend!(b, enumtype(typ)(getfield(arg,i)))
            elseif isbitstype(typ)
                prepend!(b, getfield(arg,i))
            else
                buildbuffer!(b, getfield(arg,i))
            end
        end
        n = offset(b)
    else
        # build a table type
        # check for string/array/table types
        numfields = length(T.types)
        offsets = [getoffset(b, getfieldvalue(arg,i)) for i = 1:numfields]

        # all nested have been written, with offsets in `offsets[]`
        startobject(b, numfields)
        foreach(i->putslot!(b, i, getfieldvalue(arg,i), offsets[i]), 1:numfields)
        n = endobject(b)
    end
    return n
end

function build!(b, arg)
    n = buildbuffer!(b, arg)
    finish!(b, n)
    return b
end

build!(arg::T) where {T} = build!(Builder(T), arg)

include("macros.jl")

end # module
