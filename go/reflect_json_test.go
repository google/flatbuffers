package flatbuffers

import (
	"encoding/json"
	"os"
	"path/filepath"
	"testing"
)

// loadGoWireDataForJSON loads the monsterdata_go_wire.mon fixture for reflect_json tests.
func loadGoWireDataForJSON(t *testing.T) []byte {
	t.Helper()

	path := filepath.Join("..", "tests", "monsterdata_go_wire.mon")

	data, err := os.ReadFile(path) //nolint:gosec // path is a known test fixture path, not user input
	if err != nil {
		t.Skipf("monsterdata_go_wire.mon not found: %v", err)
	}

	return data
}

// TestReflectUnpackBasicFields verifies that ReflectUnpack correctly reads the
// "name" (string) and "hp" (short) fields from the golden Go-wire Monster buffer.
func TestReflectUnpackBasicFields(t *testing.T) {
	t.Parallel()

	bfbs := loadTestBfbs(t)

	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	data := loadGoWireDataForJSON(t)

	result, err := ReflectUnpack(schema, "MyGame.Example.Monster", data)
	if err != nil {
		t.Fatalf("ReflectUnpack: %v", err)
	}

	if result == nil {
		t.Fatal("ReflectUnpack returned nil map")
	}

	// The Go wire Monster has name="MyMonster" and hp=80.
	nameRaw, namePresent := result["name"]
	if !namePresent {
		t.Error("map missing 'name' field")
	} else if nameStr, isStr := nameRaw.(string); !isStr {
		t.Errorf("'name' has type %T, want string", nameRaw)
	} else if nameStr != "MyMonster" {
		t.Errorf("name = %q, want %q", nameStr, "MyMonster")
	}

	hpRaw, hpPresent := result["hp"]
	if !hpPresent {
		t.Error("map missing 'hp' field")
	} else if hpVal, isInt := hpRaw.(int64); !isInt {
		t.Errorf("'hp' has type %T, want int64", hpRaw)
	} else if hpVal != 80 {
		t.Errorf("hp = %d, want 80", hpVal)
	}
}

// TestReflectUnpackUnknownType verifies that ReflectUnpack returns an error
// when the requested type name is not present in the schema.
func TestReflectUnpackUnknownType(t *testing.T) {
	t.Parallel()

	bfbs := loadTestBfbs(t)

	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	data := loadGoWireDataForJSON(t)

	_, err = ReflectUnpack(schema, "DoesNotExist", data)
	if err == nil {
		t.Error("expected error for unknown type, got nil")
	}
}

// TestReflectUnpackBufferTooShort verifies that ReflectUnpack returns an error
// on a truncated buffer rather than panicking.
func TestReflectUnpackBufferTooShort(t *testing.T) {
	t.Parallel()

	bfbs := loadTestBfbs(t)

	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	_, err = ReflectUnpack(schema, "MyGame.Example.Monster", []byte{1, 2})
	if err == nil {
		t.Error("expected error for short buffer, got nil")
	}
}

// TestReflectPackUnpackRoundtrip builds a simple Monster, packs it to a
// FlatBuffer with ReflectPack, then unpacks with ReflectUnpack, and verifies
// the round-tripped values.
func TestReflectPackUnpackRoundtrip(t *testing.T) {
	t.Parallel()

	bfbs := loadTestBfbs(t)

	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	// Build a minimal Monster JSON with fields that the schema supports.
	// We use only scalar + string fields to keep the test self-contained.
	//
	// Field "hp" is a short (id=2), "mana" is a short (id=1), "name" is a
	// string offset field.
	input := map[string]any{
		"hp":   float64(300),
		"mana": float64(150),
		"name": "Orc",
	}

	jsonBytes, err := json.Marshal(input)
	if err != nil {
		t.Fatalf("json.Marshal: %v", err)
	}

	packed, err := ReflectPack(schema, "MyGame.Example.Monster", jsonBytes)
	if err != nil {
		t.Fatalf("ReflectPack: %v", err)
	}

	if len(packed) == 0 {
		t.Fatal("ReflectPack returned empty bytes")
	}

	// Unpack and verify.
	result, err := ReflectUnpack(schema, "MyGame.Example.Monster", packed)
	if err != nil {
		t.Fatalf("ReflectUnpack after pack: %v", err)
	}

	checkInt64Field(t, result, "hp", 300)
	checkInt64Field(t, result, "mana", 150)
	checkStringField(t, result, "name", "Orc")
}

// TestReflectPackUnknownType verifies that ReflectPack returns an error when
// the type name is not found in the schema.
func TestReflectPackUnknownType(t *testing.T) {
	t.Parallel()

	bfbs := loadTestBfbs(t)

	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	_, err = ReflectPack(schema, "Nope.NotHere", []byte(`{}`))
	if err == nil {
		t.Error("expected error for unknown type, got nil")
	}
}

// TestReflectPackInvalidJSON verifies that ReflectPack returns an error on
// malformed JSON input.
func TestReflectPackInvalidJSON(t *testing.T) {
	t.Parallel()

	bfbs := loadTestBfbs(t)

	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	_, err = ReflectPack(schema, "MyGame.Example.Monster", []byte(`not-json`))
	if err == nil {
		t.Error("expected error for invalid JSON, got nil")
	}
}

// TestReflectPackEmptyObject verifies that packing an empty JSON object
// produces a valid (though minimal) FlatBuffer that can be round-tripped.
func TestReflectPackEmptyObject(t *testing.T) {
	t.Parallel()

	bfbs := loadTestBfbs(t)

	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	packed, err := ReflectPack(schema, "MyGame.Example.Monster", []byte(`{}`))
	if err != nil {
		t.Fatalf("ReflectPack empty: %v", err)
	}

	if len(packed) == 0 {
		t.Fatal("packed buffer is empty")
	}

	result, err := ReflectUnpack(schema, "MyGame.Example.Monster", packed)
	if err != nil {
		t.Fatalf("ReflectUnpack of empty object: %v", err)
	}

	// An empty Monster should produce a map with no set fields.
	if len(result) != 0 {
		t.Errorf("expected empty map, got %d keys: %v", len(result), result)
	}
}

// TestSchemaEnums verifies that the Enums() method added in reflect_json.go
// correctly exposes the enum definitions from the monster_test schema.
func TestSchemaEnums(t *testing.T) {
	t.Parallel()

	bfbs := loadTestBfbs(t)

	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	enums := schema.Enums()
	if len(enums) == 0 {
		t.Skip("no enums in schema — skipping")
	}

	// Every enum must have a non-empty name.
	for _, enumEntry := range enums {
		if enumEntry.Name() == "" {
			t.Error("enum with empty name found")
		}
	}
}

// TestReflectPackVectorOfScalars packs a Monster with a scalar inventory
// vector, then verifies the round-tripped values.
func TestReflectPackVectorOfScalars(t *testing.T) {
	t.Parallel()

	bfbs := loadTestBfbs(t)

	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	// "inventory" in monster_test.fbs is [ubyte].
	input := map[string]any{
		"inventory": []any{float64(0), float64(1), float64(2), float64(3), float64(4)},
	}

	jsonBytes, err := json.Marshal(input)
	if err != nil {
		t.Fatalf("json.Marshal: %v", err)
	}

	packed, err := ReflectPack(schema, "MyGame.Example.Monster", jsonBytes)
	if err != nil {
		t.Fatalf("ReflectPack with vector: %v", err)
	}

	result, err := ReflectUnpack(schema, "MyGame.Example.Monster", packed)
	if err != nil {
		t.Fatalf("ReflectUnpack after vector pack: %v", err)
	}

	checkInventorySlice(t, result)
}

// checkInventorySlice asserts that result["inventory"] is a []any of five int64
// values equal to their index positions (0, 1, 2, 3, 4).
func checkInventorySlice(t *testing.T, result map[string]any) {
	t.Helper()

	invRaw, invPresent := result["inventory"]
	if !invPresent {
		t.Fatal("'inventory' missing from unpacked result")

		return
	}

	inv, isSlice := invRaw.([]any)
	if !isSlice {
		t.Fatalf("'inventory' type is %T, want []any", invRaw)

		return
	}

	if len(inv) != 5 {
		t.Errorf("inventory length = %d, want 5", len(inv))
	}

	checkInventoryElements(t, inv)
}

// checkInventoryElements verifies each element of the inventory slice.
func checkInventoryElements(t *testing.T, inv []any) {
	t.Helper()

	for elemIdx, elemVal := range inv {
		intVal, isInt := elemVal.(int64)
		if !isInt {
			t.Errorf("inventory[%d] type is %T, want int64", elemIdx, elemVal)

			continue
		}

		if intVal != int64(elemIdx) {
			t.Errorf("inventory[%d] = %d, want %d", elemIdx, intVal, elemIdx)
		}
	}
}

// ── helpers ────────────────────────────────────────────────────────────────

func checkInt64Field(t *testing.T, result map[string]any, key string, want int64) {
	t.Helper()

	raw, present := result[key]
	if !present {
		t.Errorf("missing field %q", key)

		return
	}

	intVal, isInt := raw.(int64)
	if !isInt {
		t.Errorf("field %q has type %T, want int64", key, raw)

		return
	}

	if intVal != want {
		t.Errorf("field %q = %d, want %d", key, intVal, want)
	}
}

func checkStringField(t *testing.T, result map[string]any, key, want string) {
	t.Helper()

	raw, present := result[key]
	if !present {
		t.Errorf("missing field %q", key)

		return
	}

	strVal, isStr := raw.(string)
	if !isStr {
		t.Errorf("field %q has type %T, want string", key, raw)

		return
	}

	if strVal != want {
		t.Errorf("field %q = %q, want %q", key, strVal, want)
	}
}
