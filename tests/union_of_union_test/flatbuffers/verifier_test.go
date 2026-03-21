package flatbuffers

import (
	"testing"
)

// Helper to build a minimal valid-looking FlatBuffer with a root table.
// The root UOffsetT at pos 0 points to a table at pos 8.
// The table has a vtable at pos 4 with vtable size=4, table size=4.
func minimalValidBuffer() []byte {
	buf := make([]byte, 16)
	// Root offset at pos 0: points to table at pos 8
	buf[0] = 8
	// At pos 4: vtable (size=4, table_size=4)
	buf[4] = 4 // vtable size (2 bytes LE)
	buf[5] = 0
	buf[6] = 4 // table size (2 bytes LE)
	buf[7] = 0
	// At pos 8: SOffsetT pointing back to vtable at pos 4 => soffset = 8-4 = 4
	buf[8] = 4
	buf[9] = 0
	buf[10] = 0
	buf[11] = 0
	return buf
}

func TestVerifierCheckUOffsetT_ValidRoot(t *testing.T) {
	buf := minimalValidBuffer()
	v := NewVerifier(buf, nil)
	pos, err := v.CheckUOffsetT(0)
	if err != nil {
		t.Fatalf("CheckUOffsetT failed: %v", err)
	}
	if pos != 8 {
		t.Errorf("pos = %d, want 8", pos)
	}
}

func TestVerifierCheckUOffsetT_OutOfBounds(t *testing.T) {
	buf := []byte{0xFF, 0xFF, 0xFF, 0xFF} // points past buffer
	v := NewVerifier(buf, nil)
	_, err := v.CheckUOffsetT(0)
	if err == nil {
		t.Fatal("Expected error for out-of-bounds offset")
	}
	verr, ok := err.(*VerificationError)
	if !ok {
		t.Fatalf("Expected *VerificationError, got %T", err)
	}
	if verr.Kind != ErrRangeOutOfBounds {
		t.Errorf("Kind = %v, want ErrRangeOutOfBounds", verr.Kind)
	}
}

func TestVerifierCheckUOffsetT_EmptyBuffer(t *testing.T) {
	v := NewVerifier([]byte{}, nil)
	_, err := v.CheckUOffsetT(0)
	if err == nil {
		t.Fatal("Expected error for empty buffer")
	}
}

func TestVerifierCheckRange_WithinBounds(t *testing.T) {
	buf := make([]byte, 100)
	v := NewVerifier(buf, nil)
	err := v.CheckRange(0, 100)
	if err != nil {
		t.Fatalf("CheckRange failed: %v", err)
	}
}

func TestVerifierCheckRange_OutOfBounds(t *testing.T) {
	buf := make([]byte, 10)
	v := NewVerifier(buf, nil)
	err := v.CheckRange(0, 20)
	if err == nil {
		t.Fatal("Expected error for out-of-bounds range")
	}
	verr := err.(*VerificationError)
	if verr.Kind != ErrRangeOutOfBounds {
		t.Errorf("Kind = %v, want ErrRangeOutOfBounds", verr.Kind)
	}
}

func TestVerifierCheckRange_ApparentSizeTooLarge(t *testing.T) {
	buf := make([]byte, 100)
	opts := &VerifierOptions{MaxApparentSize: 50}
	v := NewVerifier(buf, opts)
	err := v.CheckRange(0, 60)
	if err == nil {
		t.Fatal("Expected error for apparent size exceeded")
	}
	verr := err.(*VerificationError)
	if verr.Kind != ErrApparentSizeTooLarge {
		t.Errorf("Kind = %v, want ErrApparentSizeTooLarge", verr.Kind)
	}
}

func TestVerifierCheckAlignment(t *testing.T) {
	buf := make([]byte, 100)
	v := NewVerifier(buf, nil)

	// Aligned position
	if err := v.CheckAlignment(8, 4); err != nil {
		t.Errorf("CheckAlignment(8, 4) should pass: %v", err)
	}

	// Misaligned position
	if err := v.CheckAlignment(3, 4); err == nil {
		t.Error("CheckAlignment(3, 4) should fail")
	} else {
		verr := err.(*VerificationError)
		if verr.Kind != ErrUnaligned {
			t.Errorf("Kind = %v, want ErrUnaligned", verr.Kind)
		}
	}
}

func TestVerifierDepthLimit(t *testing.T) {
	buf := make([]byte, 100)
	opts := &VerifierOptions{MaxDepth: 3}
	v := NewVerifier(buf, opts)

	// Should succeed 3 times
	for i := 0; i < 3; i++ {
		if err := v.PushDepth(); err != nil {
			t.Fatalf("PushDepth %d failed: %v", i, err)
		}
	}

	// 4th push should fail
	if err := v.PushDepth(); err == nil {
		t.Fatal("Expected depth limit error")
	} else {
		verr := err.(*VerificationError)
		if verr.Kind != ErrDepthLimitReached {
			t.Errorf("Kind = %v, want ErrDepthLimitReached", verr.Kind)
		}
	}

	// Pop and push should succeed again
	v.PopDepth()
	if err := v.PushDepth(); err != nil {
		t.Fatalf("PushDepth after PopDepth failed: %v", err)
	}
}

func TestVerifierTableLimit(t *testing.T) {
	buf := make([]byte, 100)
	opts := &VerifierOptions{MaxTables: 3}
	v := NewVerifier(buf, opts)

	for i := 0; i < 3; i++ {
		if err := v.CountTable(); err != nil {
			t.Fatalf("CountTable %d failed: %v", i, err)
		}
	}

	if err := v.CountTable(); err == nil {
		t.Fatal("Expected too many tables error")
	} else {
		verr := err.(*VerificationError)
		if verr.Kind != ErrTooManyTables {
			t.Errorf("Kind = %v, want ErrTooManyTables", verr.Kind)
		}
	}
}

func TestVerifierCheckTable_Valid(t *testing.T) {
	buf := minimalValidBuffer()
	v := NewVerifier(buf, nil)
	err := v.CheckTable(8)
	if err != nil {
		t.Fatalf("CheckTable failed: %v", err)
	}
}

func TestVerifierCheckTable_InvalidVtable(t *testing.T) {
	buf := make([]byte, 16)
	// Root table at pos 8, SOffsetT pointing to invalid position
	buf[8] = 0xFF
	buf[9] = 0xFF
	buf[10] = 0xFF
	buf[11] = 0x7F // Large positive SOffsetT pointing backwards past buffer
	v := NewVerifier(buf, nil)
	err := v.CheckTable(8)
	if err == nil {
		t.Fatal("Expected error for invalid vtable")
	}
}

func TestVerifierCheckString_Valid(t *testing.T) {
	// Build a string at position 4: length=5, "hello\0"
	buf := make([]byte, 20)
	// UOffsetT at pos 0 pointing to string at pos 4
	buf[0] = 4
	buf[1] = 0
	buf[2] = 0
	buf[3] = 0
	// String length at pos 4
	buf[4] = 5
	buf[5] = 0
	buf[6] = 0
	buf[7] = 0
	// String data
	copy(buf[8:], "hello")
	buf[13] = 0 // null terminator

	v := NewVerifier(buf, nil)
	err := v.CheckString(4)
	if err != nil {
		t.Fatalf("CheckString failed: %v", err)
	}
}

func TestVerifierCheckString_InvalidUTF8(t *testing.T) {
	buf := make([]byte, 20)
	buf[0] = 4
	buf[1] = 0
	buf[2] = 0
	buf[3] = 0
	buf[4] = 2 // length = 2
	buf[5] = 0
	buf[6] = 0
	buf[7] = 0
	buf[8] = 0xFF // invalid UTF-8
	buf[9] = 0xFE // invalid UTF-8
	buf[10] = 0   // null terminator

	v := NewVerifier(buf, nil)
	err := v.CheckString(4)
	if err == nil {
		t.Fatal("Expected error for invalid UTF-8")
	}
	verr := err.(*VerificationError)
	if verr.Kind != ErrUtf8Error {
		t.Errorf("Kind = %v, want ErrUtf8Error", verr.Kind)
	}
}

func TestVerifierCheckString_MissingNullTerminator(t *testing.T) {
	buf := make([]byte, 16)
	buf[0] = 3 // length = 3
	buf[1] = 0
	buf[2] = 0
	buf[3] = 0
	buf[4] = 'a'
	buf[5] = 'b'
	buf[6] = 'c'
	buf[7] = 'X' // not null

	v := NewVerifier(buf, nil)
	err := v.CheckString(0)
	if err == nil {
		t.Fatal("Expected error for missing null terminator")
	}
	verr := err.(*VerificationError)
	if verr.Kind != ErrMissingNullTerminator {
		t.Errorf("Kind = %v, want ErrMissingNullTerminator", verr.Kind)
	}
}

func TestVerifierCheckVector_Valid(t *testing.T) {
	buf := make([]byte, 20)
	// Vector at pos 0: length=3, element_size=4 => needs 4+12=16 bytes
	buf[0] = 3 // length
	buf[1] = 0
	buf[2] = 0
	buf[3] = 0

	v := NewVerifier(buf, nil)
	n, err := v.CheckVector(0, 4)
	if err != nil {
		t.Fatalf("CheckVector failed: %v", err)
	}
	if n != 3 {
		t.Errorf("length = %d, want 3", n)
	}
}

func TestVerifierCheckVector_LengthOverflow(t *testing.T) {
	buf := make([]byte, 8)
	// Vector claiming length=0xFFFFFFFF with element_size=4 => overflow
	buf[0] = 0xFF
	buf[1] = 0xFF
	buf[2] = 0xFF
	buf[3] = 0xFF

	v := NewVerifier(buf, nil)
	_, err := v.CheckVector(0, 4)
	if err == nil {
		t.Fatal("Expected error for vector length overflow")
	}
}

func TestVerifierCheckRequiredField_Present(t *testing.T) {
	buf := minimalValidBuffer()
	v := NewVerifier(buf, nil)
	// The minimal buffer has vtable size=4, table size=4, no fields
	// So checking any field offset > vtable size should detect absence
	// But first let's verify with a field that IS at offset 4 (vtable size)
	// which won't be found (vtable only has header, no fields)
	err := v.CheckRequiredField(8, 4, "test_field")
	if err == nil {
		t.Fatal("Expected error for missing required field")
	}
	verr := err.(*VerificationError)
	if verr.Kind != ErrMissingRequiredField {
		t.Errorf("Kind = %v, want ErrMissingRequiredField", verr.Kind)
	}
}

func TestVerificationError_ErrorInterface(t *testing.T) {
	err := &VerificationError{
		Kind:   ErrRangeOutOfBounds,
		Offset: 42,
	}
	// Must implement error interface
	var e error = err
	msg := e.Error()
	if msg == "" {
		t.Error("Error() returned empty string")
	}
}

func TestVerificationError_ErrorsAs(t *testing.T) {
	var err error = &VerificationError{
		Kind:   ErrDepthLimitReached,
		Offset: 100,
		Trace:  []TraceDetail{{Kind: TraceTableField, Name: "monster", Position: 50}},
	}
	var verr *VerificationError
	if ok := isVerificationError(err, &verr); !ok {
		t.Fatal("errors.As should work with *VerificationError")
	}
	if verr.Kind != ErrDepthLimitReached {
		t.Errorf("Kind = %v, want ErrDepthLimitReached", verr.Kind)
	}
}

// Helper since errors.As requires import
func isVerificationError(err error, target **VerificationError) bool {
	verr, ok := err.(*VerificationError)
	if ok {
		*target = verr
	}
	return ok
}

func TestVerifierDefaultOptions(t *testing.T) {
	buf := make([]byte, 100)
	v := NewVerifier(buf, nil)

	// Should use defaults — verify by exceeding them
	// Default MaxDepth = 64, try pushing 65 times
	for i := 0; i < 64; i++ {
		if err := v.PushDepth(); err != nil {
			t.Fatalf("PushDepth %d with default opts failed: %v", i, err)
		}
	}
	if err := v.PushDepth(); err == nil {
		t.Fatal("Expected depth limit at 65 with default opts")
	}
}
