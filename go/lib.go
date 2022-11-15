package flatbuffers

// FlatBuffer is the interface that represents a flatbuffer.
type FlatBuffer interface {
	Table() Table
	Init(buf []byte, i UOffsetT)
}

// GetRootAs is a generic helper to initialize a FlatBuffer with the provided buffer bytes and its data offset.
func GetRootAs(buf []byte, offset UOffsetT, fb FlatBuffer) {
	n := GetUOffsetT(buf[offset:])
	fb.Init(buf, n+offset)
}

// GetSizePrefixedRootAs is a generic helper to initialize a FlatBuffer with the provided size-prefixed buffer
// bytes and its data offset
func GetSizePrefixedRootAs(buf []byte, offset UOffsetT, fb FlatBuffer) {
	n := GetUOffsetT(buf[offset+sizePrefixLength:])
	fb.Init(buf, n+offset+sizePrefixLength)
}

// GetSizePrefix reads the size from a size-prefixed flatbuffer
func GetSizePrefix(buf []byte, offset UOffsetT) uint32 {
	return GetUint32(buf[offset:])
}

// GetFieldOffset computes the offset of field inside a vtable relative to the provided buffer.
func GetFieldOffset(buf []byte, vtableOffset VOffsetT, offset UOffsetT) UOffsetT {
	vtable := UOffsetT(len(buf)) - offset
	field := vtable + UOffsetT(vtableOffset) - GetUOffsetT(buf[vtable:])
	return UOffsetT(GetVOffsetT(buf[field:])) + vtable
}

// GetIndirectOffset retrives the realtive offset in the provided buffer stored at `offset`.
func GetIndirectOffset(buf []byte, offset UOffsetT) UOffsetT {
	return offset + GetUOffsetT(buf[offset:])
}
