package flatbuffers

// reflect_integration_test.go — end-to-end integration tests for the
// FlatBuffers reflection runtime.
//
// These tests exercise full workflows: loading a binary schema, locating
// objects and enums, iterating fields, resolving union variants, and reading
// live field values from real data buffers.
//
// Test data:
//   - Schema:  ../tests/monster_test.bfbs     (compiled from monster_test.fbs)
//   - Wire:    ../tests/monsterdata_go_wire.mon (Go-specific binary wire format)

import (
	"os"
	"path/filepath"
	"testing"
)

// loadGoWireDataIntegration loads ../tests/monsterdata_go_wire.mon.  It skips
// the test if the file does not exist.  This file is a proper binary FlatBuffer
// (root-offset-prefixed, with the MONS file identifier) suitable for field
// reads via GetFieldString / GetFieldInt / GetFieldFloat.
func loadGoWireDataIntegration(t *testing.T) []byte {
	t.Helper()
	path := filepath.Join("..", "tests", "monsterdata_go_wire.mon")
	data, err := os.ReadFile(path)
	if err != nil {
		t.Skipf("monsterdata_go_wire.mon not found: %v", err)
	}
	return data
}

// TestIntegration_LoadSchemaAndReadAllFields loads the binary schema, locates
// the Monster root table, iterates every declared field, and reads each one
// from the Go wire buffer using the appropriate typed accessor.  The test
// verifies that no panics occur and that all reads return either a value or a
// well-typed error.
func TestIntegration_LoadSchemaAndReadAllFields(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	data := loadGoWireDataIntegration(t)
	rootPos := int(GetUOffsetT(data[0:]))

	obj := schema.ObjectByName("MyGame.Example.Monster")
	if obj == nil {
		t.Fatal("MyGame.Example.Monster not found in schema")
	}

	fields := obj.Fields()
	if len(fields) == 0 {
		t.Fatal("Monster has no fields")
	}

	// Iterate every field and call the type-appropriate accessor.  The only
	// requirement is "no panic" — errors are acceptable for unsupported types.
	for _, f := range fields {
		ft := f.Type()
		if ft == nil {
			t.Errorf("Field %q has nil Type()", f.Name())
			continue
		}
		switch ft.BaseType() {
		case ReflectionBaseTypeString:
			_, _ = GetFieldString(data, rootPos, f)
		case ReflectionBaseTypeBool:
			_, _ = GetFieldBool(data, rootPos, f)
		case ReflectionBaseTypeFloat, ReflectionBaseTypeDouble:
			_, _ = GetFieldFloat(data, rootPos, f)
		default:
			// Integer, object, vector, union — attempt int read;
			// error is fine for non-integer base types.
			_, _ = GetFieldInt(data, rootPos, f)
		}
	}
}

// TestIntegration_EnumLookupAndFieldRead loads the binary schema, finds the
// Color enum, verifies its values match the expected constants from
// monster_test.fbs, then reads the Monster "color" field from the wire buffer
// and resolves its numeric value to the enum name.
func TestIntegration_EnumLookupAndFieldRead(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	// Verify Color enum structure.
	color := schema.EnumByName("MyGame.Example.Color")
	if color == nil {
		t.Fatal("MyGame.Example.Color enum not found")
	}
	if color.IsUnion() {
		t.Error("Color.IsUnion = true, want false")
	}

	// Verify the three declared Color constants and their values.
	//   bit_flags enum: Red=(1<<0)=1, Green=(1<<1)=2, Blue=(1<<3)=8
	expectedVals := map[string]int64{"Red": 1, "Green": 2, "Blue": 8}
	for name, wantVal := range expectedVals {
		v := color.ValueByName(name)
		if v == nil {
			t.Errorf("Color.ValueByName(%q) returned nil", name)
			continue
		}
		if v.Value() != wantVal {
			t.Errorf("Color.%s.Value = %d, want %d", name, v.Value(), wantVal)
		}
	}

	// Read the "color" field from the Monster wire buffer and resolve to name.
	data := loadGoWireDataIntegration(t)
	rootPos := int(GetUOffsetT(data[0:]))

	obj := schema.ObjectByName("MyGame.Example.Monster")
	if obj == nil {
		t.Fatal("Monster not found")
	}
	colorField := obj.FieldByName("color")
	if colorField == nil {
		t.Skip("color field not found in Monster — skipping field read")
	}
	rawVal, err := GetFieldInt(data, rootPos, colorField)
	if err != nil {
		t.Fatalf("GetFieldInt(color): %v", err)
	}
	// Resolve to string using the convenience helper.
	valName := schema.EnumValueName("MyGame.Example.Color", rawVal)
	if valName == "" {
		t.Logf("color field raw value %d has no matching single enum name (may be composite bit flag)", rawVal)
	} else {
		t.Logf("Monster.color = %s (%d)", valName, rawVal)
	}
}

// TestIntegration_UnionVariantResolution loads the schema, locates the Any
// union descriptor, verifies IsUnion is true, checks that the NONE variant
// exists at index 0 with value 0, and confirms each subsequent variant has a
// non-nil UnionType pointing to a real schema object.
func TestIntegration_UnionVariantResolution(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	any := schema.EnumByName("MyGame.Example.Any")
	if any == nil {
		t.Fatal("MyGame.Example.Any union not found")
	}
	if !any.IsUnion() {
		t.Fatal("Any.IsUnion = false, want true")
	}

	vals := any.Values()
	if len(vals) == 0 {
		t.Fatal("Any union has no values")
	}

	// First value must be NONE with discriminant 0.
	if vals[0].Name() != "NONE" {
		t.Errorf("Any.Values()[0].Name = %q, want %q", vals[0].Name(), "NONE")
	}
	if vals[0].Value() != 0 {
		t.Errorf("Any.Values()[0].Value = %d, want 0", vals[0].Value())
	}

	// Variant names and indices must be internally consistent.
	objects := schema.Objects()
	for i, v := range vals[1:] {
		ut := v.UnionType()
		if ut == nil {
			t.Errorf("Any variant [%d] %q has nil UnionType", i+1, v.Name())
			continue
		}
		// Each non-NONE union variant must have BaseType Obj.
		if ut.BaseType() != ReflectionBaseTypeObj {
			t.Errorf("Any variant %q UnionType.BaseType = %d, want Obj (%d)",
				v.Name(), ut.BaseType(), ReflectionBaseTypeObj)
		}
		idx := int(ut.Index())
		if idx < 0 || idx >= len(objects) {
			t.Errorf("Any variant %q UnionType.Index = %d out of objects range [0,%d)",
				v.Name(), idx, len(objects))
		}
	}
}

// TestIntegration_FullSchemaWalk walks every object, every field, every enum,
// and every enum value in the loaded schema.  It asserts that no nil panics
// occur and that every name is a non-empty string, validating overall schema
// structural integrity.
func TestIntegration_FullSchemaWalk(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	// Walk objects.
	for _, obj := range schema.Objects() {
		if obj.Name() == "" {
			t.Error("Object has empty name")
		}
		for _, f := range obj.Fields() {
			if f.Name() == "" {
				t.Errorf("Object %q has a field with empty name", obj.Name())
			}
			if f.Type() == nil {
				t.Errorf("Object %q field %q has nil Type()", obj.Name(), f.Name())
			}
		}
	}

	// Walk enums and union variants.
	for _, e := range schema.Enums() {
		if e.Name() == "" {
			t.Error("Enum has empty name")
		}
		if e.UnderlyingType() == nil {
			t.Errorf("Enum %q has nil UnderlyingType()", e.Name())
		}
		for _, v := range e.Values() {
			if v.Name() == "" {
				t.Errorf("Enum %q has value with empty name", e.Name())
			}
			// For union variants (non-NONE), UnionType must be non-nil.
			if e.IsUnion() && v.Value() != 0 && v.UnionType() == nil {
				t.Errorf("Union %q variant %q has nil UnionType", e.Name(), v.Name())
			}
		}
	}
}

// TestIntegration_GoWireDataRead reads the "name" and "hp" fields from the
// Go-specific binary wire buffer (monsterdata_go_wire.mon) and verifies the
// values are consistent with the monster fixture.
func TestIntegration_GoWireDataRead(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	data := loadGoWireDataIntegration(t)
	rootPos := int(GetUOffsetT(data[0:]))

	obj := schema.ObjectByName("MyGame.Example.Monster")
	if obj == nil {
		t.Fatal("Monster not found")
	}

	nameField := obj.FieldByName("name")
	if nameField == nil {
		t.Fatal("name field not found")
	}
	name, err := GetFieldString(data, rootPos, nameField)
	if err != nil {
		t.Fatalf("GetFieldString(name): %v", err)
	}
	if name == "" {
		t.Error("Monster name is empty in Go wire buffer")
	}
	t.Logf("Monster name (go wire) = %q", name)

	hpField := obj.FieldByName("hp")
	if hpField == nil {
		t.Fatal("hp field not found")
	}
	hp, err := GetFieldInt(data, rootPos, hpField)
	if err != nil {
		t.Fatalf("GetFieldInt(hp): %v", err)
	}
	if hp <= 0 {
		t.Errorf("hp = %d, want > 0", hp)
	}
	t.Logf("Monster hp (go wire) = %d", hp)
}

// TestIntegration_EnumValueNameConvenienceHelper exercises the EnumValueName
// convenience method end-to-end: verifying it correctly maps Race enum values
// to their declared names, returns empty string for missing names, and handles
// unknown enum names gracefully.
func TestIntegration_EnumValueNameConvenienceHelper(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	// Race enum: None=-1, Human=0, Dwarf=1, Elf=2 (from monster_test.fbs).
	cases := []struct {
		value int64
		want  string
	}{
		{-1, "None"},
		{0, "Human"},
		{1, "Dwarf"},
	}
	for _, tc := range cases {
		got := schema.EnumValueName("MyGame.Example.Race", tc.value)
		if got != tc.want {
			t.Errorf("EnumValueName(Race, %d) = %q, want %q", tc.value, got, tc.want)
		}
	}

	// Non-existent enum.
	if got := schema.EnumValueName("NoSuch.Enum", 0); got != "" {
		t.Errorf("EnumValueName for missing enum = %q, want empty", got)
	}

	// Valid enum, no matching value.
	if got := schema.EnumValueName("MyGame.Example.Race", 9999); got != "" {
		t.Errorf("EnumValueName(Race, 9999) = %q, want empty", got)
	}
}
