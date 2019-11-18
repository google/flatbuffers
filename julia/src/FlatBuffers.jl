module FlatBuffers

# utils
"""
serialize(stream::IO, value::T) where {T}
Serialize `value` to `stream` using the `FlatBuffer` format.
"""
function serialize(stream::IO, value::T) where {T}
	write(stream, bytes(build!(value)))
end

"""
deserialize(stream::IO, ::Type{T}) where {T}
Read a `T` from the flatbuffer-formatted `stream`.
"""
function deserialize(stream::IO, ::Type{T}) where {T}
	read(T, read(stream))
end

struct UndefinedType end
const Undefined = UndefinedType()
getfieldvalue(obj::T, i) where {T} = isdefined(obj, i) ? getfield(obj, i) : Undefined
getprevfieldvalue(obj::T, i) where {T} = i == 1 ? missing : getfieldvalue(obj, i - 1)

"""
Scalar
A Union of the Julia types `T <: Number` that are allowed in FlatBuffers schema
"""
const Scalar = Union{Bool,
Int8, Int16, Int32, Int64,
UInt8, UInt16, UInt32, UInt64,
Float32, Float64}

isstruct(T) = isconcretetype(T) && !T.mutable
isbitstype(T) = fieldcount(T) == 0
isunionwithnothing(T) = T isa Union && T.a == Nothing && !(isa(T.b, Union))

file_identifier(T) = ""
file_extension(T) = ""
slot_offsets(T) = [4 + ((i - 1) * 2) for i = 1:length(T.types)]

default(T, TT, sym) = default(TT)
default(::Type{T}) where {T <: Scalar} = zero(T)
default(::Type{T}) where {T <: AbstractString} = ""
default(::Type{T}) where {T <: Enum} = enumtype(T)(T(0))
default(::Type{Vector{T}}) where {T} = T[]

# attempt to call default constructors for the type,
# use above methods as fallback
function default(::Type{T}, i::Integer) where {T}
	TT = T.types[i]
	try
		return FlatBuffers.default(T, TT, fieldnames(T)[i])
		# catch because Parameters throws an error if there is no
		# default value defined...
	catch
	end
	return default(TT)
end

# fallback that recursively builds a default; for structs/tables
function default(::Type{T}) where {T}
	if isa(T, Union) || isa(T, UnionAll)
		return nothing
	else
		return T([default(T, i) for i = 1:length(T.types)]...)
	end
end

function typeorder end

enumtype(::Type{<:Enum}) = UInt8

# Types
"""
Table

The object containing the flatbuffer and positional information specific to the table.
The `vtable` containing the offsets for specific members precedes `pos`.
The actual values in the table follow `pos` offset and size of the vtable.

- `bytes::AbstractVector{UInt8}`: the flatbuffer itself
- `pos::Integer`:  the base position in `bytes` of the table
"""
mutable struct Table{T}
	bytes::AbstractVector{UInt8}
	pos::Integer
end

"""
Builder is a state machine for creating FlatBuffer objects.
Use a Builder to construct object(s) starting from leaf nodes.

A Builder constructs byte buffers in a last-first manner for simplicity and
performance.
"""
mutable struct Builder{T}
	bytes::AbstractVector{UInt8}
	minalign::Int
	vtable::Vector{Int}
	objectend::Int
	vtables::Vector{Int}
	head::Int
	nested::Bool
	finished::Bool
end

function hexloc(x)
	"0x" * lpad("$(string(x-1, base=16)) ", 6, '0')
end

function hexbyte(io, z)
	printstyled(io, lpad("$(string(z, base=16)) ", 3, '0'), color=Int(z))
end

function hexoffset(x)
	"0x$(lpad(string(x, base=16), 4, '0'))"
end

function stringify(io, buf, offset, x, y, msg="", msgcolor=:blue)
	y = min(y, length(buf))
	printstyled(io, hexloc(x + offset), color=:blue)
	for i = x:y
		hexbyte(io, buf[i])
	end
	if length(msg) > 0
		printstyled(io, " " * msg, color=msgcolor)
	end
	println(io)
end

function showvtable(io::IO, T, buffer, vtabstart, vtabsize)
	syms = T.name.names
	printstyled(io, "vtable start pos: $(hexoffset(vtabstart))\n", color=:green)
	printstyled(io, "vtable size: $vtabsize\n", color=:green)
	i = vtabstart + 4
	soff = slot_offsets(T)
    numslots = div(soff[end] - 4, 2) + 1
	field = 1
	slot = 1
	numfields = length(T.types)
	while slot <= numslots
		# leave holes for deprecated fields
		j = 2
		start = field == 1 ? soff[1] : soff[field - 1]
		while (start + j) < soff[field]
			# empty slot
			stringify(io, buffer, 1, i, i+1, "[deprecated field]", :red)
			slot += 1
			j += 2
			i += 2
			if (i - vtabstart) > vtabsize
				break
			end
		end
		if (i - vtabstart) > vtabsize
			break
		end
		stringify(io, buffer, 1, i, i+1, "[$(fieldnames(T)[field])]")
		slot += 1
		field += 1
		i += 2
		if (i - vtabstart) > vtabsize
			break
		end
	end
	# now we're pointing at data
	printstyled(io, "payload:\n", color=:green)
	while i < length(buffer)
		stringify(io, buffer, 1, i, i+7, "")
		i += 8
	end
end

function Base.show(io::IO, x::Union{Builder{T}, Table{T}}) where {T}
	printstyled(io, "FlatBuffers.$(typeof(x)):\n", color=:green)
	buffer = x isa Builder ? x.bytes[x.head+1:end] : x.bytes
	if isempty(buffer)
		printstyled(io, " (empty flatbuffer)", color=:red)
	else
		pos = Int(typeof(x) <: Table ? x.pos : readbuffer(buffer, 0, Int32))
		printstyled(io, "root offset: $(hexoffset(pos))\n", color=:green)
		vtaboff = readbuffer(buffer, pos, Int32)
		vtabstart = pos - vtaboff
		vtabsize = readbuffer(buffer, vtabstart, Int16)
		showvtable(io, T, buffer, vtabstart, vtabsize)
	end
end

include("internals.jl")

function Table(::Type{T}, buffer::AbstractVector{UInt8}, pos::Integer) where {T}
	return Table{T}(buffer, pos)
end

Table(b::Builder{T}) where {T} = Table(T, b.bytes[b.head+1:end], get(b, b.head, Int32))

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
		o += t.pos
		newt = Table{T}(t.bytes, indirect(t, o))
		return FlatBuffers.read(newt, T)
	end
end

function typetoread(prevfield, ::Type{T}, ::Type{TT}) where {T, TT}
    R = TT
    nullable = false
    if isunionwithnothing(R)
        nullable = true
        R = TT.b
    end

    # if it's a Union type, use the previous arg to figure out the true type that was serialized
    if !isunionwithnothing(R) && R isa Union
        R = typeorder(R, prevfield)
    end

    if R <: AbstractVector
        # hacks! if it's a union all, assume it's because we're working around circular dependencies
        if isa(eltype(R), UnionAll)
            return Vector{T}, false, nullable
        # if it's a vector of Unions, use the previous field to figure out the types of all the elements
        elseif isa(eltype(R), Union)
            types = typeorder.(eltype(R), prevfield)
            R = definestruct(types)
            return R, true, nullable
        end
    end
    return R, false, nullable
end

"""
`FlatBuffers.read` parses a `T` at `t.pos` in Table `t`.
Will recurse as necessary for nested types (Arrays, Tables, etc.)
"""
function FlatBuffers.read(t::Table{T1}, ::Type{T}=T1) where {T1, T}
	args = []
	numfields = length(T.types)
	soff = slot_offsets(T)
	for i = 1:numfields
		TT = T.types[i]
		o = offset(t, soff[i])
        R, isunionvector, nullable = typetoread(i == 1 ? nothing : args[end], T, TT)
        if o == 0
            push!(args, nullable ? nothing : default(T, TT, T.name.names[i]))
        else
            if isunionvector
                eval(:(newr = getvalue($t, $o, $R)))
                eval(:(n = length($R.types)))
                push!(args, [getfieldvalue(newr, j) for j = 1:n]) 
            else
                push!(args, getvalue(t, o, R))
            end
        end
    end

	return T(args...)
end

FlatBuffers.read(::Type{T}, buffer::AbstractVector{UInt8}, pos::Integer) where {T} = FlatBuffers.read(Table(T, buffer, pos))
FlatBuffers.read(b::Builder{T}) where {T} = FlatBuffers.read(Table(T, b.bytes[b.head+1:end], get(b, b.head, Int32)))
# assume `bytes` is a pure flatbuffer buffer where we can read the root position at the beginning
FlatBuffers.read(::Type{T}, bytes) where {T} = FlatBuffers.read(T, bytes, read(IOBuffer(bytes), Int32))

has_identifier(::Type{T}, bytes) where {T} = length(bytes) >= 8 && String(bytes[5:8]) == FlatBuffers.file_identifier(T)
root_type(::Type{T}) where {T} = false

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
function buildvector!(b, A::Vector{Nothing}, len, prev)
	startvector(b, 1, 0, 1)
	return endvector(b, 0)
end
# scalar type vector
function buildvector!(b, A::Vector{T}, len, prev) where {T <: Scalar}
	startvector(b, sizeof(T), len, sizeof(T))
	foreach(x->prepend!(b, A[x]), len:-1:1)
	return endvector(b, len)
end
function buildvector!(b, A::Vector{T}, len, prev) where {T <: Enum}
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
function buildvector!(b, A::Vector{Vector{UInt8}}, len, prev)
	offsets = map(x->createbytevector(b, A[x]), 1:len)
	return putoffsetvector!(b, offsets, len)
end
# string vector
function buildvector!(b, A::Vector{T}, len, prev) where {T <: AbstractString}
	offsets = map(x->createstring(b, A[x]), 1:len)
	return putoffsetvector!(b, offsets, len)
end
# array vector
function buildvector!(b, A::Vector{Vector{T}}, len, prev) where {T}
	offsets = map(x->buildbuffer!(b, A[x]), 1:len)
	return putoffsetvector!(b, offsets, len)
end

# make a new struct which has fields of the given type
function definestruct(types::Vector{DataType})
	fields = [:($(gensym())::$(TT)) for TT in types]
	T1 = gensym()
	eval(:(mutable struct $T1
		$(fields...)
	end))
	return T1
end

# make a new struct which has fields of the given type
# and populate them with values from the vector
function createstruct(types::Vector{DataType}, A::Vector{T}) where {T}
	T1 = definestruct(types)
	eval(:(newt = $T1($(A...))))
	return newt
end

# struct or table/object vector
function buildvector!(b, A::Vector{T}, len, prev) where {T}
	if isstruct(T)
		# struct
		startvector(b, sizeof(T), len, alignment(T)) #TODO: forced elsize/alignment correct here?
		foreach(x->buildbuffer!(b, A[x]), len:-1:1)
		return endvector(b, len)
	elseif isa(T, Union)
		types = typeorder.(T, prev)

		# define a new type, construct one, and pack it into the buffer
		newt = createstruct(types, A)
		buildbuffer!(b, newt)
	else
		# table/object
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

getoffset(b, arg::Nothing, prev=nothing) = 0
getoffset(b, arg::T, prev=nothing) where {T <: Scalar} = 0
getoffset(b, arg::T, prev=nothing) where {T <: Enum} = 0
getoffset(b, arg::AbstractString, prev=nothing) = createstring(b, arg)
getoffset(b, arg::Vector{UInt8}, prev) = createbytevector(b, arg)
getoffset(b, arg::Vector{T}, prev) where {T} = buildbuffer!(b, arg, prev)

# structs or table/object
getoffset(b, arg::T, prev=nothing) where {T} = isstruct(T) ? 0 : buildbuffer!(b, arg, prev)

"""
`putslot!` is one of the final steps in building a flatbuffer.
It puts the final value in the "data" section of the flatbuffer,
whether that be an actual value (for `Scalar`, Struct types) or an offset
to the actual data (Arrays, Strings, other tables)
"""
function putslot! end

putslot!(b, i, arg::T, off, default, prev) where {T <: Scalar} = prependslot!(b, i, arg, default)
putslot!(b, i, arg::T, off, default, prev) where {T <: Enum} = prependslot!(b, i, enumtype(T)(arg), default)
putslot!(b, i, arg::AbstractString, off, default, prev) = prependoffsetslot!(b, i, off, 0)
putslot!(b, i, arg::Vector{T}, off, default, prev) where {T} = prependoffsetslot!(b, i, off, 0)
# structs or table/object
function putslot!(b, i, arg::T, off, default, prev) where {T}
	if isstruct(T)
		prependstructslot!(b, i, buildbuffer!(b, arg, prev), 0)
	else
		prependoffsetslot!(b, i, off, 0)
	end
end

function needreconstruct(T)
	for TT in T.types 
        if TT <: Vector && eltype(TT) isa Union && !(eltype(TT) isa UnionAll)
            return true
        elseif TT isa Union && !isunionwithnothing(TT)
            return true
        end
	end
    return false	
end

function reconstructkwargs(arg::T) where {T}
    kwargs = Dict{Symbol, Any}()
    numfields = length(T.types)
    fnames = fieldnames(T)
    for i = 2:numfields
        field = getfield(arg, i)
        prevname = fnames[i - 1]
        # hack to make the example work
        TT = field isa Vector ? eltype(field) : typeof(field)
        if :parameters in propertynames(TT) && length(TT.parameters) > 0
            TT = TT.name.wrapper
        end
        if field isa Vector && eltype(field) isa Union && !(eltype(field) isa UnionAll)
            kwargs[prevname] = [FlatBuffers.typeorder(TT, typeof(x)) for x in field]
        elseif (T.types[i] isa Union && !isunionwithnothing(T.types[i]))
            kwargs[prevname] = FlatBuffers.typeorder(T.types[i], TT)
        end
    end
    return kwargs
end

function buildbuffer!(b::Builder{T1}, arg::T, prev=nothing) where {T1<:Any, T<:Array}
    # array of things
    buildvector!(b, arg, length(arg), prev)
end

function buildbuffer!(b::Builder{T1}, arg::T, prev=nothing) where {T1<:Any, T<:Any}
    # populate the _type field before unions/vectors of unions
    if needreconstruct(T)
        # reconstruct it so the types before the fields
        # are populated correctly
        kwargs = reconstructkwargs(arg)
        arg = Parameters.reconstruct(arg; kwargs...)
    end
    if isstruct(T)
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
                buildbuffer!(b, getfield(arg, i), getprevfieldvalue(arg, i))
            end
        end
        n = offset(b)
    else
        # build a table type
        # check for string/array/table types
        numfields = length(T.types)
        # early exit for empty objects
        if numfields == 0
            startobject(b, 0)
            return endobject(b)
        end
        os = Int[]
        isdefault = falses(numfields)
        for i = 1:numfields
            push!(os, getoffset(b, getfieldvalue(arg, i), getprevfieldvalue(arg, i)))
        end
        # all nested have been written, with offsets in `os[]`
        # don't use slots for the last N members if they are all default
        # also leave slots for deprecated fields
        i = numfields
        isdefault = getfieldvalue(arg, i) == default(T, i)
        while isdefault && i > 0
            i -= 1
            isdefault = getfieldvalue(arg, i) == default(T, i)
        end
        soff = slot_offsets(T)
        numslots = div(soff[i] - 4, 2) + 1
        startobject(b, numslots)
        i = 1
        field = 1
        while i <= numslots
            # leave holes for deprecated fields
            j = 2
            start = field == 1 ? soff[1] : soff[field - 1]
            while (start + j) < soff[field]
                # empty slot
                i += 1
                j += 2
            end
            val = getfieldvalue(arg, field)
            d = default(T, field)
            if !(isunionwithnothing(T.types[field]) && val == nothing)
                putslot!(b, i,
                    val,
                    os[field],
                    d,
                    getprevfieldvalue(arg, field)
                    )
            end
            field += 1
            i += 1
        end
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
