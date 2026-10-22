package flatbuffers

// reflect_property_test.go — property-based tests for the FlatBuffers
// reflection runtime.
//
// Property tests verify invariants that must hold for any well-formed .bfbs
// schema, regardless of which tables, enums, or fields it declares.  Each
// test loads monster_test.bfbs and asserts a structural property over the
// entire schema graph.
//
// Properties tested:
//   - Every object has a non-empty name.
//   - Every field has a non-nil type.
//   - Every enum has at least one value.
//   - Enum values within a non-bit-flag enum are in non-descending order.
//   - Every union's first value is NONE with discriminant 0.
//   - vtable offsets for every field are even numbers ≥ 4.

import (
	"sort"
	"testing"
)

// TestProperty_AllObjectsHaveNames asserts that every object returned by
// schema.Objects() has a non-empty name string.  An empty name indicates
// a corrupt or incomplete schema buffer.
func TestProperty_AllObjectsHaveNames(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	objects := schema.Objects()
	if len(objects) == 0 {
		t.Fatal("schema has no objects")
	}
	for i, obj := range objects {
		if obj.Name() == "" {
			t.Errorf("Objects()[%d].Name() is empty", i)
		}
	}
}

// TestProperty_AllFieldsHaveTypes asserts that every field in every object
// has a non-nil Type() descriptor.  A nil type would prevent any field
// accessor (GetFieldString, GetFieldInt, etc.) from working correctly.
func TestProperty_AllFieldsHaveTypes(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	for _, obj := range schema.Objects() {
		for _, f := range obj.Fields() {
			if f.Type() == nil {
				t.Errorf("Object %q field %q has nil Type()", obj.Name(), f.Name())
			}
		}
	}
}

// TestProperty_AllEnumsHaveValues asserts that every enum and union descriptor
// has at least one declared value.  A zero-value enum is not valid according
// to the FlatBuffers specification and would make enum lookup impossible.
func TestProperty_AllEnumsHaveValues(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	enums := schema.Enums()
	if len(enums) == 0 {
		t.Fatal("schema has no enums")
	}
	for _, e := range enums {
		vals := e.Values()
		if len(vals) == 0 {
			t.Errorf("Enum %q has no values", e.Name())
		}
	}
}

// TestProperty_EnumValuesAreSorted asserts that enum values within each
// non-union enum appear in non-descending order of their integer discriminant.
// The FlatBuffers specification and flatc both guarantee this ordering in .bfbs
// output, so any violation indicates schema corruption.
func TestProperty_EnumValuesAreSorted(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	for _, e := range schema.Enums() {
		vals := e.Values()
		numeric := make([]int64, len(vals))
		for i, v := range vals {
			numeric[i] = v.Value()
		}
		if !sort.SliceIsSorted(numeric, func(i, j int) bool {
			return numeric[i] <= numeric[j]
		}) {
			t.Errorf("Enum %q values are not in non-descending order: %v",
				e.Name(), numeric)
		}
	}
}

// TestProperty_UnionFirstValueIsNone asserts that for every union type
// descriptor (IsUnion == true) the first declared value has the name "NONE"
// and integer discriminant 0.  flatc always emits a synthetic NONE variant
// first to serve as the "no value" sentinel.
func TestProperty_UnionFirstValueIsNone(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	for _, e := range schema.Enums() {
		if !e.IsUnion() {
			continue
		}
		vals := e.Values()
		if len(vals) == 0 {
			t.Errorf("Union %q has no values", e.Name())
			continue
		}
		if vals[0].Name() != "NONE" {
			t.Errorf("Union %q first value name = %q, want %q",
				e.Name(), vals[0].Name(), "NONE")
		}
		if vals[0].Value() != 0 {
			t.Errorf("Union %q first value discriminant = %d, want 0",
				e.Name(), vals[0].Value())
		}
	}
}

// TestProperty_FieldOffsetsAreEven asserts that the vtable byte offset
// returned by ReflectionField.Offset() is even for every field.  For table
// fields, vtable offsets start at 4 and advance in 2-byte steps, so any odd
// or sub-4 value indicates a malformed schema.  For struct fields, Offset()
// is the byte position within the fixed-size inline struct; it is always even
// (natural alignment) but may be 0 for the first field.
func TestProperty_FieldOffsetsAreEven(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	for _, obj := range schema.Objects() {
		isStruct := obj.IsStruct()
		for _, f := range obj.Fields() {
			off := f.Offset()
			if isStruct {
				// Struct offsets are byte positions within the inline struct;
				// can be 0, 2, etc. — just verify even alignment.
				if off%2 != 0 {
					t.Errorf("Struct %q field %q: Offset %d is odd; struct offsets must be even",
						obj.Name(), f.Name(), off)
				}
				continue
			}
			// Table fields: skip 0 (absent vtable slot for deprecated fields).
			if off == 0 {
				continue
			}
			if off < 4 {
				t.Errorf("Table %q field %q: Offset %d < 4 (minimum vtable slot offset)",
					obj.Name(), f.Name(), off)
			}
			if off%2 != 0 {
				t.Errorf("Table %q field %q: Offset %d is odd; vtable offsets must be even",
					obj.Name(), f.Name(), off)
			}
		}
	}
}

// TestProperty_RootTableIsInObjectsList verifies that the root table returned
// by schema.RootTable() is also present in schema.Objects().  This confirms
// that the root_table pointer in the Schema table references an entry in the
// objects vector rather than an out-of-band location.
func TestProperty_RootTableIsInObjectsList(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	root := schema.RootTable()
	if root == nil {
		t.Fatal("RootTable() returned nil")
	}
	rootName := root.Name()

	found := false
	for _, obj := range schema.Objects() {
		if obj.Name() == rootName {
			found = true
			break
		}
	}
	if !found {
		t.Errorf("RootTable name %q not found in Objects() list", rootName)
	}
}

// TestProperty_EnumNamesAreUnique verifies that within a given schema no two
// enums share the same fully-qualified name.  Duplicate names would make
// EnumByName non-deterministic.
func TestProperty_EnumNamesAreUnique(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	seen := make(map[string]bool)
	for i, e := range schema.Enums() {
		name := e.Name()
		if seen[name] {
			t.Errorf("Enums()[%d] name %q already seen (duplicate)", i, name)
		}
		seen[name] = true
	}
}

// TestProperty_ObjectNamesAreUnique verifies that within a given schema no two
// objects share the same fully-qualified name.  Duplicate names would make
// ObjectByName non-deterministic.
func TestProperty_ObjectNamesAreUnique(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	seen := make(map[string]bool)
	for i, obj := range schema.Objects() {
		name := obj.Name()
		if seen[name] {
			t.Errorf("Objects()[%d] name %q already seen (duplicate)", i, name)
		}
		seen[name] = true
	}
}

// TestProperty_UnionVariantsHaveNonNilUnionType verifies that every union
// variant (value) other than NONE has a non-nil UnionType() describing the
// table type it carries.  A nil UnionType on a non-NONE variant would prevent
// union dispatch from working.
func TestProperty_UnionVariantsHaveNonNilUnionType(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	for _, e := range schema.Enums() {
		if !e.IsUnion() {
			continue
		}
		for _, v := range e.Values() {
			if v.Value() == 0 {
				// NONE — UnionType may legitimately be nil.
				continue
			}
			if v.UnionType() == nil {
				t.Errorf("Union %q variant %q (value=%d) has nil UnionType()",
					e.Name(), v.Name(), v.Value())
			}
		}
	}
}
