module SmallExample

using FlatBuffers

@enum(Color, Red = 1, Green = 2, Blue = 8)
@DEFAULT Color Red

@STRUCT struct Test
	a::Int16
	b::UInt8
	# _1::UInt8 # padding
end

mutable struct TestSimpleTableWithEnum
	color::Color
end

@DEFAULT TestSimpleTableWithEnum color=Green

@STRUCT struct Vec3
	x::Float32
	y::Float32
	z::Float32
	# _::UInt32 # padding
	test1::Float64
	test2::Color
	# __::UInt8 # padding
	test3::Test
	# ___::UInt16 # padding
end

@ALIGN Vec3 16

mutable struct Stat
	id::Union{Nothing, String}
	val::Int64
	count::UInt16
end

# Julia doesn't support forward referencing of types
# @union Any_ Union{Monster, TestSimpleTableWithEnum}

mutable struct Monster
	pos::Vec3
	mana::Int16
	hp::Int16
	name::Union{Nothing, String}
	friendly::Bool # deprecated
	inventory::Union{Nothing, Vector{UInt8}}
	color::Color
	# test_type::Any_
	# test::Union{Nothing, Vector{UInt8}}
	test4::Union{Nothing, Vector{Test}}
	testarrayofstring::Union{Nothing, Vector{String}}
	testarrayoftables::Union{Nothing, Vector{Monster}}
	# don't support nested circulr reference objects yet
	# enemy::Monster
	testnestedflatbuffer::Union{Nothing, Vector{UInt8}}
	testempty::Union{Nothing, Stat}
	testbool::Bool
	testhashs32_fnv1::Int32
	testhashu32_fnv1::UInt32
	testhashs64_fnv1::Int64
	testhashu64_fnv1::UInt64
	testhashs32_fnv1a::Int32
	testhashu32_fnv1a::UInt32
	testhashs64_fnv1a::Int64
	testhashu64_fnv1a::UInt64
	testarrayofbools::Union{Nothing, Vector{Bool}}
	testf::Float32
	testf2::Float32
	testf3::Float32
end

@DEFAULT Monster hp=Int16(100) mana=Int16(150) color=convert(UInt8, Blue) friendly=false testf=Float32(3.14159) testf2=Float32(3)

end # module
