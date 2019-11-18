# Store specific byte patterns in these variables for the fuzzer. These
# values are taken verbatim from the C++ function FuzzTest1.
const overflowingInt32Val = read(IOBuffer(UInt8[0x83, 0x33, 0x33, 0x33]), Int32)
const overflowingInt64Val = read(IOBuffer(UInt8[0x84, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44]), Int64)

# CheckByteLayout verifies the bytes of a Builder in various scenarios.
function CheckByteLayout()
	check = want-> begin
	got = b.bytes[b.head+1:end]
	@test want == got
	return
end

# test 1: numbers

b = FlatBuffers.Builder()
check(UInt8[])
FlatBuffers.prepend!(b, true)
check(UInt8[1])
FlatBuffers.prepend!(b, Int8(-127))
check(UInt8[129, 1])
FlatBuffers.prepend!(b, UInt8(255))
check(UInt8[255, 129, 1])
FlatBuffers.prepend!(b, Int16(-32222))
check(UInt8[0x22, 0x82, 0, 255, 129, 1]) # first pad
FlatBuffers.prepend!(b, UInt16(0xFEEE))
check(UInt8[0xEE, 0xFE, 0x22, 0x82, 0, 255, 129, 1]) # no pad this time
FlatBuffers.prepend!(b, Int32(-53687092))
check(UInt8[204, 204, 204, 252, 0xEE, 0xFE, 0x22, 0x82, 0, 255, 129, 1])
FlatBuffers.prepend!(b, UInt32(0x98765432))
check(UInt8[0x32, 0x54, 0x76, 0x98, 204, 204, 204, 252, 0xEE, 0xFE, 0x22, 0x82, 0, 255, 129, 1])

# test 1b: numbers 2

b = FlatBuffers.Builder()
prepend!(b, 0x1122334455667788)
check(UInt8[0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11])

# test 2: 1xbyte vector

b = FlatBuffers.Builder()
check(UInt8[])
FlatBuffers.startvector(b, sizeof(Bool), 1, 1)
check(UInt8[0, 0, 0]) # align to 4bytes
FlatBuffers.prepend!(b, UInt8(1))
check(UInt8[1, 0, 0, 0])
FlatBuffers.endvector(b, 1)
check(UInt8[1, 0, 0, 0, 1, 0, 0, 0]) # padding

# test 3: 2xbyte vector

b = FlatBuffers.Builder()
FlatBuffers.startvector(b, sizeof(UInt8), 2, 1)
check(UInt8[0, 0]) # align to 4bytes
FlatBuffers.prepend!(b, UInt8(1))
check(UInt8[1, 0, 0])
FlatBuffers.prepend!(b, UInt8(2))
check(UInt8[2, 1, 0, 0])
FlatBuffers.endvector(b, 2)
check(UInt8[2, 0, 0, 0, 2, 1, 0, 0]) # padding

# test 3b: 11xbyte vector matches builder size

b = FlatBuffers.Builder(Any, 12)
FlatBuffers.startvector(b, sizeof(UInt8), 8, 1)
start = UInt8[]
check(start)
for i = 1:11
	FlatBuffers.prepend!(b, UInt8(i))
	start = append!(UInt8[i], start)
	check(start)
end
FlatBuffers.endvector(b, 8)
check(append!(UInt8[8, 0, 0, 0], start))

# test 4: 1xuint16 vector

b = FlatBuffers.Builder()
FlatBuffers.startvector(b, sizeof(UInt16), 1, 1)
check(UInt8[0, 0]) # align to 4bytes
FlatBuffers.prepend!(b, UInt16(1))
check(UInt8[1, 0, 0, 0])
FlatBuffers.endvector(b, 1)
check(UInt8[1, 0, 0, 0, 1, 0, 0, 0]) # padding

# test 5: 2xuint16 vector

b = FlatBuffers.Builder()
FlatBuffers.startvector(b, sizeof(UInt16), 2, 1)
check(UInt8[]) # align to 4bytes
FlatBuffers.prepend!(b, UInt16(0xABCD))
check(UInt8[0xCD, 0xAB])
FlatBuffers.prepend!(b, UInt16(0xDCBA))
check(UInt8[0xBA, 0xDC, 0xCD, 0xAB])
FlatBuffers.endvector(b, 2)
check(UInt8[2, 0, 0, 0, 0xBA, 0xDC, 0xCD, 0xAB])

# test 6: CreateString

b = FlatBuffers.Builder()
FlatBuffers.createstring(b, "foo")
check(UInt8[3, 0, 0, 0, 'f', 'o', 'o', 0]) # 0-terminated, no pad
FlatBuffers.createstring(b, "moop")
check(UInt8[4, 0, 0, 0, 'm', 'o', 'o', 'p', 0, 0, 0, 0, # 0-terminated, 3-byte pad
	3, 0, 0, 0, 'f', 'o', 'o', 0])

# test 6b: CreateString unicode

b = FlatBuffers.Builder()
# These characters are chinese from blog.golang.org/strings
# We use escape codes here so that editors without unicode support
# aren't bothered:
uni_str = "\u65e5\u672c\u8a9e"
FlatBuffers.createstring(b, uni_str)
check(UInt8[9, 0, 0, 0, 230, 151, 165, 230, 156, 172, 232, 170, 158, 0, #  null-terminated, 2-byte pad
	0, 0])

# test 6c: CreateUInt8String

b = FlatBuffers.Builder()
FlatBuffers.createstring(b, "foo")
check(UInt8[3, 0, 0, 0, 'f', 'o', 'o', 0]) # 0-terminated, no pad
FlatBuffers.createstring(b, "moop")
check(UInt8[4, 0, 0, 0, 'm', 'o', 'o', 'p', 0, 0, 0, 0, # 0-terminated, 3-byte pad
	3, 0, 0, 0, 'f', 'o', 'o', 0])

# test 7: empty vtable
b = FlatBuffers.Builder()
FlatBuffers.startobject(b, 0)
check(UInt8[])
FlatBuffers.endobject(b)
check(UInt8[4, 0, 4, 0, 4, 0, 0, 0])

# test 8: vtable with one true bool
b = FlatBuffers.Builder()
check(UInt8[])
FlatBuffers.startobject(b, 1)
check(UInt8[])
FlatBuffers.prependslot!(b, 1, true, false)
FlatBuffers.endobject(b)
check(UInt8[
	6, 0, # vtable bytes
	8, 0, # length of object including vtable offset
	7, 0, # start of bool value
	6, 0, 0, 0, # offset for start of vtable (int32)
	0, 0, 0, # padded to 4 bytes
	1, # bool value
	])

# test 9: vtable with one default bool
b = FlatBuffers.Builder()
check(UInt8[])
FlatBuffers.startobject(b, 1)
check(UInt8[])
FlatBuffers.prependslot!(b, 1, false, false)
FlatBuffers.endobject(b)
check(UInt8[
	6, 0, # vtable bytes
	4, 0, # end of object from here
	0, 0, # entry 1 is zero
	6, 0, 0, 0, # offset for start of vtable (int32)
	])

# test 10: vtable with one int16
b = FlatBuffers.Builder()
FlatBuffers.startobject(b, 1)
FlatBuffers.prependslot!(b, 1, Int16(0x789A), Int16(0))
FlatBuffers.endobject(b)
check(UInt8[
	6, 0, # vtable bytes
	8, 0, # end of object from here
	6, 0, # offset to value
	6, 0, 0, 0, # offset for start of vtable (int32)
	0, 0, # padding to 4 bytes
	0x9A, 0x78,
	])

# test 11: vtable with two int16
b = FlatBuffers.Builder()
FlatBuffers.startobject(b, 2)
FlatBuffers.prependslot!(b, 1, Int16(0x3456), Int16(0))
FlatBuffers.prependslot!(b, 2, Int16(0x789A), Int16(0))
FlatBuffers.endobject(b)
check(UInt8[
	8, 0, # vtable bytes
	8, 0, # end of object from here
	6, 0, # offset to value 0
	4, 0, # offset to value 1
	8, 0, 0, 0, # offset for start of vtable (int32)
	0x9A, 0x78, # value 1
	0x56, 0x34, # value 0
	])

# test 12: vtable with int16 and bool
b = FlatBuffers.Builder()
FlatBuffers.startobject(b, 2)
FlatBuffers.prependslot!(b, 1, Int16(0x3456), Int16(0))
FlatBuffers.prependslot!(b, 2, true, false)
FlatBuffers.endobject(b)
check(UInt8[
	8, 0, # vtable bytes
	8, 0, # end of object from here
	6, 0, # offset to value 0
	5, 0, # offset to value 1
	8, 0, 0, 0, # offset for start of vtable (int32)
	0,          # padding
	1,          # value 1
	0x56, 0x34, # value 0
	])

# test 12: vtable with empty vector
b = FlatBuffers.Builder()
FlatBuffers.startvector(b, sizeof(UInt8), 0, 1)
vecend = FlatBuffers.endvector(b, 0)
FlatBuffers.startobject(b, 1)
FlatBuffers.prependoffsetslot!(b, 1, UInt32(vecend), UInt32(0))
FlatBuffers.endobject(b)
check(UInt8[
	6, 0, # vtable bytes
	8, 0,
	4, 0, # offset to vector offset
	6, 0, 0, 0, # offset for start of vtable (int32)
	4, 0, 0, 0,
	0, 0, 0, 0, # length of vector (not in struct)
	])

# test 12b: vtable with empty vector of byte and some scalars
b = FlatBuffers.Builder()
FlatBuffers.startvector(b, sizeof(UInt8), 0, 1)
vecend = FlatBuffers.endvector(b, 0)
FlatBuffers.startobject(b, 2)
FlatBuffers.prependslot!(b, 1, Int16(55), Int16(0))
FlatBuffers.prependoffsetslot!(b, 2, UInt32(vecend), UInt32(0))
FlatBuffers.endobject(b)
check(UInt8[
	8, 0, # vtable bytes
	12, 0,
	10, 0, # offset to value 0
	4, 0, # offset to vector offset
	8, 0, 0, 0, # vtable loc
	8, 0, 0, 0, # value 1
	0, 0, 55, 0, # value 0

	0, 0, 0, 0, # length of vector (not in struct)
	])

# test 13: vtable with 1 int16 and 2-vector of int16
b = FlatBuffers.Builder()
FlatBuffers.startvector(b, sizeof(Int16), 2, 1)
FlatBuffers.prepend!(b, Int16(0x1234))
FlatBuffers.prepend!(b, Int16(0x5678))
vecend = FlatBuffers.endvector(b, 2)
FlatBuffers.startobject(b, 2)
FlatBuffers.prependoffsetslot!(b, 2, UInt32(vecend), UInt32(0))
FlatBuffers.prependslot!(b, 1, Int16(55), Int16(0))
FlatBuffers.endobject(b)
check(UInt8[
	8, 0, # vtable bytes
	12, 0, # length of object
	6, 0, # start of value 0 from end of vtable
	8, 0, # start of value 1 from end of buffer
	8, 0, 0, 0, # offset for start of vtable (int32)
	0, 0, # padding
	55, 0, # value 0
	4, 0, 0, 0, # vector position from here
	2, 0, 0, 0, # length of vector (uint32)
	0x78, 0x56, # vector value 1
	0x34, 0x12, # vector value 0
	])

# test 14: vtable with 1 struct of 1 int8, 1 int16, 1 int32
b = FlatBuffers.Builder()
FlatBuffers.startobject(b, 1)
FlatBuffers.prep!(b, 4+4+4, 0)
FlatBuffers.prepend!(b, Int8(55))
FlatBuffers.pad!(b, 3)
FlatBuffers.prepend!(b, Int16(0x1234))
FlatBuffers.pad!(b, 2)
FlatBuffers.prepend!(b, Int32(0x12345678))
structStart = FlatBuffers.offset(b)
FlatBuffers.prependstructslot!(b, 1, structStart, 0)
FlatBuffers.endobject(b)
check(UInt8[
	6, 0, # vtable bytes
	16, 0, # end of object from here
	4, 0, # start of struct from here
	6, 0, 0, 0, # offset for start of vtable (int32)
	0x78, 0x56, 0x34, 0x12, # value 2
	0, 0, # padding
	0x34, 0x12, # value 1
	0, 0, 0, # padding
	55, # value 0
	])

# test 15: vtable with 1 vector of 2 struct of 2 int8
b = FlatBuffers.Builder()
FlatBuffers.startvector(b, sizeof(Int8)*2, 2, 1)
FlatBuffers.prepend!(b, Int8(33))
FlatBuffers.prepend!(b, Int8(44))
FlatBuffers.prepend!(b, Int8(55))
FlatBuffers.prepend!(b, Int8(66))
vecend = FlatBuffers.endvector(b, 2)
FlatBuffers.startobject(b, 1)
FlatBuffers.prependoffsetslot!(b, 1, UInt32(vecend), UInt32(0))
FlatBuffers.endobject(b)
check(UInt8[
	6, 0, # vtable bytes
	8, 0,
	4, 0, # offset of vector offset
	6, 0, 0, 0, # offset for start of vtable (int32)
	4, 0, 0, 0, # vector start offset

	2, 0, 0, 0, # vector length
	66, # vector value 1,1
	55, # vector value 1,0
	44, # vector value 0,1
	33, # vector value 0,0
	])

# test 16: table with some elements
b = FlatBuffers.Builder()
FlatBuffers.startobject(b, 2)
FlatBuffers.prependslot!(b, 1, Int8(33), Int8(0))
FlatBuffers.prependslot!(b, 2, Int16(66), Int16(0))
off = FlatBuffers.endobject(b)
FlatBuffers.finish!(b, off) #TODO

check(UInt8[
	12, 0, 0, 0, # root of table: points to vtable offset

	8, 0, # vtable bytes
	8, 0, # end of object from here
	7, 0, # start of value 0
	4, 0, # start of value 1

	8, 0, 0, 0, # offset for start of vtable (int32)

	66, 0, # value 1
	0,  # padding
	33, # value 0
	])

# test 17: one unfinished table and one finished table
b = FlatBuffers.Builder()
FlatBuffers.startobject(b, 2)
FlatBuffers.prependslot!(b, 1, Int8(33), Int8(0))
FlatBuffers.prependslot!(b, 2, Int8(44), Int8(0))
off = FlatBuffers.endobject(b)
FlatBuffers.finish!(b, off)

FlatBuffers.startobject(b, 3)
FlatBuffers.prependslot!(b, 1, Int8(55), Int8(0))
FlatBuffers.prependslot!(b, 2, Int8(66), Int8(0))
FlatBuffers.prependslot!(b, 3, Int8(77), Int8(0))
off = FlatBuffers.endobject(b)
FlatBuffers.finish!(b, off)

check(UInt8[
	16, 0, 0, 0, # root of table: points to object
	0, 0, # padding

	10, 0, # vtable bytes
	8, 0, # size of object
	7, 0, # start of value 0
	6, 0, # start of value 1
	5, 0, # start of value 2
	10, 0, 0, 0, # offset for start of vtable (int32)
	0,  # padding
	77, # value 2
	66, # value 1
	55, # value 0

	12, 0, 0, 0, # root of table: points to object

	8, 0, # vtable bytes
	8, 0, # size of object
	7, 0, # start of value 0
	6, 0, # start of value 1
	8, 0, 0, 0, # offset for start of vtable (int32)
	0, 0, # padding
	44, # value 1
	33, # value 0
	])

# test 18: a bunch of bools
b = FlatBuffers.Builder()
FlatBuffers.startobject(b, 8)
FlatBuffers.prependslot!(b, 1, true, false)
FlatBuffers.prependslot!(b, 2, true, false)
FlatBuffers.prependslot!(b, 3, true, false)
FlatBuffers.prependslot!(b, 4, true, false)
FlatBuffers.prependslot!(b, 5, true, false)
FlatBuffers.prependslot!(b, 6, true, false)
FlatBuffers.prependslot!(b, 7, true, false)
FlatBuffers.prependslot!(b, 8, true, false)
off = FlatBuffers.endobject(b)
FlatBuffers.finish!(b, off)

check(UInt8[
	24, 0, 0, 0, # root of table: points to vtable offset

	20, 0, # vtable bytes
	12, 0, # size of object
	11, 0, # start of value 0
	10, 0, # start of value 1
	9, 0, # start of value 2
	8, 0, # start of value 3
	7, 0, # start of value 4
	6, 0, # start of value 5
	5, 0, # start of value 6
	4, 0, # start of value 7
	20, 0, 0, 0, # vtable offset

	1, # value 7
	1, # value 6
	1, # value 5
	1, # value 4
	1, # value 3
	1, # value 2
	1, # value 1
	1, # value 0
	])

# test 19: three bools
b = FlatBuffers.Builder()
FlatBuffers.startobject(b, 3)
FlatBuffers.prependslot!(b, 1, true, false)
FlatBuffers.prependslot!(b, 2, true, false)
FlatBuffers.prependslot!(b, 3, true, false)
off = FlatBuffers.endobject(b)
FlatBuffers.finish!(b, off)

check(UInt8[
	16, 0, 0, 0, # root of table: points to vtable offset

	0, 0, # padding

	10, 0, # vtable bytes
	8, 0, # size of object
	7, 0, # start of value 0
	6, 0, # start of value 1
	5, 0, # start of value 2
	10, 0, 0, 0, # vtable offset from here

	0, # padding
	1, # value 2
	1, # value 1
	1, # value 0
	])

# test 20: some floats
b = FlatBuffers.Builder()
FlatBuffers.startobject(b, 1)
FlatBuffers.prependslot!(b, 1, Float32(1.0), Float32(0.0))
off = FlatBuffers.endobject(b)

check(UInt8[
	6, 0, # vtable bytes
	8, 0, # size of object
	4, 0, # start of value 0
	6, 0, 0, 0, # vtable offset

	0, 0, 128, 63, # value 0
	])
end

# CheckManualBuild builds a Monster manually.
function CheckManualBuild()
	b = FlatBuffers.Builder()
	str = FlatBuffers.createstring(b, "MyMonster")

	FlatBuffers.startvector(b, 1, 5, 1)
	FlatBuffers.prepend!(b, UInt8(4))
	FlatBuffers.prepend!(b, UInt8(3))
	FlatBuffers.prepend!(b, UInt8(2))
	FlatBuffers.prepend!(b, UInt8(1))
	FlatBuffers.prepend!(b, UInt8(0))
	inv = FlatBuffers.endvector(b, 5)

	FlatBuffers.startobject(b, 13)
	FlatBuffers.prependslot!(b, 2, Int16(20), Int16(100))
	mon2 = FlatBuffers.endobject(b)

	# Test4Vector
	FlatBuffers.startvector(b, 4, 2, 1)

	# Test 0
	FlatBuffers.prep!(b, 2, 4)
	FlatBuffers.pad!(b, 1)
	FlatBuffers.place!(b, Int8(20))
	FlatBuffers.place!(b, Int16(10))

	# Test 1
	FlatBuffers.prep!(b, 2, 4)
	FlatBuffers.pad!(b, 1)
	FlatBuffers.place!(b, Int8(40))
	FlatBuffers.place!(b, Int16(30))

	# end testvector
	test4 = FlatBuffers.endvector(b, 2)

	FlatBuffers.startobject(b, 13)

	# a vec3
	FlatBuffers.prep!(b, 16, 32)
	FlatBuffers.pad!(b, 2)
	FlatBuffers.prep!(b, 2, 4)
	FlatBuffers.pad!(b, 1)
	FlatBuffers.place!(b, UInt8(6))
	FlatBuffers.place!(b, Int16(5))
	FlatBuffers.pad!(b, 1)
	FlatBuffers.place!(b, UInt8(4))
	FlatBuffers.place!(b, Float64(3.0))
	FlatBuffers.pad!(b, 4)
	FlatBuffers.place!(b, Float32(3.0))
	FlatBuffers.place!(b, Float32(2.0))
	FlatBuffers.place!(b, Float32(1.0))
	vec3Loc = FlatBuffers.offset(b)
	# end vec3

	FlatBuffers.prependstructslot!(b, 1, vec3Loc, 0) # vec3. noop
	FlatBuffers.prependslot!(b, 3, Int16(80), Int16(100))     # hp
	FlatBuffers.prependoffsetslot!(b, 4, Int32(str), Int32(0))
	FlatBuffers.prependoffsetslot!(b, 6, Int32(inv), Int32(0)) # inventory
	FlatBuffers.prependslot!(b, 8, UInt8(1), UInt8(0))
	FlatBuffers.prependoffsetslot!(b, 9, Int32(mon2), Int32(0))
	FlatBuffers.prependoffsetslot!(b, 10, Int32(test4), Int32(0))
	mon = FlatBuffers.endobject(b)

	FlatBuffers.finish!(b, mon)

	return b.bytes, b.head
end

# CheckVtableDeduplication verifies that vtables are deduplicated.
function CheckVtableDeduplication()
	b = FlatBuffers.Builder()

	FlatBuffers.startobject(b, 4)
	FlatBuffers.prependslot!(b, 1, UInt8(0), UInt8(0))
	FlatBuffers.prependslot!(b, 2, UInt8(11), UInt8(0))
	FlatBuffers.prependslot!(b, 3, UInt8(22), UInt8(0))
	FlatBuffers.prependslot!(b, 4, Int16(33), Int16(0))
	obj0 = FlatBuffers.endobject(b)

	FlatBuffers.startobject(b, 4)
	FlatBuffers.prependslot!(b, 1, UInt8(0), UInt8(0))
	FlatBuffers.prependslot!(b, 2, UInt8(44), UInt8(0))
	FlatBuffers.prependslot!(b, 3, UInt8(55), UInt8(0))
	FlatBuffers.prependslot!(b, 4, Int16(66), Int16(0))
	obj1 = FlatBuffers.endobject(b)

	FlatBuffers.startobject(b, 4)
	FlatBuffers.prependslot!(b, 1, UInt8(0), UInt8(0))
	FlatBuffers.prependslot!(b, 2, UInt8(77), UInt8(0))
	FlatBuffers.prependslot!(b, 3, UInt8(88), UInt8(0))
	FlatBuffers.prependslot!(b, 4, Int16(99), Int16(0))
	obj2 = FlatBuffers.endobject(b)

	got = b.bytes[b.head+1:end]

	want = UInt8[
	240, 255, 255, 255, # == -12. offset to dedupped vtable.
	99, 0,
	88,
	77,
	248, 255, 255, 255, # == -8. offset to dedupped vtable.
	66, 0,
	55,
	44,
	12, 0, # start of vtable
	8, 0,
	0, 0,
	7, 0,
	6, 0,
	4, 0,
	12, 0, 0, 0, # table0
	33, 0,
	22,
	11
	]

	@test got == want

	table0 = FlatBuffers.Table{Any}(b.bytes, length(b.bytes) - obj0)
	table1 = FlatBuffers.Table{Any}(b.bytes, length(b.bytes) - obj1)
	table2 = FlatBuffers.Table{Any}(b.bytes, length(b.bytes) - obj2)

	function testTable(tab, a, b, c, d)
		# vtable size
		got = FlatBuffers.getoffsetslot(tab, 0, Int16(0))
		@test 12 == got
		# object size
		got = FlatBuffers.getoffsetslot(tab, 2, Int16(0))
		@test 8 == got
		# default value
		got = FlatBuffers.getoffsetslot(tab, 4, Int16(0))
		@test a == got
		got = FlatBuffers.getslot(tab, 6, UInt8(0))
		@test b == got
		val = FlatBuffers.getslot(tab, 8, UInt8(0))
		c != val && throw(ArgumentError("failed 8, 0: $got"))
		got = FlatBuffers.getslot(tab, 10, UInt8(0))
		@test d == got
		return
	end

	testTable(table0, UInt16(0), UInt8(11), UInt8(22), UInt8(33))
	testTable(table1, UInt16(0), UInt8(44), UInt8(55), UInt8(66))
	testTable(table2, UInt16(0), UInt8(77), UInt8(88), UInt8(99))
end

# CheckNotInObjectError verifies that `endobject` fails if not inside an
# object.
function CheckNotInObjectError()
	b = FlatBuffers.Builder()

	@test_throws ArgumentError FlatBuffers.endobject(b)
end

# CheckStringIsNestedError verifies that a string can not be created inside
# another object.
function CheckStringIsNestedError()
	b = FlatBuffers.Builder()
	FlatBuffers.startobject(b, 0)
	@test_throws ArgumentError FlatBuffers.createstring(b, "foo")
end

# CheckByteStringIsNestedError verifies that a bytestring can not be created
# inside another object.
function CheckByteStringIsNestedError()
	b = FlatBuffers.Builder()
	FlatBuffers.startobject(b, 0)
	@test_throws ArgumentError FlatBuffers.createstring(b, "foo")
end

# CheckStructIsNotInlineError verifies that writing a struct in a location
# away from where it is used will cause a panic.
function CheckStructIsNotInlineError()
	b = FlatBuffers.Builder()
	FlatBuffers.startobject(b, 0)
	@test_throws ArgumentError FlatBuffers.prependstructslot!(b, 0, 1, 0)
end

# CheckFinishedBytesError verifies that `FinishedBytes` panics if the table
# is not finished.
function CheckFinishedBytesError()
	b = FlatBuffers.Builder()

	@test_throws ArgumentError FlatBuffers.finishedbytes(b)
end

function CheckCreateByteVector()
	raw = UInt8(0):UInt8(29)

	for size = 1:30
		b1 = FlatBuffers.Builder()
		b2 = FlatBuffers.Builder()
		FlatBuffers.startvector(b1, 1, size, 1)
		for i = size:-1:1
			FlatBuffers.prepend!(b1, raw[i])
		end
		FlatBuffers.endvector(b1, size)
		FlatBuffers.createbytevector(b2, raw[1:size])
		@test b1.bytes == b2.bytes
	end
end

const InitialLCGSeed = 48271
mutable struct LCG
	val::UInt32
	LCG() = new(UInt32(InitialLCGSeed))
end
reset!(lcg::LCG) = lcg.val = UInt32(InitialLCGSeed)
function next(lcg::LCG)
	n = UInt32((UInt64(lcg.val) * UInt64(279470273)) % UInt64(4294967291))
	lcg.val = n
	return n
end

# Low level stress/fuzz test: serialize/deserialize a variety of
# different kinds of data in different combinations
function checkFuzz(fuzzFields, fuzzObjects, verbose=true)

	# Values we're testing against: chosen to ensure no bits get chopped
	# off anywhere, and also be different from eachother.
	boolVal = true
	int8Val = Int8(-127) # 0x81
	uint8Val = UInt8(0xFF)
	int16Val = Int16(-32222) # 0x8222
	uint16Val = UInt16(0xFEEE)
	int32Val = Int32(overflowingInt32Val)
	uint32Val = UInt32(0xFDDDDDDD)
	int64Val = Int64(overflowingInt64Val)
	uint64Val = UInt64(0xFCCCCCCCCCCCCCCC)
	float32Val = Float32(3.14159)
	float64Val = Float64(3.14159265359)

	testValuesMax = 11 # hardcoded to the number of scalar types

	b = FlatBuffers.Builder()
	l = LCG()

	objects = fill(0, fuzzObjects)

	# Generate fuzzObjects random objects each consisting of
	# fuzzFields fields, each of a random type.
	for i = 1:fuzzObjects
		FlatBuffers.startobject(b, fuzzFields)

		for f = 1:fuzzFields
			choice = next(l) % UInt32(testValuesMax)
			if choice ==  0
				FlatBuffers.prependslot!(b, f, boolVal, false)
			elseif choice ==  1
				FlatBuffers.prependslot!(b, f, int8Val, Int8(0))
			elseif choice ==  2
				FlatBuffers.prependslot!(b, f, uint8Val, UInt8(0))
			elseif choice ==  3
				FlatBuffers.prependslot!(b, f, int16Val, Int16(0))
			elseif choice ==  4
				FlatBuffers.prependslot!(b, f, uint16Val, UInt16(0))
			elseif choice ==  5
				FlatBuffers.prependslot!(b, f, int32Val, Int32(0))
			elseif choice ==  6
				FlatBuffers.prependslot!(b, f, uint32Val, UInt32(0))
			elseif choice ==  7
				FlatBuffers.prependslot!(b, f, int64Val, Int64(0))
			elseif choice ==  8
				FlatBuffers.prependslot!(b, f, uint64Val, UInt64(0))
			elseif choice ==  9
				FlatBuffers.prependslot!(b, f, float32Val, Float32(0))
			elseif choice ==  10
				FlatBuffers.prependslot!(b, f, float64Val, Float64(0))
			end
		end

		off = FlatBuffers.endobject(b)

		# store the offset from the end of the builder buffer,
		# since it will keep growing:
		objects[i] = off
	end

	# Do some bookkeeping to generate stats on fuzzes:
	stats = Dict{String,Int}()
	function check(desc, want, got)
		v = get!(stats, desc, 0)
		stats[desc] = v + 1
		@test want == got
	end

	l = LCG() # Reset.

	# Test that all objects we generated are readable and return the
	# expected values. We generate random objects in the same order
	# so this is deterministic.
	for i = 1:fuzzObjects

		table = FlatBuffers.Table{Any}(b.bytes, length(b.bytes) - objects[i])

		for j = 0:(fuzzFields - 1)
			f = (FlatBuffers.VtableMetadataFields + j) * sizeof(Int16)
			choice = next(l) % UInt32(testValuesMax)

			if choice == 0
				check("bool", boolVal, FlatBuffers.getslot(table, f, false))
			elseif choice == 1
				check("int8", int8Val, FlatBuffers.getslot(table, f, Int8(0)))
			elseif choice == 2
				check("uint8", uint8Val, FlatBuffers.getslot(table, f, UInt8(0)))
			elseif choice == 3
				check("int16", int16Val, FlatBuffers.getslot(table, f, Int16(0)))
			elseif choice == 4
				check("uint16", uint16Val, FlatBuffers.getslot(table, f, UInt16(0)))
			elseif choice == 5
				check("int32", int32Val, FlatBuffers.getslot(table, f, Int32(0)))
			elseif choice == 6
				check("uint32", uint32Val, FlatBuffers.getslot(table, f, UInt32(0)))
			elseif choice == 7
				check("int64", int64Val, FlatBuffers.getslot(table, f, Int64(0)))
			elseif choice == 8
				check("uint64", uint64Val, FlatBuffers.getslot(table, f, UInt64(0)))
			elseif choice == 9
				check("float32", float32Val, FlatBuffers.getslot(table, f, Float32(0)))
			elseif choice == 10
				check("float64", float64Val, FlatBuffers.getslot(table, f, Float64(0)))
			end
		end
	end

	# If enough checks were made, verify that all scalar types were used:
	if fuzzFields*fuzzObjects >= testValuesMax
		if length(stats) != testValuesMax
			throw(ArgumentError("fuzzing failed to test all scalar types"))
		end
	end

	# Print some counts, if needed:
	if verbose
		if fuzzFields == 0 || fuzzObjects == 0
			println("fuzz\tfields: $fuzzFields \tobjects: $fuzzObjects \t[none]\t 0")
		else
			ks = sort!(collect(keys(stats)))
			for k in ks
				println("fuzz\tfields: $fuzzFields \tobjects: $fuzzObjects \t$(k): $(stats[k])")
			end
		end
	end
	return
end
