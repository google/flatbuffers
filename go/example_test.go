package flatbuffers

import (
	"errors"
	"fmt"
	"os"
	"path/filepath"
)

// ExampleNewVerifier demonstrates creating a Verifier with custom options and
// performing a basic range check on a raw buffer.
func ExampleNewVerifier() {
	buf := make([]byte, 64)
	opts := &VerifierOptions{
		MaxDepth:        32,
		MaxTables:       100,
		MaxApparentSize: 1024,
	}
	v := NewVerifier(buf, opts)

	// CheckRange verifies a half-open range [pos, pos+size) lies within buf.
	if err := v.CheckRange(0, 64); err != nil {
		fmt.Println("unexpected error:", err)
		return
	}
	fmt.Println("ok")
	// Output: ok
}

// ExampleVerifier_CheckTable demonstrates verifying a hand-crafted table.
//
// The buffer layout mirrors the minimal valid FlatBuffer used in the
// package tests:
//
//	bytes 0–3:   root UOffsetT = 8 (table starts at byte 8)
//	bytes 4–7:   vtable — size=4, table_size=4 (two VOffsetT fields)
//	bytes 8–11:  table SOffsetT = 4 (vtable is 4 bytes before the table)
func ExampleVerifier_CheckTable() {
	buf := make([]byte, 16)
	// Root offset: table is at byte 8.
	buf[0] = 8
	// Vtable at byte 4: size=4, table_size=4 (little-endian uint16 pairs).
	buf[4] = 4
	buf[5] = 0
	buf[6] = 4
	buf[7] = 0
	// Table at byte 8: SOffsetT = 4 (points back to vtable at byte 4).
	buf[8] = 4
	buf[9] = 0
	buf[10] = 0
	buf[11] = 0

	v := NewVerifier(buf, nil)
	if err := v.CheckTable(8); err != nil {
		fmt.Println("unexpected error:", err)
		return
	}
	fmt.Println("ok")
	// Output: ok
}

// ExampleVerificationError demonstrates constructing a VerificationError and
// using errors.As to extract structured fields from a returned error.
//
// In production code the error originates from Verifier methods or generated
// VerifyRootAs* functions; this example constructs one directly to show the
// error message format and the errors.As pattern.
func ExampleVerificationError() {
	// Simulate a verification failure: range out of bounds at offset 42.
	var err error = &VerificationError{
		Kind:   ErrRangeOutOfBounds,
		Offset: 42,
	}

	// errors.As extracts the concrete *VerificationError from the error chain.
	var ve *VerificationError
	if errors.As(err, &ve) {
		fmt.Printf("kind=%d field=%q offset=%d\n", ve.Kind, ve.Field, ve.Offset)
	}

	// The Error() method produces a human-readable message.
	fmt.Println(err.Error())
	// Output:
	// kind=1 field="" offset=42
	// range out of bounds at offset 42
}

// ExampleVerifier_CheckAlignment demonstrates the alignment check used when
// reading naturally-aligned scalars (e.g., a uint32 must start at a
// multiple-of-4 byte offset).
func ExampleVerifier_CheckAlignment() {
	buf := make([]byte, 32)
	v := NewVerifier(buf, nil)

	// Position 8 is 4-byte aligned — no error.
	if err := v.CheckAlignment(8, 4); err != nil {
		fmt.Println("unexpected:", err)
		return
	}
	fmt.Println("aligned: ok")

	// Position 3 is not 4-byte aligned.
	if err := v.CheckAlignment(3, 4); err != nil {
		var ve *VerificationError
		if errors.As(err, &ve) && ve.Kind == ErrUnaligned {
			fmt.Println("unaligned: ErrUnaligned")
		}
	}
	// Output:
	// aligned: ok
	// unaligned: ErrUnaligned
}

// ExampleVerifier_PushDepth demonstrates how the depth counter enforces a
// nesting limit when verifying deeply nested tables or vectors-of-tables.
func ExampleVerifier_PushDepth() {
	buf := make([]byte, 64)
	opts := &VerifierOptions{MaxDepth: 2}
	v := NewVerifier(buf, opts)

	for range 2 {
		if err := v.PushDepth(); err != nil {
			fmt.Println("unexpected early error:", err)
			return
		}
	}

	// The third push exceeds MaxDepth=2.
	if err := v.PushDepth(); err != nil {
		var ve *VerificationError
		if errors.As(err, &ve) {
			fmt.Printf("depth exceeded: %s\n", ve.Error())
		}
	}
	// Output: depth exceeded: nested table depth limit reached at offset 0
}

// ExampleLoadReflectionSchema demonstrates loading a compiled binary schema
// (.bfbs) and accessing top-level metadata.
//
// This example reads monster_test.bfbs produced by the flatc compiler.  If
// the file is not present (e.g., outside the repository tree), the example
// exits without printing.
func ExampleLoadReflectionSchema() {
	bfbs, err := os.ReadFile(filepath.Join("..", "tests", "monster_test.bfbs"))
	if err != nil {
		// File absent — skip silently so go test does not fail outside the repo.
		return
	}

	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		fmt.Println("error:", err)
		return
	}

	root := schema.RootTable()
	if root == nil {
		fmt.Println("no root table")
		return
	}
	fmt.Println(root.Name())
	// Output: MyGame.Example.Monster
}

// ExampleGetFieldString demonstrates reading a UTF-8 string field from a live
// data buffer using reflection metadata derived from a compiled schema.
//
// The workflow is:
//  1. Load the .bfbs schema once at startup via [LoadReflectionSchema].
//  2. Resolve the table and field descriptors once and cache them.
//  3. For each incoming data buffer call [GetFieldString] with the cached field.
//
// This example reads the "name" field from monsterdata_go_wire.mon, a binary
// FlatBuffer encoding a Monster table with name="MyMonster".
func ExampleGetFieldString() {
	bfbs, err := os.ReadFile(filepath.Join("..", "tests", "monster_test.bfbs"))
	if err != nil {
		return
	}
	data, err := os.ReadFile(filepath.Join("..", "tests", "monsterdata_go_wire.mon"))
	if err != nil {
		return
	}

	schema, err := LoadReflectionSchema(bfbs)
	if err != nil {
		fmt.Println("schema error:", err)
		return
	}

	obj := schema.ObjectByName("MyGame.Example.Monster")
	if obj == nil {
		fmt.Println("table not found")
		return
	}

	field := obj.FieldByName("name")
	if field == nil {
		fmt.Println("field not found")
		return
	}

	// Locate the root table: the first 4 bytes of a FlatBuffer are a
	// UOffsetT pointing to the root table relative to byte 0.
	rootPos := int(GetUOffsetT(data))
	name, err := GetFieldString(data, rootPos, field)
	if err != nil {
		fmt.Println("read error:", err)
		return
	}
	fmt.Println(name)
	// Output: MyMonster
}
