using FlatBuffers
@static if VERSION < v"0.7.0-DEV.2005"
    using Base.Test
else
    using Test
end
if !isdefined(Base, :Nothing)
    const Nothing = Void
end

include("internals.jl")
CheckByteLayout()
CheckManualBuild()
CheckVtableDeduplication()
CheckNotInObjectError()
CheckStringIsNestedError()
CheckByteStringIsNestedError()
CheckStructIsNotInlineError()
CheckFinishedBytesError()
CheckCreateByteVector()
checkFuzz(100, 100, true)

include("monster.jl")

vec3 = Example.Vec3(1.0, 2.0, 3.0, 3.0, Example.Color(1), Example.Test(5, 6))
test4 = Example.Test[Example.Test(10, 20), Example.Test(30, 40)]
testArrayOfString = ["test1","test2"]

mon = Example.Monster(vec3, 150, 80, "MyMonster", false, collect(0x00:0x04),
        Example.Blue, test4, testArrayOfString, Example.Monster[],
        UInt8[], Example.Stat("",0,0), false, 0, 0, 0, 0, 0, 0, 0, 0,
        Bool[], 0, 0, 0)
b = FlatBuffers.build!(mon)
monst = FlatBuffers.read(b)
monst2 = FlatBuffers.read(Example.Monster, FlatBuffers.bytes(b))

@test mon.pos == monst.pos
@test mon.pos == monst2.pos

# create test types
# types (Scalar, Enum, struct, T, String, Vector{UInt8})
mutable struct TestInt8T
    x::Int8
end

inst1 = TestInt8T(1)

b = FlatBuffers.Builder(TestInt8T)
FlatBuffers.build!(b, inst1)
t = FlatBuffers.Table(b)
inst1_2 = FlatBuffers.read(t)

@test inst1.x === inst1_2.x

struct TestInt8I
    x::Int8
end

inst2 = TestInt8I(2)

mutable struct TestInt8A
    x::Vector{Int8}
end

inst3 = TestInt8A([1,2,3])

b = FlatBuffers.Builder(TestInt8A)
FlatBuffers.build!(b, inst3)
t = FlatBuffers.Table(b)
inst3_2 = FlatBuffers.read(t)

@test inst3.x == inst3_2.x

mutable struct TestMixT
    x::Int8
    y::String
    z::Vector{Int8}
end

inst4 = TestMixT(10,"hey there sailor",[1,2,3])

b = FlatBuffers.Builder(TestMixT)
FlatBuffers.build!(b, inst4)
t = FlatBuffers.Table(b)
inst4_2 = FlatBuffers.read(t)

@test inst4.x == inst4_2.x && inst4.y == inst4_2.y && inst4.z == inst4_2.z

# simple sub-table/type (Stat)
mutable struct TestSubT
    x::TestInt8T
    y::TestInt8I
    z::TestInt8A
end

inst5 = TestSubT(inst1, inst2, inst3)

b = FlatBuffers.Builder(TestSubT)
FlatBuffers.build!(b, inst5)
t = FlatBuffers.Table(b)
inst5_2 = FlatBuffers.read(t)

@test inst5.x.x == inst5_2.x.x && inst5.y.x == inst5_2.y.x && inst5.z.x == inst5_2.z.x

# vtable duplicates
mutable struct TestDupT
    x::TestInt8T
    y::TestInt8I
    z::TestInt8T
end

inst6 = TestDupT(inst1, inst2, TestInt8T(2))

b = FlatBuffers.Builder(TestDupT)
FlatBuffers.build!(b, inst6)
t = FlatBuffers.Table(b)
inst6_2 = FlatBuffers.read(t)

@test inst6.x.x == inst6_2.x.x && inst6.y.x == inst6_2.y.x && inst6.z.x == inst6_2.z.x

mutable struct TestDup2T
    x::Vector{TestInt8T}
end

inst7 = TestDup2T([inst1, TestInt8T(2), TestInt8T(3), TestInt8T(4)])

b = FlatBuffers.Builder(TestDup2T)
FlatBuffers.build!(b, inst7)
t = FlatBuffers.Table(b)
inst7_2 = FlatBuffers.read(t)

@test all(map(x->x.x, inst7.x) .== map(x->x.x, inst7_2.x))

# self-referential type test (type has subtype of itself)
# type TestCircT
#     x::Int8
#     y::TestCircT
# end
#
# struct TestCircI
#     x::Int8
#     y::TestCircI
# end

# simple Union (Any_)

# fbs
# table TestUnionT {
#     x::TestUnionI
# }

@UNION TestUnionU (Nothing,TestInt8T,TestInt8A)

mutable struct TestUnionT
    x_type::Int8
    x::TestUnionU
end

TestUnionT(x::TestUnionU) = TestUnionT(FlatBuffers.typeorder(TestUnionU, typeof(x)), x)

inst8 = TestUnionT(inst1)

b = FlatBuffers.Builder(TestUnionT)
FlatBuffers.build!(b, inst8)
t = FlatBuffers.Table(b)
inst8_2 = FlatBuffers.read(t)

@test inst8.x_type == inst8_2.x_type && inst8.x.x == inst8_2.x.x

inst9 = TestUnionT(inst3)

b = FlatBuffers.Builder(TestUnionT)
FlatBuffers.build!(b, inst9)
t = FlatBuffers.Table(b)
inst9_2 = FlatBuffers.read(t)

@test inst9.x_type == inst9_2.x_type && inst9.x.x == inst9_2.x.x

# test @STRUCT macro
@STRUCT struct A
    a::Int32
end
@test sizeof(A) == 4
@test all(fieldnames(A) .== [:a])
@test A(1) == A(1)

@STRUCT struct B
    a::Int8
    b::Int32
end
@test sizeof(B) == 8
@test all(fieldnames(B) .== [:a, :_pad_a_B_0, :_pad_a_B_1, :b])
@test B(1,2) == B(1,2)

@STRUCT struct C
    a::Int16
    b::Int32
    c::Int16
end
@test sizeof(C) == 12
@test all(fieldnames(C) .== [:a, :_pad_a_C_0, :b, :c, :_pad_c_C_1])
@test C(1,2,3) == C(1,2,3)

@STRUCT struct D
    a::Int8
    b::Int64
end
@test sizeof(D) == 16
@test all(fieldnames(D) .== [:a, :_pad_a_D_0, :_pad_a_D_1, :_pad_a_D_2, :b])
@test D(1,2) == D(1,2)

@STRUCT struct E
    a::Int64
    b::Int32
end
@test sizeof(E) == 16
@test all(fieldnames(E) .== [:a, :b, :_pad_b_E_0])
@test E(1,2) == E(1,2)

@STRUCT struct F
    a::Int32
    b::Int16
    c::Int32
    d::Int32
    e::Int64
end
@test sizeof(F) == 24
@test all(fieldnames(F) .== [:a, :b, :_pad_b_F_0, :c, :d, :e])
@test F(1,2,3,4,5) == F(1,2,3,4,5)

@STRUCT struct G
    a::Float64
    b::Int8
    c::Int16
    d::Int32
end
@test sizeof(G) == 24
@test all(fieldnames(G) .== [:a, :b, :_pad_b_G_0, :c, :_pad_c_G_1, :d])
@test G(1,2,3,4) == G(1,2,3,4)

@STRUCT struct H
    a::Float32
    b::Int8
    c::Int16
end
@test sizeof(H) == 8
@test all(fieldnames(H) .== [:a, :b, :_pad_b_H_0, :c])
@test H(1,2,3) == H(1,2,3)

@STRUCT struct I
    a::Float64
    b::Int8
    c::Int32
end
@test sizeof(I) == 16
@test all(fieldnames(I) .== [:a, :b, :_pad_b_I_0, :_pad_b_I_1, :c])
@test I(1,2,3) == I(1,2,3)

@STRUCT struct J
    a::Int8
    b::A
end
@test sizeof(J) == 8
@test all(fieldnames(J) .== [:a, :_pad_a_J_0, :_pad_a_J_1, :b_A_a])
@test J(1,A(2)) == J(1,A(2))

@STRUCT struct K
    a::J
    b::I
    c::J
end
@test sizeof(K) == 48
@test all(fieldnames(K) .== [:a_J_a, :a_J__pad_a_J_0, :a_J__pad__pad_a_J_0_J_0, :a_J__pad_a_J_1, :a_J__pad__pad_a_J_1_J_1, :a_J_b_A_a, :b_I_a, :b_I_b, :b_I__pad_b_I_0, :b_I__pad__pad_b_I_0_I_0, :b_I__pad_b_I_1, :b_I__pad__pad_b_I_1_I_1, :b_I_c, :c_J_a, :c_J__pad_a_J_0, :c_J__pad__pad_a_J_0_J_0, :c_J__pad_a_J_1, :c_J__pad__pad_a_J_1_J_1, :c_J_b_A_a])
@test K(J(1,A(2)), I(3.0, 4, 5), J(6, A(7))) == K(J(1,A(2)), I(3.0, 4, 5), J(6, A(7)))
