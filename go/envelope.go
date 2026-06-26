// Package flatbuffers provides encoding and decoding of FlatBuffers, along with
// utilities including self-describing envelope support.
package flatbuffers

import (
	"crypto/sha256"
	"encoding/binary"
	"errors"
	"fmt"
)

// Envelope format (42-byte header + payload):
//
//	[4 bytes]  magic: "LN2E" (0x4C 0x4E 0x32 0x45)
//	[1 byte]   version: 0x01
//	[1 byte]   hash algorithm: 0x01 = SHA-256
//	[32 bytes] SHA-256 hash of the .bfbs binary schema
//	[4 bytes]  payload length, little-endian uint32
//	[N bytes]  FlatBuffer payload

// envelopeMagic is the 4-byte magic that identifies an LN2E envelope.
const (
	envelopeMagicByte0     = 'L'
	envelopeMagicByte1     = 'N'
	envelopeMagicByte2     = '2'
	envelopeMagicByte3     = 'E'
	envelopeVersion        = 0x01
	envelopeHashAlgoSHA256 = 0x01
	// envelopeHeaderSize is the fixed size of the envelope header in bytes.
	envelopeHeaderSize = 42
)

var (
	errEnvelopeTooShort     = errors.New("envelope: data too short to contain header")
	errEnvelopeBadMagic     = errors.New("envelope: invalid magic bytes (expected 'LN2E')")
	errEnvelopeBadVersion   = errors.New("envelope: unsupported version")
	errEnvelopeBadHashAlgo  = errors.New("envelope: unsupported hash algorithm")
	errEnvelopeTruncated    = errors.New("envelope: payload length exceeds available data")
	errEnvelopeHashMismatch = errors.New("envelope: schema hash mismatch")
)

// ComputeSchemaHash returns the SHA-256 hash of binary schema data (.bfbs file contents).
func ComputeSchemaHash(bfbsData []byte) [32]byte {
	return sha256.Sum256(bfbsData)
}

// EnvelopeWrap wraps a FlatBuffer payload with a self-describing envelope that
// embeds the SHA-256 hash of the associated .bfbs binary schema. The resulting
// byte slice can be unwrapped with EnvelopeUnwrap after verifying the schema hash.
func EnvelopeWrap(schemaHash [32]byte, payload []byte) []byte {
	buf := make([]byte, envelopeHeaderSize+len(payload))

	// magic
	buf[0] = envelopeMagicByte0
	buf[1] = envelopeMagicByte1
	buf[2] = envelopeMagicByte2
	buf[3] = envelopeMagicByte3

	// version
	buf[4] = envelopeVersion

	// hash algorithm
	buf[5] = envelopeHashAlgoSHA256

	// schema hash
	copy(buf[6:38], schemaHash[:])

	// payload length — len(payload) fits in uint32 for any realistic message
	binary.LittleEndian.PutUint32(buf[38:42], uint32(len(payload))) //nolint:gosec

	// payload
	copy(buf[42:], payload)

	return buf
}

// EnvelopeUnwrap validates and extracts the FlatBuffer payload from an envelope.
// It checks the magic bytes, version, hash algorithm, and that the embedded schema
// hash matches expectedHash. Returns the payload slice (a sub-slice of data) or an
// error if any check fails.
func EnvelopeUnwrap(data []byte, expectedHash [32]byte) ([]byte, error) {
	payload, hash, err := envelopeParse(data)
	if err != nil {
		return nil, err
	}

	if hash != expectedHash {
		return nil, fmt.Errorf("%w: got %x, want %x", errEnvelopeHashMismatch, hash, expectedHash)
	}

	return payload, nil
}

// EnvelopePayload extracts the FlatBuffer payload from an envelope without
// validating the schema hash. Useful for inspection or routing before the
// expected schema is known. Returns an error if the header is malformed.
func EnvelopePayload(data []byte) ([]byte, error) {
	payload, _, err := envelopeParse(data)

	return payload, err
}

// EnvelopeSchemaHash extracts the 32-byte schema hash from an envelope header
// without validating the payload or comparing against an expected hash. Useful
// for routing messages to the correct handler by schema identity.
func EnvelopeSchemaHash(data []byte) ([32]byte, error) {
	_, hash, err := envelopeParse(data)

	return hash, err
}

// IsEnvelope returns true if data begins with the LN2E magic bytes and is at
// least long enough to contain a complete envelope header.
func IsEnvelope(data []byte) bool {
	if len(data) < envelopeHeaderSize {
		return false
	}

	return data[0] == envelopeMagicByte0 &&
		data[1] == envelopeMagicByte1 &&
		data[2] == envelopeMagicByte2 &&
		data[3] == envelopeMagicByte3
}

// envelopeParse validates the envelope header and returns (payload, schemaHash, error).
func envelopeParse(data []byte) ([]byte, [32]byte, error) {
	var zero [32]byte

	if len(data) < envelopeHeaderSize {
		return nil, zero, errEnvelopeTooShort
	}

	if data[0] != envelopeMagicByte0 || data[1] != envelopeMagicByte1 ||
		data[2] != envelopeMagicByte2 || data[3] != envelopeMagicByte3 {
		return nil, zero, errEnvelopeBadMagic
	}

	if data[4] != envelopeVersion {
		return nil, zero, fmt.Errorf("%w: %d", errEnvelopeBadVersion, data[4])
	}

	if data[5] != envelopeHashAlgoSHA256 {
		return nil, zero, fmt.Errorf("%w: %d", errEnvelopeBadHashAlgo, data[5])
	}

	var hash [32]byte

	copy(hash[:], data[6:38])

	payloadLen := binary.LittleEndian.Uint32(data[38:42])
	end := envelopeHeaderSize + int(payloadLen)

	if end > len(data) {
		return nil, zero, fmt.Errorf("%w: need %d bytes, have %d", errEnvelopeTruncated, end, len(data))
	}

	return data[envelopeHeaderSize:end], hash, nil
}
