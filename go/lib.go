package flatbuffers

import "errors"

// ErrBufferTooShort is returned when the buffer is too short to read the
// root table offset (UOffsetT).
var ErrBufferTooShort = errors.New("flatbuffers: buffer too short")

// FlatBuffer is the interface that represents a flatbuffer.
type FlatBuffer interface {
	Table() Table
	Init(buf []byte, i UOffsetT)
}

// GetRootAs is a generic helper to initialize a FlatBuffer with the provided buffer bytes and its data offset.
func GetRootAs(buf []byte, offset UOffsetT, fb FlatBuffer) {
	if int(offset)+SizeUOffsetT > len(buf) {
		return
	}
	n := GetUOffsetT(buf[offset:])
	fb.Init(buf, n+offset)
}

// GetSizePrefixedRootAs is a generic helper to initialize a FlatBuffer with the provided size-prefixed buffer
// bytes and its data offset
func GetSizePrefixedRootAs(buf []byte, offset UOffsetT, fb FlatBuffer) {
	if int(offset)+sizePrefixLength+SizeUOffsetT > len(buf) {
		return
	}
	n := GetUOffsetT(buf[offset+sizePrefixLength:])
	fb.Init(buf, n+offset+sizePrefixLength)
}

// GetSizePrefix reads the size from a size-prefixed flatbuffer
func GetSizePrefix(buf []byte, offset UOffsetT) uint32 {
	if int(offset)+SizeUOffsetT > len(buf) {
		return 0
	}
	return GetUint32(buf[offset:])
}

// GetIndirectOffset retrives the relative offset in the provided buffer stored at `offset`.
func GetIndirectOffset(buf []byte, offset UOffsetT) UOffsetT {
	if int(offset)+SizeUOffsetT > len(buf) {
		return 0
	}
	return offset + GetUOffsetT(buf[offset:])
}

// GetBufferIdentifier returns the file identifier as string
func GetBufferIdentifier(buf []byte) string {
	if len(buf) < SizeUOffsetT+fileIdentifierLength {
		return ""
	}
	return string(buf[SizeUOffsetT:][:fileIdentifierLength])
}

// GetBufferIdentifier returns the file identifier as string for a size-prefixed buffer
func GetSizePrefixedBufferIdentifier(buf []byte) string {
	if len(buf) < SizeUOffsetT+int(sizePrefixLength)+fileIdentifierLength {
		return ""
	}
	return string(buf[SizeUOffsetT+sizePrefixLength:][:fileIdentifierLength])
}

// BufferHasIdentifier checks if the identifier in a buffer has the expected value
func BufferHasIdentifier(buf []byte, identifier string) bool {
	return GetBufferIdentifier(buf) == identifier
}

// BufferHasIdentifier checks if the identifier in a buffer has the expected value for a size-prefixed buffer
func SizePrefixedBufferHasIdentifier(buf []byte, identifier string) bool {
	return GetSizePrefixedBufferIdentifier(buf) == identifier
}
