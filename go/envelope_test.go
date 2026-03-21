package flatbuffers //nolint:testpackage // internal tests consistent with existing test files in this package

import (
	"bytes"
	"testing"
)

func TestEnvelopeRoundtrip(t *testing.T) {
	t.Parallel()

	schema := []byte("fake schema binary")
	payload := []byte("hello flatbuffer payload")
	hash := ComputeSchemaHash(schema)

	wrapped := EnvelopeWrap(hash, payload)

	got, err := EnvelopeUnwrap(wrapped, hash)
	if err != nil {
		t.Fatalf("EnvelopeUnwrap: unexpected error: %v", err)
	}

	if !bytes.Equal(got, payload) {
		t.Fatalf("payload mismatch: got %q, want %q", got, payload)
	}
}

func TestEnvelopeEmptyPayload(t *testing.T) {
	t.Parallel()

	hash := ComputeSchemaHash([]byte("schema"))
	wrapped := EnvelopeWrap(hash, []byte{})

	got, err := EnvelopeUnwrap(wrapped, hash)
	if err != nil {
		t.Fatalf("unexpected error: %v", err)
	}

	if len(got) != 0 {
		t.Fatalf("expected empty payload, got %d bytes", len(got))
	}
}

func TestEnvelopeWrongHash(t *testing.T) {
	t.Parallel()

	hash := ComputeSchemaHash([]byte("schema A"))
	wrongHash := ComputeSchemaHash([]byte("schema B"))
	wrapped := EnvelopeWrap(hash, []byte("data"))

	_, err := EnvelopeUnwrap(wrapped, wrongHash)
	if err == nil {
		t.Fatal("expected error for wrong hash, got nil")
	}
}

func TestEnvelopeShortBuffer(t *testing.T) {
	t.Parallel()

	cases := [][]byte{
		nil,
		{},
		{'L', 'N', '2', 'E'},
		make([]byte, 41),
	}

	for _, data := range cases {
		_, err := EnvelopePayload(data)
		if err == nil {
			t.Errorf("expected error for short buffer of len %d", len(data))
		}
	}
}

func TestEnvelopeBadMagic(t *testing.T) {
	t.Parallel()

	hash := ComputeSchemaHash([]byte("schema"))
	wrapped := EnvelopeWrap(hash, []byte("payload"))
	wrapped[0] = 'X' // corrupt magic

	_, err := EnvelopeUnwrap(wrapped, hash)
	if err == nil {
		t.Fatal("expected error for bad magic")
	}
}

func TestEnvelopeTruncatedPayload(t *testing.T) {
	t.Parallel()

	hash := ComputeSchemaHash([]byte("schema"))
	wrapped := EnvelopeWrap(hash, []byte("full payload"))

	// truncate data after header
	truncated := wrapped[:envelopeHeaderSize+2]

	_, err := EnvelopePayload(truncated)
	if err == nil {
		t.Fatal("expected error for truncated payload")
	}
}

func TestIsEnvelope(t *testing.T) {
	t.Parallel()

	hash := ComputeSchemaHash([]byte("schema"))
	wrapped := EnvelopeWrap(hash, []byte("data"))

	if !IsEnvelope(wrapped) {
		t.Error("IsEnvelope returned false for valid envelope")
	}

	if IsEnvelope([]byte("not an envelope")) {
		t.Error("IsEnvelope returned true for non-envelope data")
	}

	if IsEnvelope([]byte("LN2")) { // too short
		t.Error("IsEnvelope returned true for too-short data")
	}
}

func TestEnvelopeSchemaHash(t *testing.T) {
	t.Parallel()

	schema := []byte("my schema")
	hash := ComputeSchemaHash(schema)
	wrapped := EnvelopeWrap(hash, []byte("payload"))

	extracted, err := EnvelopeSchemaHash(wrapped)
	if err != nil {
		t.Fatalf("EnvelopeSchemaHash: unexpected error: %v", err)
	}

	if extracted != hash {
		t.Fatalf("schema hash mismatch: got %x, want %x", extracted, hash)
	}
}

func TestEnvelopePayloadNoHashCheck(t *testing.T) {
	t.Parallel()

	hash := ComputeSchemaHash([]byte("schema"))
	payload := []byte("some payload data")
	wrapped := EnvelopeWrap(hash, payload)

	got, err := EnvelopePayload(wrapped)
	if err != nil {
		t.Fatalf("EnvelopePayload: unexpected error: %v", err)
	}

	if !bytes.Equal(got, payload) {
		t.Fatalf("payload mismatch: got %q, want %q", got, payload)
	}
}

func TestEnvelopeLargePayload(t *testing.T) {
	t.Parallel()

	hash := ComputeSchemaHash([]byte("schema"))

	payload := make([]byte, 1<<20) // 1 MiB
	for i := range payload {
		payload[i] = byte(i % 251)
	}

	wrapped := EnvelopeWrap(hash, payload)

	got, err := EnvelopeUnwrap(wrapped, hash)
	if err != nil {
		t.Fatalf("unexpected error: %v", err)
	}

	if !bytes.Equal(got, payload) {
		t.Fatal("large payload roundtrip failed")
	}
}
