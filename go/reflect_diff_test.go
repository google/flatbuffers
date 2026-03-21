package flatbuffers

// Tests for DiffBuffers using the monster_test.bfbs schema and hand-crafted
// buffers.  The MonsterHelper functions below construct minimal Monster
// FlatBuffers using the Builder API with the field layout from
// monster_test.fbs.
//
// Monster field vtable offsets (from monster_test.fbs declaration order):
//   field id 0 — pos     (Vec3 struct, voffset=4)
//   field id 1 — mana    (short,       voffset=6)
//   field id 2 — hp      (short,       voffset=8)
//   field id 3 — name    (string,      voffset=10)
//   field id 4 — inventory ([ubyte],   voffset=14)  — id 4 but actual is higher
//
// Because the exact vtable offsets are set by the generated code (not
// reconstructed here), we use only the reflection API to write and read.
// Instead of using generated code, we build buffers with
// [buildSimpleMonster] which places known scalar values and a name string.

import (
	"os"
	"testing"
)

// buildMonsterBuf creates a minimal Monster buffer with the given name, hp,
// and mana values, using the same vtable slot layout that flatc generates for
// MyGame.Example.Monster.
//
// The Monster table in monster_test.fbs declares:
//
//	pos:  Vec3   (field id 0, voffset 4)
//	mana: short  (field id 1, voffset 6)  default 150
//	hp:   short  (field id 2, voffset 8)  default 100
//	name: string (field id 3, voffset 10)
//
// We write only hp, mana, and name (leaving pos absent) so the buffer is
// small and predictable.
func buildMonsterBuf(name string, hp, mana int16) []byte {
	b := NewBuilder(256)

	// Write the name string before starting the object.
	nameOff := b.CreateString(name)

	// The Monster table has many fields; we must declare the same number of
	// vtable slots that flatc-generated code would use so that voffset values
	// match.  Monster has 15 declared fields (ids 0..14); flatc uses:
	//   voffset = 4 + 2*field_id
	// We start the object with the maximum field id + 1 so the vtable is wide
	// enough.  We set only the three fields we care about.
	const numFields = 16 // 0..15 covers all Monster fields
	b.StartObject(numFields)

	// mana (field id 1, voffset 6)
	b.PrependInt16Slot(1, mana, 150)
	// hp (field id 2, voffset 8)
	b.PrependInt16Slot(2, hp, 100)
	// name (field id 3, voffset 10)
	b.PrependUOffsetTSlot(3, nameOff, 0)

	monsterOff := b.EndObject()
	b.Finish(monsterOff)
	return b.FinishedBytes()
}

// buildMonsterBufWithInventory creates a Monster buffer with a byte vector
// in the inventory field (field id 4, voffset 12 in generated code).
// inventory is a [ubyte] field.
func buildMonsterBufWithInventory(name string, hp int16, inventory []byte) []byte {
	b := NewBuilder(256)

	nameOff := b.CreateString(name)

	// Create inventory vector.
	b.StartVector(1, len(inventory), 1)
	for i := len(inventory) - 1; i >= 0; i-- {
		b.PrependByte(inventory[i])
	}
	invOff := b.EndVector(len(inventory))

	const numFields = 16
	b.StartObject(numFields)
	b.PrependInt16Slot(2, hp, 100)
	b.PrependUOffsetTSlot(3, nameOff, 0)
	b.PrependUOffsetTSlot(5, invOff, 0) // inventory is field id 5 in monster_test.fbs
	monsterOff := b.EndObject()
	b.Finish(monsterOff)
	return b.FinishedBytes()
}

// TestDiffBuffersIdentical verifies that identical buffers produce no deltas.
func TestDiffBuffersIdentical(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	buf := buildMonsterBuf("TestMonster", 80, 150)
	deltas, err := DiffBuffers(schema, "MyGame.Example.Monster", buf, buf)
	if err != nil {
		t.Fatalf("DiffBuffers: %v", err)
	}
	if len(deltas) != 0 {
		t.Errorf("expected 0 deltas for identical buffers, got %d: %v", len(deltas), deltas)
	}
}

// TestDiffBuffersNameChanged verifies that a changed name string is detected.
func TestDiffBuffersNameChanged(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	bufA := buildMonsterBuf("OldMonster", 80, 150)
	bufB := buildMonsterBuf("NewMonster", 80, 150)

	deltas, err := DiffBuffers(schema, "MyGame.Example.Monster", bufA, bufB)
	if err != nil {
		t.Fatalf("DiffBuffers: %v", err)
	}

	found := false
	for _, d := range deltas {
		if d.Path == "name" && d.Kind == DeltaChanged {
			if d.OldValue != "OldMonster" {
				t.Errorf("OldValue = %q, want %q", d.OldValue, "OldMonster")
			}
			if d.NewValue != "NewMonster" {
				t.Errorf("NewValue = %q, want %q", d.NewValue, "NewMonster")
			}
			found = true
		}
	}
	if !found {
		t.Errorf("expected delta for 'name' (Changed), got: %v", deltas)
	}
}

// TestDiffBuffersHPChanged verifies that a changed hp integer field is detected.
func TestDiffBuffersHPChanged(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	bufA := buildMonsterBuf("Monster", 80, 150)
	bufB := buildMonsterBuf("Monster", 200, 150)

	deltas, err := DiffBuffers(schema, "MyGame.Example.Monster", bufA, bufB)
	if err != nil {
		t.Fatalf("DiffBuffers: %v", err)
	}

	found := false
	for _, d := range deltas {
		if d.Path == "hp" && d.Kind == DeltaChanged {
			if d.OldValue != int64(80) {
				t.Errorf("OldValue = %v (%T), want int64(80)", d.OldValue, d.OldValue)
			}
			if d.NewValue != int64(200) {
				t.Errorf("NewValue = %v (%T), want int64(200)", d.NewValue, d.NewValue)
			}
			found = true
		}
	}
	if !found {
		t.Errorf("expected delta for 'hp' (Changed), got: %v", deltas)
	}
}

// TestDiffBuffersManaDefault verifies that fields whose values equal the schema
// default are not reported when present in only one buffer.
func TestDiffBuffersManaDefault(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	// bufA writes mana=150 (the default), bufB omits mana.
	bufA := buildMonsterBuf("Monster", 100, 150)
	bufB := buildMonsterBuf("Monster", 100, 150)

	// Confirm identical.
	deltas, err := DiffBuffers(schema, "MyGame.Example.Monster", bufA, bufB)
	if err != nil {
		t.Fatalf("DiffBuffers: %v", err)
	}

	for _, d := range deltas {
		if d.Path == "mana" {
			t.Errorf("unexpected mana delta (both at default): %+v", d)
		}
	}
}

// TestDiffBuffersManaChanged verifies mana delta when both buffers have
// non-default values that differ.
func TestDiffBuffersManaChanged(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	bufA := buildMonsterBuf("Monster", 100, 200)
	bufB := buildMonsterBuf("Monster", 100, 300)

	deltas, err := DiffBuffers(schema, "MyGame.Example.Monster", bufA, bufB)
	if err != nil {
		t.Fatalf("DiffBuffers: %v", err)
	}

	found := false
	for _, d := range deltas {
		if d.Path == "mana" && d.Kind == DeltaChanged {
			found = true
		}
	}
	if !found {
		t.Errorf("expected delta for 'mana', got: %v", deltas)
	}
}

// TestDiffBuffersMultipleChanges verifies that multiple changed fields are all
// reported.
func TestDiffBuffersMultipleChanges(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	bufA := buildMonsterBuf("OldName", 80, 200)
	bufB := buildMonsterBuf("NewName", 300, 400)

	deltas, err := DiffBuffers(schema, "MyGame.Example.Monster", bufA, bufB)
	if err != nil {
		t.Fatalf("DiffBuffers: %v", err)
	}

	deltasByPath := make(map[string]FieldDelta)
	for _, d := range deltas {
		deltasByPath[d.Path] = d
	}

	if _, ok := deltasByPath["name"]; !ok {
		t.Error("expected delta for 'name'")
	}
	if _, ok := deltasByPath["hp"]; !ok {
		t.Error("expected delta for 'hp'")
	}
	if _, ok := deltasByPath["mana"]; !ok {
		t.Error("expected delta for 'mana'")
	}
}

// TestDiffBuffersUnknownType verifies that DiffBuffers returns an error for an
// unknown type name.
func TestDiffBuffersUnknownType(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	buf := buildMonsterBuf("Monster", 100, 150)
	_, err = DiffBuffers(schema, "NoSuchType", buf, buf)
	if err == nil {
		t.Fatal("expected error for unknown type, got nil")
	}
}

// loadGoWireData loads the binary monsterdata_go_wire.mon file used by
// example_test.go.  Skips the test if the file is not present.
func loadGoWireData(t *testing.T) []byte {
	t.Helper()
	data, err := readFile("../tests/monsterdata_go_wire.mon")
	if err != nil {
		t.Skipf("monsterdata_go_wire.mon not found: %v", err)
	}
	return data
}

// readFile is a thin helper to read a file path without importing os directly
// (os is not imported at the top of this test file on purpose to keep the
// test import section minimal).
func readFile(path string) ([]byte, error) {
	return os.ReadFile(path)
}

// TestDiffBuffersWireData verifies DiffBuffers with the repo's wire-format
// binary monster buffer vs itself (should produce no deltas).
func TestDiffBuffersWireData(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	data := loadGoWireData(t)

	deltas, err := DiffBuffers(schema, "MyGame.Example.Monster", data, data)
	if err != nil {
		t.Fatalf("DiffBuffers: %v", err)
	}
	if len(deltas) != 0 {
		t.Errorf("expected 0 deltas when diffing wire data against itself, got %d: %v",
			len(deltas), deltas)
	}
}

// TestDiffBuffersVectorLengthChange verifies that when a vector grows or
// shrinks, Added/Removed deltas are emitted for the extra elements.
func TestDiffBuffersVectorLengthChange(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	bufA := buildMonsterBufWithInventory("Monster", 100, []byte{1, 2, 3})
	bufB := buildMonsterBufWithInventory("Monster", 100, []byte{1, 2, 3, 4, 5})

	deltas, err := DiffBuffers(schema, "MyGame.Example.Monster", bufA, bufB)
	if err != nil {
		t.Fatalf("DiffBuffers: %v", err)
	}

	addedCount := 0
	for _, d := range deltas {
		if d.Kind == DeltaAdded {
			addedCount++
		}
	}
	if addedCount != 2 {
		t.Errorf("expected 2 Added deltas (inventory[3] and inventory[4]), got %d: %v",
			addedCount, deltas)
	}
}

// TestDiffBuffersVectorElementChange verifies that changed vector elements are
// reported.
func TestDiffBuffersVectorElementChange(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema: %v", err)
	}

	bufA := buildMonsterBufWithInventory("Monster", 100, []byte{10, 20, 30})
	bufB := buildMonsterBufWithInventory("Monster", 100, []byte{10, 99, 30})

	deltas, err := DiffBuffers(schema, "MyGame.Example.Monster", bufA, bufB)
	if err != nil {
		t.Fatalf("DiffBuffers: %v", err)
	}

	found := false
	for _, d := range deltas {
		if d.Kind == DeltaChanged {
			found = true
			break
		}
	}
	if !found {
		t.Errorf("expected Changed delta for modified inventory element, got: %v", deltas)
	}
}
