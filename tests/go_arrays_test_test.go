// Test for Go fixed-size array support in FlatBuffers.
//
// Validates that structs with fixed-size array fields can be correctly
// built, read, mutated, and round-tripped through the object API.

package main

import (
	"testing"

	example "MyGame/Example"

	flatbuffers "github.com/google/flatbuffers/go"
)

// buildArrayTable creates a FlatBuffer containing an ArrayTable with known
// values, returning the finished byte slice.
func buildArrayTable(t *testing.T) []byte {
	t.Helper()
	builder := flatbuffers.NewBuilder(512)

	// NestedStruct field values for d[0] and d[1].
	d_a := [][]int32{{1, 2}, {3, 4}}
	d_b := []example.TestEnum{example.TestEnumA, example.TestEnumB}
	d_c := [][]example.TestEnum{
		{example.TestEnumB, example.TestEnumC},
		{example.TestEnumA, example.TestEnumC},
	}
	d_d := [][]int64{{100, 200}, {300, 400}}

	b := make([]int32, 15)
	for i := range b {
		b[i] = int32(i + 1)
	}
	f := []int64{-1, -2}

	off := example.CreateArrayStruct(builder,
		1.5,  // a
		b,    // b: [int:15]
		42,   // c
		d_a, d_b, d_c, d_d, // d: [NestedStruct:2]
		99,  // e
		f,   // f: [int64:2]
	)

	example.ArrayTableStart(builder)
	example.ArrayTableAddA(builder, off)
	root := example.ArrayTableEnd(builder)
	builder.Finish(root)

	return builder.FinishedBytes()
}

func TestArrayStructRead(t *testing.T) {
	buf := buildArrayTable(t)
	table := example.GetRootAsArrayTable(buf, 0)

	s := new(example.ArrayStruct)
	table.A(s)

	// Scalar field.
	if got := s.A(); got != 1.5 {
		t.Fatalf("A: got %v, want 1.5", got)
	}

	// Scalar array field.
	if s.BLength() != 15 {
		t.Fatalf("BLength: got %d, want 15", s.BLength())
	}
	for i := 0; i < 15; i++ {
		want := int32(i + 1)
		if got := s.B(i); got != want {
			t.Fatalf("B(%d): got %d, want %d", i, got, want)
		}
	}

	if got := s.C(); got != 42 {
		t.Fatalf("C: got %d, want 42", got)
	}

	// Struct array field.
	if s.DLength() != 2 {
		t.Fatalf("DLength: got %d, want 2", s.DLength())
	}

	ns0 := s.D(nil, 0)
	if ns0.A(0) != 1 || ns0.A(1) != 2 {
		t.Fatalf("D(0).A: got [%d,%d], want [1,2]", ns0.A(0), ns0.A(1))
	}
	if ns0.B() != example.TestEnumA {
		t.Fatalf("D(0).B: got %d, want %d", ns0.B(), example.TestEnumA)
	}
	if ns0.C(0) != example.TestEnumB || ns0.C(1) != example.TestEnumC {
		t.Fatalf("D(0).C: got [%d,%d], want [%d,%d]",
			ns0.C(0), ns0.C(1), example.TestEnumB, example.TestEnumC)
	}
	if ns0.D(0) != 100 || ns0.D(1) != 200 {
		t.Fatalf("D(0).D: got [%d,%d], want [100,200]", ns0.D(0), ns0.D(1))
	}

	ns1 := s.D(nil, 1)
	if ns1.A(0) != 3 || ns1.A(1) != 4 {
		t.Fatalf("D(1).A: got [%d,%d], want [3,4]", ns1.A(0), ns1.A(1))
	}
	if ns1.B() != example.TestEnumB {
		t.Fatalf("D(1).B: got %d, want %d", ns1.B(), example.TestEnumB)
	}
	if ns1.D(0) != 300 || ns1.D(1) != 400 {
		t.Fatalf("D(1).D: got [%d,%d], want [300,400]", ns1.D(0), ns1.D(1))
	}

	if got := s.E(); got != 99 {
		t.Fatalf("E: got %d, want 99", got)
	}

	// Int64 array.
	if s.FLength() != 2 {
		t.Fatalf("FLength: got %d, want 2", s.FLength())
	}
	if s.F(0) != -1 || s.F(1) != -2 {
		t.Fatalf("F: got [%d,%d], want [-1,-2]", s.F(0), s.F(1))
	}
}

func TestArrayStructMutate(t *testing.T) {
	buf := buildArrayTable(t)
	table := example.GetRootAsArrayTable(buf, 0)

	s := new(example.ArrayStruct)
	table.A(s)

	// Mutate scalar array elements.
	if !s.MutateB(0, 777) {
		t.Fatal("MutateB returned false")
	}
	if got := s.B(0); got != 777 {
		t.Fatalf("after MutateB: got %d, want 777", got)
	}

	// Mutate nested struct scalar array.
	ns := s.D(nil, 0)
	if !ns.MutateA(1, 999) {
		t.Fatal("MutateA returned false")
	}
	if got := ns.A(1); got != 999 {
		t.Fatalf("after MutateA: got %d, want 999", got)
	}

	// Mutate int64 array.
	if !s.MutateF(1, 12345) {
		t.Fatal("MutateF returned false")
	}
	if got := s.F(1); got != 12345 {
		t.Fatalf("after MutateF: got %d, want 12345", got)
	}
}

func TestArrayStructObjectAPI(t *testing.T) {
	buf := buildArrayTable(t)
	table := example.GetRootAsArrayTable(buf, 0)

	s := new(example.ArrayStruct)
	table.A(s)

	// Unpack to native struct.
	native := s.UnPack()
	if native == nil {
		t.Fatal("UnPack returned nil")
	}

	if native.A != 1.5 {
		t.Fatalf("native.A: got %v, want 1.5", native.A)
	}
	for i := 0; i < 15; i++ {
		want := int32(i + 1)
		if native.B[i] != want {
			t.Fatalf("native.B[%d]: got %d, want %d", i, native.B[i], want)
		}
	}
	if native.C != 42 {
		t.Fatalf("native.C: got %d, want 42", native.C)
	}
	if native.D[0] == nil || native.D[1] == nil {
		t.Fatal("native.D elements should not be nil")
	}
	if native.D[0].A != [2]int32{1, 2} {
		t.Fatalf("native.D[0].A: got %v, want [1,2]", native.D[0].A)
	}
	if native.D[1].D != [2]int64{300, 400} {
		t.Fatalf("native.D[1].D: got %v, want [300,400]", native.D[1].D)
	}
	if native.E != 99 {
		t.Fatalf("native.E: got %d, want 99", native.E)
	}
	if native.F != [2]int64{-1, -2} {
		t.Fatalf("native.F: got %v, want [-1,-2]", native.F)
	}

	// Round-trip: Pack the native struct back, then re-read.
	builder2 := flatbuffers.NewBuilder(512)
	off2 := native.Pack(builder2)

	example.ArrayTableStart(builder2)
	example.ArrayTableAddA(builder2, off2)
	root2 := example.ArrayTableEnd(builder2)
	builder2.Finish(root2)

	buf2 := builder2.FinishedBytes()
	table2 := example.GetRootAsArrayTable(buf2, 0)
	s2 := new(example.ArrayStruct)
	table2.A(s2)

	// Verify all values survived the round trip.
	if s2.A() != 1.5 {
		t.Fatal("round trip: A mismatch")
	}
	for i := 0; i < 15; i++ {
		if s2.B(i) != int32(i+1) {
			t.Fatalf("round trip: B(%d) mismatch: got %d", i, s2.B(i))
		}
	}
	if s2.C() != 42 {
		t.Fatal("round trip: C mismatch")
	}
	ns0 := s2.D(nil, 0)
	if ns0.A(0) != 1 || ns0.A(1) != 2 {
		t.Fatal("round trip: D(0).A mismatch")
	}
	ns1 := s2.D(nil, 1)
	if ns1.D(0) != 300 || ns1.D(1) != 400 {
		t.Fatal("round trip: D(1).D mismatch")
	}
	if s2.E() != 99 {
		t.Fatal("round trip: E mismatch")
	}
	if s2.F(0) != -1 || s2.F(1) != -2 {
		t.Fatal("round trip: F mismatch")
	}
}

func TestNestedStructStandalone(t *testing.T) {
	builder := flatbuffers.NewBuilder(128)

	a := []int32{10, 20}
	c := []example.TestEnum{example.TestEnumC, example.TestEnumA}
	d := []int64{0x7FFFFFFFFFFFFFFF, -0x7FFFFFFFFFFFFFFF}
	example.CreateNestedStruct(builder, a, example.TestEnumB, c, d)

	buf := builder.Bytes[builder.Head():]
	ns := &example.NestedStruct{}
	ns.Init(buf, 0)

	if ns.A(0) != 10 || ns.A(1) != 20 {
		t.Fatalf("A: got [%d,%d], want [10,20]", ns.A(0), ns.A(1))
	}
	if ns.ALength() != 2 {
		t.Fatalf("ALength: got %d, want 2", ns.ALength())
	}
	if ns.B() != example.TestEnumB {
		t.Fatalf("B: got %d, want %d", ns.B(), example.TestEnumB)
	}
	if ns.C(0) != example.TestEnumC || ns.C(1) != example.TestEnumA {
		t.Fatalf("C: got [%d,%d]", ns.C(0), ns.C(1))
	}
	if ns.D(0) != 0x7FFFFFFFFFFFFFFF || ns.D(1) != -0x7FFFFFFFFFFFFFFF {
		t.Fatalf("D: got [%d,%d]", ns.D(0), ns.D(1))
	}

	// Object API round-trip.
	native := ns.UnPack()
	builder2 := flatbuffers.NewBuilder(128)
	native.Pack(builder2)
	buf2 := builder2.Bytes[builder2.Head():]
	ns2 := &example.NestedStruct{}
	ns2.Init(buf2, 0)

	if ns2.A(0) != 10 || ns2.A(1) != 20 {
		t.Fatal("round trip: A mismatch")
	}
	if ns2.D(0) != 0x7FFFFFFFFFFFFFFF {
		t.Fatal("round trip: D mismatch")
	}
}
