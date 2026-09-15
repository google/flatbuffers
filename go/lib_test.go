package flatbuffers

import "testing"

// TestGetRootAsShortBuffer verifies that GetRootAs does not panic when
// given a buffer shorter than SizeUOffsetT (4 bytes).
func TestGetRootAsShortBuffer(t *testing.T) {
	shortBuffers := [][]byte{
		nil,
		{},
		{0x01},
		{0x01, 0x02},
		{0x01, 0x02, 0x03},
	}
	for _, buf := range shortBuffers {
		// Must not panic
		tab := &Table{}
		GetRootAs(buf, 0, tab)
	}
}

// TestGetSizePrefixedRootAsShortBuffer verifies that GetSizePrefixedRootAs
// does not panic when given a buffer shorter than the required minimum.
func TestGetSizePrefixedRootAsShortBuffer(t *testing.T) {
	shortBuffers := [][]byte{
		nil,
		{},
		{0x01, 0x02, 0x03},
		{0x01, 0x02, 0x03, 0x04},       // 4 bytes: only size prefix, no root offset
		{0x01, 0x02, 0x03, 0x04, 0x05},  // 5 bytes: still too short for prefix + UOffsetT
		{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07}, // 7 bytes: still too short
	}
	for _, buf := range shortBuffers {
		// Must not panic
		tab := &Table{}
		GetSizePrefixedRootAs(buf, 0, tab)
	}
}

// TestGetSizePrefixShortBuffer verifies GetSizePrefix doesn't panic on
// buffers shorter than 4 bytes.
func TestGetSizePrefixShortBuffer(t *testing.T) {
	shortBuffers := [][]byte{
		nil,
		{},
		{0x01},
		{0x01, 0x02, 0x03},
	}
	for _, buf := range shortBuffers {
		result := GetSizePrefix(buf, 0)
		if result != 0 {
			t.Errorf("GetSizePrefix on short buffer should return 0, got %d", result)
		}
	}
}

// TestGetIndirectOffsetShortBuffer verifies GetIndirectOffset doesn't panic
// on buffers shorter than 4 bytes.
func TestGetIndirectOffsetShortBuffer(t *testing.T) {
	shortBuffers := [][]byte{
		nil,
		{},
		{0x01, 0x02, 0x03},
	}
	for _, buf := range shortBuffers {
		result := GetIndirectOffset(buf, 0)
		if result != 0 {
			t.Errorf("GetIndirectOffset on short buffer should return 0, got %d", result)
		}
	}
}

// TestGetBufferIdentifierShortBuffer verifies GetBufferIdentifier doesn't
// panic on buffers shorter than SizeUOffsetT + fileIdentifierLength.
func TestGetBufferIdentifierShortBuffer(t *testing.T) {
	shortBuffers := [][]byte{
		nil,
		{},
		{0x01, 0x02, 0x03},
		{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07}, // 7 bytes: need at least 8
	}
	for _, buf := range shortBuffers {
		result := GetBufferIdentifier(buf)
		if result != "" {
			t.Errorf("GetBufferIdentifier on short buffer should return empty, got %q", result)
		}
	}
}

// TestGetRootAsValidBuffer ensures that GetRootAs still works correctly
// for valid buffers.
func TestGetRootAsValidBuffer(t *testing.T) {
	// Create a minimal valid buffer: a 4-byte UOffsetT pointing to offset 4
	buf := []byte{0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
	tab := &Table{}
	GetRootAs(buf, 0, tab)
	if tab.Pos != 4 {
		t.Errorf("Expected tab.Pos to be 4, got %d", tab.Pos)
	}
}

// TestGetRootAsWithOffset verifies bounds checking works with non-zero offset.
func TestGetRootAsWithOffset(t *testing.T) {
	// Buffer with offset=2 needs at least 6 bytes (offset + SizeUOffsetT)
	buf := []byte{0x00, 0x00, 0x04, 0x00, 0x00, 0x00}
	tab := &Table{}
	GetRootAs(buf, 2, tab)
	if tab.Pos != 6 {
		t.Errorf("Expected tab.Pos to be 6, got %d", tab.Pos)
	}

	// Same buffer but with offset=3 should silently fail (only 3 bytes remain)
	tab2 := &Table{}
	GetRootAs(buf, 3, tab2)
	if tab2.Pos != 0 {
		t.Errorf("Expected tab.Pos to be 0 for short buffer, got %d", tab2.Pos)
	}
}
