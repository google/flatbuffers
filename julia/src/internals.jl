# table.go
const VtableMetadataFields = 2

const TableOrBuilder = Union{Table,Builder}

const Bytes2Type = Dict{Int, DataType}(1=>UInt8, 2=>UInt16, 4=>UInt32, 8=>UInt64)

Base.get(t::TableOrBuilder, pos, ::Type{T}) where {T} = read(IOBuffer(view(t.bytes, (pos+1):length(t.bytes))), T)
readbuffer(t::AbstractVector{UInt8}, pos::Int, ::Type{T}) where {T} = read(IOBuffer(view(t, (pos+1):length(t))), T)
Base.get(t::TableOrBuilder, pos, ::Type{T}) where {T <: Enum} = T(read(IOBuffer(view(t.bytes, (pos+1):length(t.bytes))), Bytes2Type[sizeof(T)]))

"""
`offset` provides access into the Table's vtable.

Deprecated fields are ignored by checking against the vtable's length.
"""
function offset(t::Table, vtableoffset)
	vtable = t.pos - get(t, t.pos, Int32)
	return vtableoffset < get(t, vtable, Int16) ? get(t, vtable + vtableoffset, Int16) : 0
end

"`indirect` retrieves the relative offset stored at `offset`."
indirect(t::Table, off) = off + get(t, off, Int32)

"""
`vectorlen` retrieves the length of the vector whose offset is stored at
`off` in this object.
"""
function vectorlen(t::Table, off)
	off += t.pos
	off += get(t, off, Int32)
	return get(t, off, Int32)
end

"""
`vector` retrieves the start of data of the vector whose offset is stored
at `off` in this object.
"""
function vector(t::Table, off)
	off += t.pos
	off += get(t, off, Int32)
	# data starts after metadata containing the vector length
	return off + sizeof(Int32)
end

"""
GetVOffsetTSlot retrieves the VOffsetT that the given vtable location
points to. If the vtable value is zero, the default value `d`
will be returned.
"""
function getoffsetslot(t::Table, slot, d)
	off = offset(t, slot)
	if off == 0
		return d
	end
	return off
end

"""
`getslot` retrieves the `T` that the given vtable location
points to. If the vtable value is zero, the default value `d`
will be returned.
"""
function getslot(t::Table, slot, d::T) where {T}
	off = offset(t, slot)
	if off == 0
		return d
	end

	return get(t, t.pos + off, T)
end

# builder.go
value(x::T) where {T <: Enum} = length(T.types) == 0 ? Int(x) : getfield(x,1)

Base.write(sink::Builder, o, x::Union{Bool,UInt8}) = sink.bytes[o+1] = UInt8(x)
function Base.write(sink::Builder, off, x::T) where {T}
	off += 1
	for (i,ind) = enumerate(off:(off + sizeof(T) - 1))
		sink.bytes[ind] = (x >> ((i-1) * 8)) % UInt8
	end
end
Base.write(b::Builder, o, x::Float32) = write(b, o, reinterpret(UInt32, x))
Base.write(b::Builder, o, x::Float64) = write(b, o, reinterpret(UInt64, x))
Base.write(b::Builder, o, x::Enum) = write(b, o, reinterpret(Bytes2Type[sizeof(x)], value(x)))

"Offset relative to the end of the buffer."
offset(b::Builder) = length(b.bytes) - b.head

pad!(b::Builder, n) = foreach(x->place!(b, 0x00), 1:n)

"""
`finishedbytes` returns a pointer to the written data in the byte buffer.
Panics if the builder is not in a finished state (which is caused by calling
`finish!()`).
"""
function finishedbytes(b::Builder)
	assertfinished(b)
	return b.bytes[b.head+1:end]
end

function startobject(b::Builder, numslots)
	assertnotnested(b)
	b.nested = true
	b.vtable = zeros(Int, numslots)
	b.objectend = offset(b)
	b.minalign = 1
	return b
end

"""
`endobject` writes data necessary to finish object construction.
"""
function endobject(b::Builder{T}) where {T}
	assertnested(b)
	n = writevtable!(b)
	b.nested = false
	return n
end

"""
`prep!` prepares to write an element of `size` after `additionalbytes`
have been written, e.g. if you write a string, you need to align such
the int length field is aligned to sizeof(Int32), and the string data follows it
directly.
If all you need to do is align, `additionalbytes` will be 0.
"""
function prep!(b::Builder, size, additionalbytes)
	# Track the biggest thing we've ever aligned to.
	if size > b.minalign
		b.minalign = size
	end
	# Find the amount of alignment needed such that `size` is properly
	# aligned after `additionalBytes`:
	alignsize = xor(Int(-1), (length(b.bytes) - b.head) + additionalbytes) + 1
	alignsize &= (size - 1)

	# Reallocate the buffer if needed:
	totalsize = alignsize + size + additionalbytes
	if b.head <= totalsize
		len = length(b.bytes)
		prepend!(b.bytes, zeros(UInt8, totalsize))
		b.head += length(b.bytes) - len
	end
	pad!(b, alignsize)
	return
end

"""
`prepend!` prepends a `T` to the Builder buffer.
Aligns and checks for space.
"""
function Base.prepend!(b::Builder, x::T) where {T}
	prep!(b, sizeof(T), 0)
	place!(b, x)
	return
end

"""
`place!` prepends a `T` to the Builder, without checking for space.
"""
function place!(b::Builder, x::T) where {T}
	b.head -= sizeof(T)
	write(b, b.head, x)
	return
end

"""
`startvector` initializes bookkeeping for writing a new vector.

A vector has the following format:
<UOffsetT: number of elements in this vector>
<T: data>+, where T is the type of elements of this vector.
"""
function startvector(b::Builder, elemSize, numElems, alignment)
	assertnotnested(b)
	b.nested = true
	prep!(b, sizeof(UInt32), elemSize * numElems)
	prep!(b, alignment, elemSize * numElems)
	return offset(b)
end

"""
`endvector` writes data necessary to finish vector construction.
"""
function endvector(b::Builder, vectorNumElems)
	assertnested(b)
	place!(b, UInt32(vectorNumElems))
	b.nested = false
	return offset(b)
end

"""
`createstring` writes a null-terminated string as a vector.
"""
function createstring(b::Builder, s::AbstractString)
	assertnotnested(b)
	b.nested = true
	s = codeunits(s)
	prep!(b, sizeof(UInt32), length(s) + 1)
	place!(b, UInt8(0))

	l = length(s)

	b.head -= l
	copyto!(b.bytes, b.head+1, s, 1, l)
	return endvector(b, length(s))
end

"""
`createbytevector` writes a byte vector
"""
function createbytevector(b::Builder, v::AbstractVector{UInt8})
	assertnotnested(b)
	b.nested = true

	prep!(b, sizeof(UInt32), length(v))

	l = length(v)

	b.head -= l
	copyto!(b.bytes, b.head+1, v, 1, l)

	return endvector(b, length(v))
end

"""
`prependoffset!` prepends an Int32, relative to where it will be written.
"""
function prependoffset!(b::Builder, off)
	prep!(b, sizeof(Int32), 0) # Ensure alignment is already done.
	if !(off <= offset(b))
		throw(ArgumentError("unreachable: $off <= $(offset(b))"))
	end
	off2 = offset(b) - off + sizeof(Int32)
	place!(b, Int32(off2))
	return
end

function prependoffsetslot!(b::Builder, o::Int, x::T, d) where {T}
	if x != T(d)
		prependoffset!(b, x)
		slot!(b, o)
	end
	return
end

"""
`prependslot!` prepends a `T` onto the object at vtable slot `o`.
If value `x` equals default `d`, then the slot will be set to zero and no
other data will be written.
"""
function prependslot!(b::Builder, o::Int, x::T, d) where {T}
	if x != T(d)
		prepend!(b, x)
		slot!(b, o)
	end
	return
end

"""
`prependstructslot!` prepends a struct onto the object at vtable slot `o`.
Structs are stored inline, so nothing additional is being added.
In generated code, `d` is always 0.
"""
function prependstructslot!(b::Builder, voffset, x, d)
	if x != d
		assertnested(b)
		if x != offset(b)
			throw(ArgumentError("inline data write outside of object"))
		end
		slot!(b, voffset)
	end
	return
end

"""
`slot!` sets the vtable key `voffset` to the current location in the buffer.
"""
function slot!(b::Builder, slotnum)
	b.vtable[slotnum] = offset(b)
end

"""
`finish!` finalizes a buffer, pointing to the given `rootTable`.
"""
function finish!(b::Builder{T}, rootTable) where {T}
	assertnotnested(b)
	identifier = file_identifier(T)
	n = length(identifier)
	prep!(b, b.minalign, sizeof(UInt32))
	for i = 0:(n-1)
		prepend!(b, UInt8(identifier[n - i]))
	end
	prependoffset!(b, Int32(rootTable))
	b.finished = true
	return
end

function assertnested(b::Builder)
	# If you get this assert, you're in an object while trying to write
	# data that belongs outside of an object.
	# To fix this, write non-inline data (like vectors) before creating
	# objects.
	if !b.nested
		throw(ArgumentError("Incorrect creation order: must be inside object."))
	end
	return
end

function assertnotnested(b::Builder)
	# If you hit this, you're trying to construct a Table/Vector/String
	# during the construction of its parent table (between the MyTableBuilder
	# and builder.Finish()).
	# Move the creation of these view-objects to above the MyTableBuilder to
	# not get this assert.
	# Ignoring this assert may appear to work in simple cases, but the reason
	# it is here is that storing objects in-line may cause vtable offsets
	# to not fit anymore. It also leads to vtable duplication.
	if b.nested
		throw(ArgumentError("Incorrect creation order: object must not be nested."))
	end
	return
end

function assertfinished(b::Builder)
	# If you get this assert, you're attempting to get access a buffer
	# which hasn't been finished yet. Be sure to call builder.Finish()
	# with your root table.
	# If you really need to access an unfinished buffer, use the bytes
	# buffer directly.
	if !b.finished
		throw(ArgumentError("Incorrect use of FinishedBytes(): must call 'Finish' first."))
	end
end

"""
WriteVtable serializes the vtable for the current object, if applicable.

Before writing out the vtable, this checks pre-existing vtables for equality
to this one. If an equal vtable is found, point the object to the existing
vtable and return.

Because vtable values are sensitive to alignment of object data, not all
logically-equal vtables will be deduplicated.

A vtable has the following format:
<VOffsetT: size of the vtable in bytes, including this value>
<VOffsetT: size of the object in bytes, including the vtable offset>
<VOffsetT: offset for a field> * N, where N is the number of fields in
the schema for this type. Includes deprecated fields.
Thus, a vtable is made of 2 + N elements, each SizeVOffsetT bytes wide.

An object has the following format:
<SOffsetT: offset to this object's vtable (may be negative)>
<byte: data>+
"""
function writevtable!(b::Builder{T}) where {T}
	# Prepend a zero scalar to the object. Later in this function we'll
	# write an offset here that points to the object's vtable:
	prepend!(b, Int32(0))

	objectOffset = offset(b)
	existingVtable = 0

	# Search backwards through existing vtables, because similar vtables
	# are likely to have been recently appended. See
	# BenchmarkVtableDeduplication for a case in which this heuristic
	# saves about 30% of the time used in writing objects with duplicate
	# tables.
	for i = length(b.vtables):-1:1
		# Find the other vtable, which is associated with `i`:
		vt2Offset = b.vtables[i]
		vt2Start = length(b.bytes) - vt2Offset
		vt2Len = readbuffer(b.bytes, vt2Start, Int16)

		metadata = VtableMetadataFields * sizeof(Int16)
		vt2End = vt2Start + vt2Len
		vt2 = view(b.bytes, (vt2Start + metadata + 1):vt2End) #TODO: might need a +1 on the start of range here

		# Compare the other vtable to the one under consideration.
		# If they are equal, store the offset and break:
		if vtableEqual(b.vtable, objectOffset, vt2)
			existingVtable = vt2Offset
			break
		end
	end

	if existingVtable == 0
		# Did not find a vtable, so write this one to the buffer.

		# Write out the current vtable in reverse , because
		# serialization occurs in last-first order:
		for i = length(b.vtable):-1:1
			off::Int16 = 0
			if b.vtable[i] != 0
				# Forward reference to field;
				# use 32bit number to assert no overflow:
				off = objectOffset - b.vtable[i]
			end
			prepend!(b, Int16(off))
		end

		# The two metadata fields are written last.

		# First, store the object bytesize:
		objectSize::Int16 = objectOffset - b.objectend
		prepend!(b, objectSize)

		# Second, store the vtable bytesize:
		vbytes::Int16 = (length(b.vtable) + VtableMetadataFields) * sizeof(Int16)
		prepend!(b, vbytes)

		# Next, write the offset to the new vtable in the
		# already-allocated SOffsetT at the beginning of this object:
		objectStart::Int32 = length(b.bytes) - objectOffset
		write(b, objectStart, Int32(offset(b) - objectOffset))

		# Finally, store this vtable in memory for future
		# deduplication:
		push!(b.vtables, offset(b))
	else
		# Found a duplicate vtable.

		objectStart = length(b.bytes) - objectOffset
		b.head = objectStart

		# Write the offset to the found vtable in the
		# already-allocated SOffsetT at the beginning of this object:
		write(b, b.head, Int32(existingVtable - objectOffset))
	end

	empty!(b.vtable)
	return objectOffset
end

"vtableEqual compares an unwritten vtable to a written vtable."
function vtableEqual(a::Vector{Int}, objectStart, b::AbstractVector{UInt8})
	if length(a) * sizeof(Int16) != length(b)
		return false
	end

	for i = 0:(length(a)-1)
		x = read(IOBuffer(view(b, (i * sizeof(Int16) + 1):length(b))), Int16)

		# Skip vtable entries that indicate a default value.
		x == 0 && a[i+1] == 0 && continue

		y = objectStart - a[i+1]
		x != y && return false
	end
	return true
end
