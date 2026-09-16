package flatbuffers

// reflect_fuzz_test.go — fuzz tests for the FlatBuffers reflection runtime.
//
// These fuzz tests verify that the reflection runtime never panics on
// arbitrary input: it must always return an error rather than crashing,
// regardless of how malformed the input is.
//
// Two fuzz targets are provided:
//   - FuzzLoadReflectionSchema: fuzz the raw .bfbs byte slice passed to
//     LoadReflectionSchema.  The seed corpus is the real monster_test.bfbs.
//   - FuzzGetFieldString: fuzz the live data buffer passed to GetFieldString.
//     The schema is held fixed (monster_test.bfbs) so we test only the data
//     decoding path.
//
// Run with: go test -fuzz=FuzzLoadReflectionSchema -fuzztime=30s

import (
	"os"
	"path/filepath"
	"testing"
)

// FuzzLoadReflectionSchema fuzzes the raw bytes passed to LoadReflectionSchema.
// The schema parser must never panic — it must either succeed or return an error.
// Seeded with the real binary schema so the fuzzer starts from a valid baseline
// and explores mutations from there.
func FuzzLoadReflectionSchema(f *testing.F) {
	// Seed with the real .bfbs file so the fuzzer starts from a valid corpus.
	bfbsPath := filepath.Join("..", "tests", "monster_test.bfbs")
	bfbs, err := os.ReadFile(bfbsPath)
	if err != nil {
		f.Skipf("monster_test.bfbs not found: %v", err)
	}
	f.Add(bfbs)

	// Add a few short degenerate seeds to help the fuzzer explore error paths.
	f.Add([]byte{})
	f.Add([]byte{0x00})
	f.Add([]byte{0x00, 0x00, 0x00, 0x00})
	f.Add([]byte{0xff, 0xff, 0xff, 0xff})

	f.Fuzz(func(t *testing.T, data []byte) {
		// Must not panic — only succeed or return an error.
		schema, err := LoadReflectionSchema(data)
		if err != nil {
			// An error is fine for malformed input.
			return
		}
		// If the parse succeeded, basic schema operations also must not panic.
		_ = schema.Objects()
		_ = schema.Enums()
		_ = schema.RootTable()
	})
}

// FuzzGetFieldString fuzzes the live data buffer passed to GetFieldString.
// The schema is fixed (monster_test.bfbs) and the "name" field descriptor is
// resolved once during setup.  Only the data bytes are fuzzed.  The function
// must never panic; errors are acceptable.
func FuzzGetFieldString(f *testing.F) {
	bfbsPath := filepath.Join("..", "tests", "monster_test.bfbs")
	bfbs, err := os.ReadFile(bfbsPath)
	if err != nil {
		f.Skipf("monster_test.bfbs not found: %v", err)
	}

	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		f.Fatalf("LoadReflectionSchema: %v", err)
	}

	obj := schema.ObjectByName("MyGame.Example.Monster")
	if obj == nil {
		f.Fatal("Monster not found in schema")
	}

	nameField := obj.FieldByName("name")
	if nameField == nil {
		f.Fatal("name field not found in Monster")
	}

	// Seed with the real binary wire data buffer.
	wirePath := filepath.Join("..", "tests", "monsterdata_go_wire.mon")
	golden, err := os.ReadFile(wirePath)
	if err != nil {
		f.Skipf("monsterdata_go_wire.mon not found: %v", err)
	}
	f.Add(golden)

	// Short degenerate seeds to exercise error paths.
	f.Add([]byte{})
	f.Add([]byte{0x00, 0x00, 0x00, 0x00})
	f.Add([]byte{0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00})

	f.Fuzz(func(t *testing.T, data []byte) {
		if len(data) < SizeUOffsetT {
			return
		}
		rootPos := int(GetUOffsetT(data[0:]))
		// Must not panic — errors are fine.
		_, _ = GetFieldString(data, rootPos, nameField)
	})
}

// FuzzGetFieldInt fuzzes the live data buffer passed to GetFieldInt using the
// hp (short) field as the fixed field descriptor.  The function must never
// panic regardless of the data contents.
func FuzzGetFieldInt(f *testing.F) {
	bfbsPath := filepath.Join("..", "tests", "monster_test.bfbs")
	bfbs, err := os.ReadFile(bfbsPath)
	if err != nil {
		f.Skipf("monster_test.bfbs not found: %v", err)
	}

	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		f.Fatalf("LoadReflectionSchema: %v", err)
	}

	obj := schema.ObjectByName("MyGame.Example.Monster")
	if obj == nil {
		f.Fatal("Monster not found in schema")
	}

	hpField := obj.FieldByName("hp")
	if hpField == nil {
		f.Fatal("hp field not found in Monster")
	}

	wirePath := filepath.Join("..", "tests", "monsterdata_go_wire.mon")
	golden, err := os.ReadFile(wirePath)
	if err != nil {
		f.Skipf("monsterdata_go_wire.mon not found: %v", err)
	}
	f.Add(golden)
	f.Add([]byte{0x00, 0x00, 0x00, 0x00})

	f.Fuzz(func(t *testing.T, data []byte) {
		if len(data) < SizeUOffsetT {
			return
		}
		rootPos := int(GetUOffsetT(data[0:]))
		_, _ = GetFieldInt(data, rootPos, hpField)
	})
}
