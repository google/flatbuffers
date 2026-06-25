package flatbuffers

import (
	"os"
	"path/filepath"
	"testing"
)

func loadTestBfbs(t *testing.T) []byte {
	t.Helper()
	path := filepath.Join("..", "tests", "monster_test.bfbs")
	data, err := os.ReadFile(path)
	if err != nil {
		t.Skipf("Binary schema not found: %v", err)
	}
	return data
}

func loadTestData(t *testing.T) []byte {
	t.Helper()
	path := filepath.Join("..", "tests", "monsterdata_go_wire.mon")
	data, err := os.ReadFile(path)
	if err != nil {
		t.Skipf("Binary wire data not found: %v", err)
	}
	return data
}

func TestLoadReflectionSchema(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema failed: %v", err)
	}
	if schema == nil {
		t.Fatal("Schema is nil")
	}
}

func TestSchemaRootTable(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema failed: %v", err)
	}

	root := schema.RootTable()
	if root == nil {
		t.Fatal("Root table is nil")
	}

	name := root.Name()
	if name != "MyGame.Example.Monster" {
		t.Errorf("Root table name = %q, want %q", name, "MyGame.Example.Monster")
	}
}

func TestSchemaObjectByName(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema failed: %v", err)
	}

	obj := schema.ObjectByName("MyGame.Example.Monster")
	if obj == nil {
		t.Fatal("ObjectByName returned nil for MyGame.Example.Monster")
	}
	if obj.Name() != "MyGame.Example.Monster" {
		t.Errorf("Name = %q, want %q", obj.Name(), "MyGame.Example.Monster")
	}

	notFound := schema.ObjectByName("NonExistent")
	if notFound != nil {
		t.Error("ObjectByName should return nil for non-existent name")
	}
}

func TestSchemaFieldByName(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema failed: %v", err)
	}

	obj := schema.ObjectByName("MyGame.Example.Monster")
	if obj == nil {
		t.Fatal("ObjectByName returned nil")
	}

	nameField := obj.FieldByName("name")
	if nameField == nil {
		t.Fatal("FieldByName returned nil for 'name'")
	}
	if nameField.Name() != "name" {
		t.Errorf("Field name = %q, want %q", nameField.Name(), "name")
	}
	if nameField.Type().BaseType() != ReflectionBaseTypeString {
		t.Errorf("Field base type = %d, want String (%d)",
			nameField.Type().BaseType(), ReflectionBaseTypeString)
	}

	hpField := obj.FieldByName("hp")
	if hpField == nil {
		t.Fatal("FieldByName returned nil for 'hp'")
	}
	if hpField.Type().BaseType() != ReflectionBaseTypeShort {
		t.Errorf("hp base type = %d, want Short (%d)",
			hpField.Type().BaseType(), ReflectionBaseTypeShort)
	}

	notFound := obj.FieldByName("nonexistent")
	if notFound != nil {
		t.Error("FieldByName should return nil for non-existent field")
	}
}

func TestSchemaObjectIsStruct(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema failed: %v", err)
	}

	monster := schema.ObjectByName("MyGame.Example.Monster")
	if monster == nil {
		t.Fatal("Monster not found")
	}
	if monster.IsStruct() {
		t.Error("Monster should not be a struct")
	}

	// Vec3 is a struct in monster_test.fbs
	vec3 := schema.ObjectByName("MyGame.Example.Vec3")
	if vec3 == nil {
		t.Skip("Vec3 not found in schema — may not be included")
	}
	if !vec3.IsStruct() {
		t.Error("Vec3 should be a struct")
	}
}

func TestSchemaObjects(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema failed: %v", err)
	}

	objects := schema.Objects()
	if len(objects) == 0 {
		t.Fatal("No objects in schema")
	}

	// monster_test.fbs has several types
	foundMonster := false
	for _, obj := range objects {
		if obj.Name() == "MyGame.Example.Monster" {
			foundMonster = true
			break
		}
	}
	if !foundMonster {
		t.Error("Monster not found in objects list")
	}
}

func TestGetFieldStringFromBuffer(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema failed: %v", err)
	}

	data := loadTestData(t)

	obj := schema.ObjectByName("MyGame.Example.Monster")
	if obj == nil {
		t.Fatal("Monster not found")
	}

	nameField := obj.FieldByName("name")
	if nameField == nil {
		t.Fatal("name field not found")
	}

	// Get root table position
	rootPos := int(GetUOffsetT(data[0:]))

	val, err := GetFieldString(data, rootPos, nameField)
	if err != nil {
		t.Fatalf("GetFieldString failed: %v", err)
	}
	if val != "MyMonster" {
		t.Errorf("name = %q, want %q", val, "MyMonster")
	}
}

func TestGetFieldIntFromBuffer(t *testing.T) {
	bfbs := loadTestBfbs(t)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		t.Fatalf("LoadReflectionSchema failed: %v", err)
	}

	data := loadTestData(t)

	obj := schema.ObjectByName("MyGame.Example.Monster")
	if obj == nil {
		t.Fatal("Monster not found")
	}

	hpField := obj.FieldByName("hp")
	if hpField == nil {
		t.Fatal("hp field not found")
	}

	rootPos := int(GetUOffsetT(data[0:]))

	val, err := GetFieldInt(data, rootPos, hpField)
	if err != nil {
		t.Fatalf("GetFieldInt failed: %v", err)
	}
	// The golden monster has hp=80
	if val != 80 {
		t.Errorf("hp = %d, want 80", val)
	}
}
