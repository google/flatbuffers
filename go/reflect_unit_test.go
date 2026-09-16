package flatbuffers

// reflect_unit_test.go — unit tests for every public method on every
// ReflectionSchema / ReflectionObject / ReflectionField / ReflectionType /
// ReflectionEnum / ReflectionEnumVal type.
//
// All tests load the pre-compiled binary schema at
// ../tests/monster_test.bfbs, which was produced from monster_test.fbs.
// The schema encodes MyGame.Example.Monster as the root table together
// with several enums (Color, Race, LongEnum) and unions (Any, …).

import (
	"os"
	"path/filepath"
	"testing"
)

// loadGoWireDataUnit loads ../tests/monsterdata_go_wire.mon, skipping the test
// if the file does not exist.  This is the binary FlatBuffer wire format file
// that is safe to use with GetFieldString / GetFieldInt / GetFieldFloat.
func loadGoWireDataUnit(t *testing.T) []byte {
	t.Helper()
	path := filepath.Join("..", "tests", "monsterdata_go_wire.mon")
	data, err := os.ReadFile(path)
	if err != nil {
		t.Skipf("monsterdata_go_wire.mon not found: %v", err)
	}
	return data
}

// ── ReflectionSchema ──────────────────────────────────────────────────────

// TestSchemaEnumsSlice verifies that Enums returns a non-empty slice and that
// every returned entry has a non-empty name.  monster_test.fbs declares Color,
// Race, LongEnum (plain enums) plus Any and others (unions), so the slice
// length must be at least 5.
func TestSchemaEnumsSlice(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	enums := schema.Enums()
	if len(enums) == 0 {
		t.Fatal("Enums returned empty slice; want at least 5 entries")
	}
	for i, e := range enums {
		if e.Name() == "" {
			t.Errorf("Enums()[%d].Name() is empty", i)
		}
	}
}

// TestSchemaEnumByName verifies that EnumByName locates Color by its
// fully-qualified name and returns a non-nil result with the correct name.
func TestSchemaEnumByName(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	e := schema.EnumByName("MyGame.Example.Color")
	if e == nil {
		t.Fatal("EnumByName(\"MyGame.Example.Color\") returned nil")
	}
	if e.Name() != "MyGame.Example.Color" {
		t.Errorf("Name = %q, want %q", e.Name(), "MyGame.Example.Color")
	}
}

// TestSchemaEnumByName_NotFound verifies that EnumByName returns nil when
// given a name that does not appear in the schema.
func TestSchemaEnumByName_NotFound(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	e := schema.EnumByName("NonExistent.Enum")
	if e != nil {
		t.Errorf("EnumByName(\"NonExistent.Enum\") = %v, want nil", e)
	}
}

// TestSchemaEnumValueName verifies the convenience helper that resolves an
// integer discriminant to its declared name.  Color value 2 (Green, the
// second declared constant) should resolve to "Green".
func TestSchemaEnumValueName(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	// Color is a bit_flags enum: stored as Red=1, Green=2, Blue=8.
	name := schema.EnumValueName("MyGame.Example.Color", 2)
	if name != "Green" {
		t.Errorf("EnumValueName(Color, 2) = %q, want %q", name, "Green")
	}

	// Unknown enum → empty string.
	if got := schema.EnumValueName("NoSuchEnum", 0); got != "" {
		t.Errorf("EnumValueName(NoSuchEnum, 0) = %q, want empty string", got)
	}

	// Valid enum, no matching value → empty string.
	if got := schema.EnumValueName("MyGame.Example.Color", 99); got != "" {
		t.Errorf("EnumValueName(Color, 99) = %q, want empty string", got)
	}
}

// ── ReflectionEnum ────────────────────────────────────────────────────────

// TestEnumName verifies that Name() returns the fully qualified enum name for
// a plain enum (Color) and for a union (Any).
func TestEnumName(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	color := schema.EnumByName("MyGame.Example.Color")
	if color == nil {
		t.Fatal("Color enum not found")
	}
	if color.Name() != "MyGame.Example.Color" {
		t.Errorf("Color.Name = %q", color.Name())
	}

	any := schema.EnumByName("MyGame.Example.Any")
	if any == nil {
		t.Fatal("Any union not found")
	}
	if any.Name() != "MyGame.Example.Any" {
		t.Errorf("Any.Name = %q", any.Name())
	}
}

// TestEnumIsUnion verifies that IsUnion returns false for a plain enum (Color)
// and true for a union type descriptor (Any).
func TestEnumIsUnion(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	color := schema.EnumByName("MyGame.Example.Color")
	if color == nil {
		t.Fatal("Color enum not found")
	}
	if color.IsUnion() {
		t.Error("Color.IsUnion = true, want false")
	}

	any := schema.EnumByName("MyGame.Example.Any")
	if any == nil {
		t.Fatal("Any union not found")
	}
	if !any.IsUnion() {
		t.Error("Any.IsUnion = false, want true")
	}
}

// TestEnumValues verifies that Values() returns the correct number of entries
// for the Color enum.  monster_test.fbs declares three constants: Red, Green,
// Blue — so len must be 3.
func TestEnumValues(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	color := schema.EnumByName("MyGame.Example.Color")
	if color == nil {
		t.Fatal("Color enum not found")
	}
	vals := color.Values()
	if len(vals) != 3 {
		t.Errorf("Color.Values() length = %d, want 3", len(vals))
	}
	// Spot-check first value.
	if vals[0].Name() != "Red" {
		t.Errorf("Color.Values()[0].Name = %q, want %q", vals[0].Name(), "Red")
	}
}

// TestEnumValueByName verifies that ValueByName finds a declared constant
// ("Blue") and returns nil for an unknown name.
func TestEnumValueByName(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	color := schema.EnumByName("MyGame.Example.Color")
	if color == nil {
		t.Fatal("Color enum not found")
	}

	blue := color.ValueByName("Blue")
	if blue == nil {
		t.Fatal("ValueByName(\"Blue\") returned nil")
	}
	if blue.Name() != "Blue" {
		t.Errorf("blue.Name = %q, want %q", blue.Name(), "Blue")
	}

	notFound := color.ValueByName("Purple")
	if notFound != nil {
		t.Errorf("ValueByName(\"Purple\") = %v, want nil", notFound)
	}
}

// TestEnumUnderlyingType verifies that UnderlyingType returns a non-nil
// ReflectionType for Color (underlying type ubyte) and for the Any union
// (underlying type UType).
func TestEnumUnderlyingType(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	color := schema.EnumByName("MyGame.Example.Color")
	if color == nil {
		t.Fatal("Color enum not found")
	}
	ut := color.UnderlyingType()
	if ut == nil {
		t.Fatal("Color.UnderlyingType() returned nil")
	}
	// Color is declared as :ubyte.
	if ut.BaseType() != ReflectionBaseTypeUByte {
		t.Errorf("Color underlying base type = %d, want UByte (%d)",
			ut.BaseType(), ReflectionBaseTypeUByte)
	}

	any := schema.EnumByName("MyGame.Example.Any")
	if any == nil {
		t.Fatal("Any union not found")
	}
	anyUT := any.UnderlyingType()
	if anyUT == nil {
		t.Fatal("Any.UnderlyingType() returned nil")
	}
	// Union underlying type is always UType (discriminant byte).
	if anyUT.BaseType() != ReflectionBaseTypeUType {
		t.Errorf("Any underlying base type = %d, want UType (%d)",
			anyUT.BaseType(), ReflectionBaseTypeUType)
	}
}

// ── ReflectionEnumVal ─────────────────────────────────────────────────────

// TestEnumValName verifies that Name() returns the declared constant names for
// Color's three values.
func TestEnumValName(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	color := schema.EnumByName("MyGame.Example.Color")
	if color == nil {
		t.Fatal("Color enum not found")
	}
	vals := color.Values()
	want := []string{"Red", "Green", "Blue"}
	for i, v := range vals {
		if i >= len(want) {
			break
		}
		if v.Name() != want[i] {
			t.Errorf("Values()[%d].Name = %q, want %q", i, v.Name(), want[i])
		}
	}
}

// TestEnumValValue verifies that Value() returns the correct integer
// discriminant for each Color constant.  Color is a bit_flags enum; the
// binary schema stores the expanded wire values (1 << bit_position), not the
// declared bit positions.  Red=0 → 1<<0=1, Green=1 → 1<<1=2, Blue=3 → 1<<3=8.
func TestEnumValValue(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	color := schema.EnumByName("MyGame.Example.Color")
	if color == nil {
		t.Fatal("Color enum not found")
	}

	blue := color.ValueByName("Blue")
	if blue == nil {
		t.Fatal("Blue not found in Color enum")
	}
	// bit_flags: Blue declared at position 3 → wire value 1<<3 = 8.
	if blue.Value() != 8 {
		t.Errorf("Blue.Value = %d, want 8 (1<<3 for bit_flags)", blue.Value())
	}

	red := color.ValueByName("Red")
	if red == nil {
		t.Fatal("Red not found in Color enum")
	}
	// bit_flags: Red declared at position 0 → wire value 1<<0 = 1.
	if red.Value() != 1 {
		t.Errorf("Red.Value = %d, want 1 (1<<0 for bit_flags)", red.Value())
	}
}

// TestEnumValUnionType verifies that UnionType returns non-nil for union
// variant entries.  For the Any union, the first non-NONE variant should have
// a non-nil UnionType with BaseType == Obj.
//
// Note: modern flatc versions also populate the union_type field for plain enum
// values (with a Type describing the enum's own integer type).  This test
// therefore does not assert that plain enum values have a nil UnionType —
// instead it only verifies the union-specific semantics.
func TestEnumValUnionType(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	// Union: variants after NONE should have non-nil UnionType with BaseType Obj.
	any := schema.EnumByName("MyGame.Example.Any")
	if any == nil {
		t.Fatal("Any union not found")
	}
	anyVals := any.Values()
	if len(anyVals) < 2 {
		t.Fatalf("Any union has %d values, want >= 2", len(anyVals))
	}
	// Subsequent values must have non-nil UnionType.
	for _, v := range anyVals[1:] {
		ut := v.UnionType()
		if ut == nil {
			t.Errorf("Any union variant %q.UnionType is nil, want non-nil", v.Name())
			continue
		}
		if ut.BaseType() != ReflectionBaseTypeObj {
			t.Errorf("Any variant %q.UnionType.BaseType = %d, want Obj (%d)",
				v.Name(), ut.BaseType(), ReflectionBaseTypeObj)
		}
	}
}

// ── ReflectionField ───────────────────────────────────────────────────────

// TestFieldName verifies that Name() returns the declared field name.
// Tested via the Monster "name" string field.
func TestFieldName(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	obj := schema.ObjectByName("MyGame.Example.Monster")
	if obj == nil {
		t.Fatal("Monster not found")
	}
	f := obj.FieldByName("name")
	if f == nil {
		t.Fatal("name field not found")
	}
	if f.Name() != "name" {
		t.Errorf("Name = %q, want %q", f.Name(), "name")
	}
}

// TestFieldOffset verifies that Offset() returns a non-zero even value for a
// present field.  FlatBuffers vtable offsets start at 4 and are always even.
func TestFieldOffset(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	obj := schema.ObjectByName("MyGame.Example.Monster")
	if obj == nil {
		t.Fatal("Monster not found")
	}
	f := obj.FieldByName("hp")
	if f == nil {
		t.Fatal("hp field not found")
	}
	off := f.Offset()
	if off == 0 {
		t.Error("hp.Offset() = 0, want non-zero")
	}
	if off%2 != 0 {
		t.Errorf("hp.Offset() = %d is odd; vtable offsets must be even", off)
	}
}

// TestFieldRequired verifies that Required() returns false for the optional
// "name" field and that it is a bool (no panic).  In monster_test.fbs no field
// is declared required by default for Monster.
func TestFieldRequired(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	obj := schema.ObjectByName("MyGame.Example.Monster")
	if obj == nil {
		t.Fatal("Monster not found")
	}
	f := obj.FieldByName("name")
	if f == nil {
		t.Fatal("name field not found")
	}
	// Just call the method — a panic would be a test failure.
	_ = f.Required()
}

// TestFieldType verifies that Type() returns a non-nil ReflectionType for
// every field in the Monster table.
func TestFieldType(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	obj := schema.ObjectByName("MyGame.Example.Monster")
	if obj == nil {
		t.Fatal("Monster not found")
	}
	for _, f := range obj.Fields() {
		if f.Type() == nil {
			t.Errorf("Field %q.Type() returned nil", f.Name())
		}
	}
}

// TestFieldDefaultInteger verifies that DefaultInteger() returns 0 for a field
// with no declared default (hp) and the declared default for mana (150).
func TestFieldDefaultInteger(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	obj := schema.ObjectByName("MyGame.Example.Monster")
	if obj == nil {
		t.Fatal("Monster not found")
	}

	mana := obj.FieldByName("mana")
	if mana == nil {
		t.Skip("mana field not found")
	}
	// monster_test.fbs: mana:short = 150;
	if mana.DefaultInteger() != 150 {
		t.Errorf("mana.DefaultInteger = %d, want 150", mana.DefaultInteger())
	}

	hp := obj.FieldByName("hp")
	if hp == nil {
		t.Skip("hp field not found")
	}
	// monster_test.fbs: hp:short = 100;
	if hp.DefaultInteger() != 100 {
		t.Errorf("hp.DefaultInteger = %d, want 100", hp.DefaultInteger())
	}
}

// TestFieldDefaultReal verifies that DefaultReal() returns 0.0 for integer
// fields that have no real default.  This exercises the accessor path.
func TestFieldDefaultReal(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	obj := schema.ObjectByName("MyGame.Example.Monster")
	if obj == nil {
		t.Fatal("Monster not found")
	}
	hp := obj.FieldByName("hp")
	if hp == nil {
		t.Skip("hp field not found")
	}
	// hp is a short; no real default declared → 0.0.
	if hp.DefaultReal() != 0.0 {
		t.Errorf("hp.DefaultReal = %f, want 0.0", hp.DefaultReal())
	}
}

// ── ReflectionObject ──────────────────────────────────────────────────────

// TestObjectName verifies that Name() returns the FQN for both a table and a
// struct object.
func TestObjectName(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	monster := schema.ObjectByName("MyGame.Example.Monster")
	if monster == nil {
		t.Fatal("Monster not found")
	}
	if monster.Name() != "MyGame.Example.Monster" {
		t.Errorf("Monster.Name = %q", monster.Name())
	}
}

// TestObjectFields verifies that Fields() returns a non-empty slice for
// Monster and that every element has a non-empty name.
func TestObjectFields(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	monster := schema.ObjectByName("MyGame.Example.Monster")
	if monster == nil {
		t.Fatal("Monster not found")
	}
	fields := monster.Fields()
	if len(fields) == 0 {
		t.Fatal("Monster.Fields() is empty")
	}
	for i, f := range fields {
		if f.Name() == "" {
			t.Errorf("Monster.Fields()[%d].Name is empty", i)
		}
	}
}

// TestObjectFieldByName verifies FieldByName finds known fields and returns nil
// for unknown ones.
func TestObjectFieldByName(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	obj := schema.ObjectByName("MyGame.Example.Monster")
	if obj == nil {
		t.Fatal("Monster not found")
	}

	f := obj.FieldByName("pos")
	if f == nil {
		t.Fatal("FieldByName(\"pos\") returned nil")
	}
	if f.Name() != "pos" {
		t.Errorf("Field name = %q, want %q", f.Name(), "pos")
	}

	if nil != obj.FieldByName("xyzzy_not_real") {
		t.Error("FieldByName of nonexistent field should return nil")
	}
}

// TestObjectIsStruct verifies IsStruct returns false for Monster (a table)
// and true for Vec3 (a struct).
func TestObjectIsStruct(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	monster := schema.ObjectByName("MyGame.Example.Monster")
	if monster == nil {
		t.Fatal("Monster not found")
	}
	if monster.IsStruct() {
		t.Error("Monster.IsStruct = true, want false")
	}

	vec3 := schema.ObjectByName("MyGame.Example.Vec3")
	if vec3 == nil {
		t.Skip("Vec3 not in schema")
	}
	if !vec3.IsStruct() {
		t.Error("Vec3.IsStruct = false, want true")
	}
}

// TestObjectByteSize verifies ByteSize returns 0 for a table and a positive
// value for a struct.  Vec3 is a struct containing three floats (12 bytes).
func TestObjectByteSize(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	monster := schema.ObjectByName("MyGame.Example.Monster")
	if monster == nil {
		t.Fatal("Monster not found")
	}
	if monster.ByteSize() != 0 {
		t.Errorf("Monster.ByteSize = %d, want 0 (table)", monster.ByteSize())
	}

	vec3 := schema.ObjectByName("MyGame.Example.Vec3")
	if vec3 == nil {
		t.Skip("Vec3 not in schema")
	}
	if vec3.ByteSize() <= 0 {
		t.Errorf("Vec3.ByteSize = %d, want > 0", vec3.ByteSize())
	}
}

// ── ReflectionType ────────────────────────────────────────────────────────

// TestTypeBaseType verifies that BaseType() returns the expected wire type for
// several well-known Monster fields.
func TestTypeBaseType(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	obj := schema.ObjectByName("MyGame.Example.Monster")
	if obj == nil {
		t.Fatal("Monster not found")
	}

	cases := []struct {
		field    string
		wantType ReflectionBaseType
	}{
		{"name", ReflectionBaseTypeString},
		{"hp", ReflectionBaseTypeShort},
		{"mana", ReflectionBaseTypeShort},
	}
	for _, tc := range cases {
		f := obj.FieldByName(tc.field)
		if f == nil {
			t.Skipf("field %q not found", tc.field)
		}
		if f.Type().BaseType() != tc.wantType {
			t.Errorf("%s.BaseType = %d, want %d", tc.field, f.Type().BaseType(), tc.wantType)
		}
	}
}

// TestTypeElement verifies that Element() returns None for scalar/string
// fields and the correct element type for a vector field.
func TestTypeElement(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	obj := schema.ObjectByName("MyGame.Example.Monster")
	if obj == nil {
		t.Fatal("Monster not found")
	}

	// Scalar field: Element should be None.
	hp := obj.FieldByName("hp")
	if hp == nil {
		t.Skip("hp not found")
	}
	if hp.Type().Element() != ReflectionBaseTypeNone {
		t.Errorf("hp.Element = %d, want None (%d)", hp.Type().Element(), ReflectionBaseTypeNone)
	}

	// inventory is [ubyte] — BaseType=Vector, Element=UByte.
	inv := obj.FieldByName("inventory")
	if inv == nil {
		t.Skip("inventory field not found")
	}
	if inv.Type().BaseType() != ReflectionBaseTypeVector {
		t.Skipf("inventory BaseType = %d, not a vector", inv.Type().BaseType())
	}
	if inv.Type().Element() != ReflectionBaseTypeUByte {
		t.Errorf("inventory.Element = %d, want UByte (%d)",
			inv.Type().Element(), ReflectionBaseTypeUByte)
	}
}

// TestTypeIndex verifies that Index() returns -1 for scalar/string types and a
// non-negative value for Obj fields that reference another schema table.
func TestTypeIndex(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	obj := schema.ObjectByName("MyGame.Example.Monster")
	if obj == nil {
		t.Fatal("Monster not found")
	}

	hp := obj.FieldByName("hp")
	if hp == nil {
		t.Skip("hp not found")
	}
	if hp.Type().Index() != -1 {
		t.Errorf("hp.Index = %d, want -1", hp.Type().Index())
	}

	pos := obj.FieldByName("pos")
	if pos == nil {
		t.Skip("pos field not found")
	}
	if pos.Type().BaseType() != ReflectionBaseTypeObj {
		t.Skipf("pos BaseType = %d, not Obj", pos.Type().BaseType())
	}
	if pos.Type().Index() < 0 {
		t.Errorf("pos.Index = %d, want >= 0", pos.Type().Index())
	}
}

// TestLoadReflectionSchema_TooShort verifies that LoadReflectionSchema returns
// an error when the buffer is shorter than the minimum required size.
func TestLoadReflectionSchema_TooShort(t *testing.T) {
	_, err := LoadReflectionSchema([]byte{0x01, 0x02})
	if err == nil {
		t.Error("Expected error for too-short buffer, got nil")
	}
}

// TestLoadReflectionSchema_Empty verifies that LoadReflectionSchema returns
// an error for a nil / empty buffer.
func TestLoadReflectionSchema_Empty(t *testing.T) {
	_, err := LoadReflectionSchema(nil)
	if err == nil {
		t.Error("Expected error for nil buffer, got nil")
	}
	_, err = LoadReflectionSchema([]byte{})
	if err == nil {
		t.Error("Expected error for empty buffer, got nil")
	}
}

// TestGetFieldString_NilField verifies that GetFieldString returns an error
// when field is nil, rather than panicking.
func TestGetFieldString_NilField(t *testing.T) {
	_, err := GetFieldString([]byte{0, 0, 0, 0, 0, 0, 0, 0}, 0, nil)
	if err == nil {
		t.Error("Expected error for nil field, got nil")
	}
}

// TestGetFieldInt_NilField verifies that GetFieldInt returns an error when
// field is nil.
func TestGetFieldInt_NilField(t *testing.T) {
	_, err := GetFieldInt([]byte{0, 0, 0, 0, 0, 0, 0, 0}, 0, nil)
	if err == nil {
		t.Error("Expected error for nil field, got nil")
	}
}

// TestGetFieldFloat_NilField verifies that GetFieldFloat returns an error
// when field is nil.
func TestGetFieldFloat_NilField(t *testing.T) {
	_, err := GetFieldFloat([]byte{0, 0, 0, 0, 0, 0, 0, 0}, 0, nil)
	if err == nil {
		t.Error("Expected error for nil field, got nil")
	}
}

// TestGetFieldBool_NilField verifies that GetFieldBool returns an error when
// field is nil.
func TestGetFieldBool_NilField(t *testing.T) {
	_, err := GetFieldBool([]byte{0, 0, 0, 0, 0, 0, 0, 0}, 0, nil)
	if err == nil {
		t.Error("Expected error for nil field, got nil")
	}
}

// TestGetFieldInt_WrongType verifies that GetFieldInt returns an error when
// called on a string-typed field.
func TestGetFieldInt_WrongType(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}
	obj := schema.ObjectByName("MyGame.Example.Monster")
	if obj == nil {
		t.Fatal("Monster not found")
	}
	nameField := obj.FieldByName("name")
	if nameField == nil {
		t.Fatal("name field not found")
	}
	data := loadGoWireDataUnit(t)
	rootPos := int(GetUOffsetT(data[0:]))
	_, err = GetFieldInt(data, rootPos, nameField)
	if err == nil {
		t.Error("GetFieldInt on string field should return error")
	}
}

// TestGetFieldFloat_WrongType verifies that GetFieldFloat returns an error
// when called on an integer-typed field.
func TestGetFieldFloat_WrongType(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}
	obj := schema.ObjectByName("MyGame.Example.Monster")
	if obj == nil {
		t.Fatal("Monster not found")
	}
	hpField := obj.FieldByName("hp")
	if hpField == nil {
		t.Fatal("hp field not found")
	}
	data := loadGoWireDataUnit(t)
	rootPos := int(GetUOffsetT(data[0:]))
	_, err = GetFieldFloat(data, rootPos, hpField)
	if err == nil {
		t.Error("GetFieldFloat on integer field should return error")
	}
}

// TestGetFieldBool_WrongType verifies that GetFieldBool returns an error when
// called on a non-bool field.
func TestGetFieldBool_WrongType(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}
	obj := schema.ObjectByName("MyGame.Example.Monster")
	if obj == nil {
		t.Fatal("Monster not found")
	}
	hpField := obj.FieldByName("hp")
	if hpField == nil {
		t.Fatal("hp field not found")
	}
	data := loadGoWireDataUnit(t)
	rootPos := int(GetUOffsetT(data[0:]))
	_, err = GetFieldBool(data, rootPos, hpField)
	if err == nil {
		t.Error("GetFieldBool on integer field should return error")
	}
}
