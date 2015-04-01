package flatbuffers

// Table wraps a byte slice and provides read access to its data.
//
// The variable `Pos` indicates the root of the FlatBuffers object therein.
type Table struct {
	Bytes []byte
	Pos   UOffsetT // Always < 1<<31.
}

// Offset provides access into the Table's vtable.
//
// Deprecated fields are ignored by checking against the vtable's length.
func (t *Table) Offset(vtableOffset VOffsetT) VOffsetT {
	vtable := UOffsetT(SOffsetT(t.Pos) - t.GetSOffsetT(t.Pos))
	if vtableOffset < t.GetVOffsetT(vtable) {
		return t.GetVOffsetT(vtable + UOffsetT(vtableOffset))
	}
	return 0
}

// Indirect retrieves the relative offset stored at `offset`.
func (t *Table) Indirect(off UOffsetT) UOffsetT {
	return off + GetUOffsetT(t.Bytes[off:])
}

// String gets a string from data stored inside the flatbuffer.
func (t *Table) String(off UOffsetT) string {
	return string(t.ByteVector(off))
}

// ByteVector gets a byte slice from data stored inside the flatbuffer.
func (t *Table) ByteVector(off UOffsetT) []byte {
	off += GetUOffsetT(t.Bytes[off:])
	start := off + UOffsetT(SizeUOffsetT)
	length := GetUOffsetT(t.Bytes[off:])
	return t.Bytes[start : start+length]
}

// VectorLen retrieves the length of the vector whose offset is stored at
// "off" in this object.
func (t *Table) VectorLen(off UOffsetT) int {
	off += t.Pos
	off += GetUOffsetT(t.Bytes[off:])
	return int(GetUOffsetT(t.Bytes[off:]))
}

// Vector retrieves the start of data of the vector whose offset is stored
// at "off" in this object.
func (t *Table) Vector(off UOffsetT) UOffsetT {
	off += t.Pos
	x := off + GetUOffsetT(t.Bytes[off:])
	// data starts after metadata containing the vector length
	x += UOffsetT(SizeUOffsetT)
	return x
}

// Union initializes any Table-derived type to point to the union at the given
// offset.
func (t *Table) Union(t2 *Table, off UOffsetT) {
	off += t.Pos
	t2.Pos = off + t.GetUOffsetT(off)
	t2.Bytes = t.Bytes
}

// GetBool retrieves a bool at the given offset.
func (t *Table) GetBool(off UOffsetT) bool {
	return GetBool(t.Bytes[off:])
}

// GetByte retrieves a byte at the given offset.
func (t *Table) GetByte(off UOffsetT) byte {
	return GetByte(t.Bytes[off:])
}

// GetUint8 retrieves a uint8 at the given offset.
func (t *Table) GetUint8(off UOffsetT) uint8 {
	return GetUint8(t.Bytes[off:])
}

// GetUint16 retrieves a uint16 at the given offset.
func (t *Table) GetUint16(off UOffsetT) uint16 {
	return GetUint16(t.Bytes[off:])
}

// GetUint32 retrieves a uint32 at the given offset.
func (t *Table) GetUint32(off UOffsetT) uint32 {
	return GetUint32(t.Bytes[off:])
}

// GetUint64 retrieves a uint64 at the given offset.
func (t *Table) GetUint64(off UOffsetT) uint64 {
	return GetUint64(t.Bytes[off:])
}

// GetInt8 retrieves a int8 at the given offset.
func (t *Table) GetInt8(off UOffsetT) int8 {
	return GetInt8(t.Bytes[off:])
}

// GetInt16 retrieves a int16 at the given offset.
func (t *Table) GetInt16(off UOffsetT) int16 {
	return GetInt16(t.Bytes[off:])
}

// GetInt32 retrieves a int32 at the given offset.
func (t *Table) GetInt32(off UOffsetT) int32 {
	return GetInt32(t.Bytes[off:])
}

// GetInt64 retrieves a int64 at the given offset.
func (t *Table) GetInt64(off UOffsetT) int64 {
	return GetInt64(t.Bytes[off:])
}

// GetFloat32 retrieves a float32 at the given offset.
func (t *Table) GetFloat32(off UOffsetT) float32 {
	return GetFloat32(t.Bytes[off:])
}

// GetFloat64 retrieves a float64 at the given offset.
func (t *Table) GetFloat64(off UOffsetT) float64 {
	return GetFloat64(t.Bytes[off:])
}

// GetUOffsetT retrieves a UOffsetT at the given offset.
func (t *Table) GetUOffsetT(off UOffsetT) UOffsetT {
	return GetUOffsetT(t.Bytes[off:])
}

// GetVOffsetT retrieves a VOffsetT at the given offset.
func (t *Table) GetVOffsetT(off UOffsetT) VOffsetT {
	return GetVOffsetT(t.Bytes[off:])
}

// GetSOffsetT retrieves a SOffsetT at the given offset.
func (t *Table) GetSOffsetT(off UOffsetT) SOffsetT {
	return GetSOffsetT(t.Bytes[off:])
}

// GetBoolSlot retrieves the bool that the given vtable location
// points to. If the vtable value is zero, the default value `d`
// will be returned.
func (t *Table) GetBoolSlot(slot VOffsetT, d bool) bool {
	off := t.Offset(slot)
	if off == 0 {
		return d
	}

	return t.GetBool(t.Pos + UOffsetT(off))
}

// GetByteSlot retrieves the byte that the given vtable location
// points to. If the vtable value is zero, the default value `d`
// will be returned.
func (t *Table) GetByteSlot(slot VOffsetT, d byte) byte {
	off := t.Offset(slot)
	if off == 0 {
		return d
	}

	return t.GetByte(t.Pos + UOffsetT(off))
}

// GetInt8Slot retrieves the int8 that the given vtable location
// points to. If the vtable value is zero, the default value `d`
// will be returned.
func (t *Table) GetInt8Slot(slot VOffsetT, d int8) int8 {
	off := t.Offset(slot)
	if off == 0 {
		return d
	}

	return t.GetInt8(t.Pos + UOffsetT(off))
}

// GetUint8Slot retrieves the uint8 that the given vtable location
// points to. If the vtable value is zero, the default value `d`
// will be returned.
func (t *Table) GetUint8Slot(slot VOffsetT, d uint8) uint8 {
	off := t.Offset(slot)
	if off == 0 {
		return d
	}

	return t.GetUint8(t.Pos + UOffsetT(off))
}

// GetInt16Slot retrieves the int16 that the given vtable location
// points to. If the vtable value is zero, the default value `d`
// will be returned.
func (t *Table) GetInt16Slot(slot VOffsetT, d int16) int16 {
	off := t.Offset(slot)
	if off == 0 {
		return d
	}

	return t.GetInt16(t.Pos + UOffsetT(off))
}

// GetUint16Slot retrieves the uint16 that the given vtable location
// points to. If the vtable value is zero, the default value `d`
// will be returned.
func (t *Table) GetUint16Slot(slot VOffsetT, d uint16) uint16 {
	off := t.Offset(slot)
	if off == 0 {
		return d
	}

	return t.GetUint16(t.Pos + UOffsetT(off))
}

// GetInt32Slot retrieves the int32 that the given vtable location
// points to. If the vtable value is zero, the default value `d`
// will be returned.
func (t *Table) GetInt32Slot(slot VOffsetT, d int32) int32 {
	off := t.Offset(slot)
	if off == 0 {
		return d
	}

	return t.GetInt32(t.Pos + UOffsetT(off))
}

// GetUint32Slot retrieves the uint32 that the given vtable location
// points to. If the vtable value is zero, the default value `d`
// will be returned.
func (t *Table) GetUint32Slot(slot VOffsetT, d uint32) uint32 {
	off := t.Offset(slot)
	if off == 0 {
		return d
	}

	return t.GetUint32(t.Pos + UOffsetT(off))
}

// GetInt64Slot retrieves the int64 that the given vtable location
// points to. If the vtable value is zero, the default value `d`
// will be returned.
func (t *Table) GetInt64Slot(slot VOffsetT, d int64) int64 {
	off := t.Offset(slot)
	if off == 0 {
		return d
	}

	return t.GetInt64(t.Pos + UOffsetT(off))
}

// GetUint64Slot retrieves the uint64 that the given vtable location
// points to. If the vtable value is zero, the default value `d`
// will be returned.
func (t *Table) GetUint64Slot(slot VOffsetT, d uint64) uint64 {
	off := t.Offset(slot)
	if off == 0 {
		return d
	}

	return t.GetUint64(t.Pos + UOffsetT(off))
}

// GetFloat32Slot retrieves the float32 that the given vtable location
// points to. If the vtable value is zero, the default value `d`
// will be returned.
func (t *Table) GetFloat32Slot(slot VOffsetT, d float32) float32 {
	off := t.Offset(slot)
	if off == 0 {
		return d
	}

	return t.GetFloat32(t.Pos + UOffsetT(off))
}

// GetFloat64Slot retrieves the float64 that the given vtable location
// points to. If the vtable value is zero, the default value `d`
// will be returned.
func (t *Table) GetFloat64Slot(slot VOffsetT, d float64) float64 {
	off := t.Offset(slot)
	if off == 0 {
		return d
	}

	return t.GetFloat64(t.Pos + UOffsetT(off))
}

// GetVOffsetTSlot retrieves the VOffsetT that the given vtable location
// points to. If the vtable value is zero, the default value `d`
// will be returned.
func (t *Table) GetVOffsetTSlot(slot VOffsetT, d VOffsetT) VOffsetT {
	off := t.Offset(slot)
	if off == 0 {
		return d
	}
	return VOffsetT(off)
}
