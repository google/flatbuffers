package flatbuffers

// reflect_bench_test.go — benchmark tests for the FlatBuffers reflection
// runtime.
//
// Benchmarks measure the hot-path operations that appear in production
// LiftCloud code: loading a schema buffer, looking up objects and fields by
// name, reading individual field values, and walking the entire schema.
//
// All benchmarks use ../tests/monster_test.bfbs as the binary schema and
// ../tests/monsterdata_test.golden as the live data buffer so that they
// exercise realistic code paths rather than trivial stubs.

import (
	"os"
	"path/filepath"
	"testing"
)

// BenchmarkLoadReflectionSchema measures the time to parse a raw .bfbs byte
// slice into a ReflectionSchema.  This is the one-time startup cost.
func BenchmarkLoadReflectionSchema(b *testing.B) {
	bfbs := benchLoadBfbs(b)
	b.ResetTimer()
	for range b.N {
		_, err := LoadReflectionSchema(bfbs)
		if err != nil {
			b.Fatalf("LoadReflectionSchema: %v", err)
		}
	}
}

// BenchmarkObjectByName measures the cost of locating a table definition by
// its fully-qualified name.  The schema is loaded once outside the timed loop.
func BenchmarkObjectByName(b *testing.B) {
	schema := benchLoadSchema(b)
	b.ResetTimer()
	for range b.N {
		obj := schema.ObjectByName("MyGame.Example.Monster")
		if obj == nil {
			b.Fatal("ObjectByName returned nil")
		}
	}
}

// BenchmarkFieldByName measures the cost of locating a specific field within a
// table using FieldByName.  Both the schema load and the ObjectByName lookup
// are done once outside the timed loop.
func BenchmarkFieldByName(b *testing.B) {
	schema := benchLoadSchema(b)
	obj := schema.ObjectByName("MyGame.Example.Monster")
	if obj == nil {
		b.Fatal("Monster not found")
	}
	b.ResetTimer()
	for range b.N {
		f := obj.FieldByName("name")
		if f == nil {
			b.Fatal("FieldByName returned nil")
		}
	}
}

// BenchmarkGetFieldString measures the cost of reading a string field from a
// live data buffer using a pre-resolved ReflectionField.  This represents the
// per-message read cost in a tight processing loop.
func BenchmarkGetFieldString(b *testing.B) {
	schema := benchLoadSchema(b)
	data := benchLoadData(b)
	rootPos := int(GetUOffsetT(data[0:]))

	obj := schema.ObjectByName("MyGame.Example.Monster")
	if obj == nil {
		b.Fatal("Monster not found")
	}
	nameField := obj.FieldByName("name")
	if nameField == nil {
		b.Fatal("name field not found")
	}

	b.ResetTimer()
	for range b.N {
		_, err := GetFieldString(data, rootPos, nameField)
		if err != nil {
			b.Fatalf("GetFieldString: %v", err)
		}
	}
}

// BenchmarkGetFieldInt measures the cost of reading an integer field (hp:short)
// from a live data buffer using a pre-resolved ReflectionField.
func BenchmarkGetFieldInt(b *testing.B) {
	schema := benchLoadSchema(b)
	data := benchLoadData(b)
	rootPos := int(GetUOffsetT(data[0:]))

	obj := schema.ObjectByName("MyGame.Example.Monster")
	if obj == nil {
		b.Fatal("Monster not found")
	}
	hpField := obj.FieldByName("hp")
	if hpField == nil {
		b.Fatal("hp field not found")
	}

	b.ResetTimer()
	for range b.N {
		_, err := GetFieldInt(data, rootPos, hpField)
		if err != nil {
			b.Fatalf("GetFieldInt: %v", err)
		}
	}
}

// BenchmarkEnumByName measures the cost of locating an enum by its
// fully-qualified name using a linear scan.
func BenchmarkEnumByName(b *testing.B) {
	schema := benchLoadSchema(b)
	b.ResetTimer()
	for range b.N {
		e := schema.EnumByName("MyGame.Example.Color")
		if e == nil {
			b.Fatal("EnumByName returned nil")
		}
	}
}

// BenchmarkEnumValueByName measures the cost of locating a specific enum value
// within an already-resolved ReflectionEnum using a linear scan.
func BenchmarkEnumValueByName(b *testing.B) {
	schema := benchLoadSchema(b)
	color := schema.EnumByName("MyGame.Example.Color")
	if color == nil {
		b.Fatal("Color enum not found")
	}
	b.ResetTimer()
	for range b.N {
		v := color.ValueByName("Blue")
		if v == nil {
			b.Fatal("Blue not found")
		}
	}
}

// BenchmarkFullSchemaWalk measures the cost of iterating every object, every
// field, every enum, and every enum value in the schema.  This represents the
// worst-case startup scan that some introspection tools perform.
func BenchmarkFullSchemaWalk(b *testing.B) {
	schema := benchLoadSchema(b)
	b.ResetTimer()
	for range b.N {
		for _, obj := range schema.Objects() {
			_ = obj.Name()
			for _, f := range obj.Fields() {
				_ = f.Name()
				_ = f.Type()
			}
		}
		for _, e := range schema.Enums() {
			_ = e.Name()
			for _, v := range e.Values() {
				_ = v.Name()
				_ = v.Value()
			}
		}
	}
}

// ── helpers ───────────────────────────────────────────────────────────────

// benchLoadBfbs reads the binary schema file for benchmarks.  It calls
// b.Fatal if the file cannot be read.
func benchLoadBfbs(b *testing.B) []byte {
	b.Helper()
	path := filepath.Join("..", "tests", "monster_test.bfbs")
	data, err := os.ReadFile(path)
	if err != nil {
		b.Skipf("monster_test.bfbs not found: %v", err)
	}
	return data
}

// benchLoadSchema loads and parses the binary schema for benchmarks.
func benchLoadSchema(b *testing.B) *ReflectionSchema {
	b.Helper()
	bfbs := benchLoadBfbs(b)
	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		b.Fatalf("LoadReflectionSchema: %v", err)
	}
	return schema
}

// benchLoadData loads the binary Go wire data buffer for benchmarks.
func benchLoadData(b *testing.B) []byte {
	b.Helper()
	path := filepath.Join("..", "tests", "monsterdata_go_wire.mon")
	data, err := os.ReadFile(path)
	if err != nil {
		b.Skipf("monsterdata_go_wire.mon not found: %v", err)
	}
	return data
}
