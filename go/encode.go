package flatbuffers

import (
	"fmt"
	"math"
)

type (
	// A SOffsetT stores a signed offset into arbitrary data.
	SOffsetT int32
	// A UOffsetT stores an unsigned offset into vector data.
	UOffsetT uint32
	// A VOffsetT stores an unsigned offset in a vtable.
	VOffsetT uint16
)

const (
	// VtableMetadataFields is the count of metadata fields in each vtable.
	VtableMetadataFields = 2
)

// GetByte decodes a little-endian byte from a byte slice.
func GetByte(buf []byte) byte {
	return byte(GetUint8(buf))
}

// SafeGetByte first bounds checks the byte slice to ensure a length of 1
// then decodes a little-endian byte from the byte slice
func SafeGetByte(buf []byte) (byte, error) {
	x, err := SafeGetUint8(buf)
	if err != nil {
		return byte(0), fmt.Errorf("Bounds check failed for returning safe byte value from SafeGetUint8")
	}
	return byte(x), nil
}

// GetBool decodes a little-endian bool from a byte slice.
func GetBool(buf []byte) bool {
	return buf[0] == 1
}

// SafeGetBool first bounds checks the byte slice to ensure a length of 1
// then decodes a little-endian bool from the byte slice
func SafeGetBool(buf []byte) (bool, error) {
	if len(buf) != 1 {
		return false, fmt.Errorf("Bounds check failed for returning safe boolean")
	}
	return buf[0] == 1, nil
}

// GetUint8 decodes a little-endian uint8 from a byte slice.
func GetUint8(buf []byte) (n uint8) {
	n = uint8(buf[0])
	return
}

// SafeGetUint8 first bounds checks the byte slice to ensure a length of 1
// then decodes a little-endian uint8 from the byte slice
func SafeGetUint8(buf []byte) (n uint8, err error) {
	if len(buf) != 1 {
		n = uint8(0)
		err = fmt.Errorf("Bounds check failed for returning safe uint8")
		return
	}
	n = uint8(buf[0])
	err = nil
	return
}

// GetUint16 decodes a little-endian uint16 from a byte slice.
func GetUint16(buf []byte) (n uint16) {
	_ = buf[1] // Force one bounds check. See: golang.org/issue/14808
	n |= uint16(buf[0])
	n |= uint16(buf[1]) << 8
	return
}

// SafeGetUint16 first bounds checks the byte slice to ensure a length of 2
// then decodes a little-endian uint16 from the byte slice
func SafeGetUint16(buf []byte) (n uint16, err error) {
	if len(buf) != 2 {
		n = uint16(0)
		err = fmt.Errorf("Bounds check failed for returning safe uint16")
		return
	}
	n |= uint16(buf[0])
	n |= uint16(buf[1]) << 8
	return
}

// GetUint32 decodes a little-endian uint32 from a byte slice.
func GetUint32(buf []byte) (n uint32) {
	_ = buf[3] // Force one bounds check. See: golang.org/issue/14808
	n |= uint32(buf[0])
	n |= uint32(buf[1]) << 8
	n |= uint32(buf[2]) << 16
	n |= uint32(buf[3]) << 24
	return
}

// SafeGetUint32 first bounds checks the byte slice to ensure a length of 4
// then decodes a little-endian uint32 from the byte slice
func SafeGetUint32(buf []byte) (n uint32, err error) {
	if len(buf) != 4 {
		n = uint32(0)
		err = fmt.Errorf("Bounds check failed for returning safe uint32")
		return
	}
	n |= uint32(buf[0])
	n |= uint32(buf[1]) << 8
	n |= uint32(buf[2]) << 16
	n |= uint32(buf[3]) << 24
	return
}

// GetUint64 decodes a little-endian uint64 from a byte slice.
func GetUint64(buf []byte) (n uint64) {
	_ = buf[7] // Force one bounds check. See: golang.org/issue/14808
	n |= uint64(buf[0])
	n |= uint64(buf[1]) << 8
	n |= uint64(buf[2]) << 16
	n |= uint64(buf[3]) << 24
	n |= uint64(buf[4]) << 32
	n |= uint64(buf[5]) << 40
	n |= uint64(buf[6]) << 48
	n |= uint64(buf[7]) << 56
	return
}

// SafeGetUint64 first bounds checks the byte slice to ensure a length of 8
// then decodes a little-endian uint64 from the byte slice
func SafeGetUint64(buf []byte) (n uint64, err error) {
	if len(buf) != 8 {
		n = uint64(0)
		err = fmt.Errorf("Bounds check failed for returning safe uint64")
		return
	}
	n |= uint64(buf[0])
	n |= uint64(buf[1]) << 8
	n |= uint64(buf[2]) << 16
	n |= uint64(buf[3]) << 24
	n |= uint64(buf[4]) << 32
	n |= uint64(buf[5]) << 40
	n |= uint64(buf[6]) << 48
	n |= uint64(buf[7]) << 56
	return
}

// GetInt8 decodes a little-endian int8 from a byte slice.
func GetInt8(buf []byte) (n int8) {
	n = int8(buf[0])
	return
}

// SafeGetInt8 first bounds checks the byte slice to ensure a length of 1
// then decodes a little-endian int8 from the byte slice
func SafeGetInt8(buf []byte) (n int8, err error) {
	if len(buf) != 1 {
		n = int8(0)
		err = fmt.Errorf("Bounds check failed for returning safe int8")
		return
	}
	n = int8(buf[0])
	err = nil
	return
}

// GetInt16 decodes a little-endian int16 from a byte slice.
func GetInt16(buf []byte) (n int16) {
	_ = buf[1] // Force one bounds check. See: golang.org/issue/14808
	n |= int16(buf[0])
	n |= int16(buf[1]) << 8
	return
}

// SafeGetInt16 first bounds checks the byte slice to ensure a length of 2
// then decodes a little-endian int16 from the byte slice
func SafeGetInt16(buf []byte) (n int16, err error) {
	if len(buf) != 2 {
		n = int16(0)
		err = fmt.Errorf("Bounds check failed for returning safe int16")
		return
	}
	n |= int16(buf[0])
	n |= int16(buf[1]) << 8
	return
}

// GetInt32 decodes a little-endian int32 from a byte slice.
func GetInt32(buf []byte) (n int32) {
	_ = buf[3] // Force one bounds check. See: golang.org/issue/14808
	n |= int32(buf[0])
	n |= int32(buf[1]) << 8
	n |= int32(buf[2]) << 16
	n |= int32(buf[3]) << 24
	return
}

// SafeGetInt32 first bounds checks the byte slice to ensure a length of 4
// then decodes a little-endian int32 from the byte slice
func SafeGetInt32(buf []byte) (n int32, err error) {
	if len(buf) != 4 {
		n = int32(0)
		err = fmt.Errorf("Bounds check failed for returning safe int32")
		return
	}
	n |= int32(buf[0])
	n |= int32(buf[1]) << 8
	n |= int32(buf[2]) << 16
	n |= int32(buf[3]) << 24
	return
}

// GetInt64 decodes a little-endian int64 from a byte slice.
func GetInt64(buf []byte) (n int64) {
	_ = buf[7] // Force one bounds check. See: golang.org/issue/14808
	n |= int64(buf[0])
	n |= int64(buf[1]) << 8
	n |= int64(buf[2]) << 16
	n |= int64(buf[3]) << 24
	n |= int64(buf[4]) << 32
	n |= int64(buf[5]) << 40
	n |= int64(buf[6]) << 48
	n |= int64(buf[7]) << 56
	return
}

// SafeGetInt64 first bounds checks the byte slice to ensure a length of 8
// then decodes a little-endian int64 from the byte slice
func SafeGetInt64(buf []byte) (n int64, err error) {
	if len(buf) != 8 {
		n = int64(0)
		err = fmt.Errorf("Bounds check failed for returning safe int64")
		return
	}
	n |= int64(buf[0])
	n |= int64(buf[1]) << 8
	n |= int64(buf[2]) << 16
	n |= int64(buf[3]) << 24
	n |= int64(buf[4]) << 32
	n |= int64(buf[5]) << 40
	n |= int64(buf[6]) << 48
	n |= int64(buf[7]) << 56
	return
}

// GetFloat32 decodes a little-endian float32 from a byte slice.
func GetFloat32(buf []byte) float32 {
	x := GetUint32(buf)
	return math.Float32frombits(x)
}

// SafeGetFloat32 first bounds checks the byte slice to ensure a length of 4
// then decodes a little-endian float32 from the byte slice
func SafeGetFloat32(buf []byte) (float32, error) {
	x, err := SafeGetUint32(buf)
	if err != nil {
		return float32(0), fmt.Errorf("Bounds check failed for returning safe float32 from SafeGetUint32")
	}
	return math.Float32frombits(x), nil
}

// GetFloat64 decodes a little-endian float64 from a byte slice.
func GetFloat64(buf []byte) float64 {
	x := GetUint64(buf)
	return math.Float64frombits(x)
}

// SafeGetFloat64 first bounds checks the byte slice to ensure a length of 8
// then decodes a little-endian float64 from the byte slice
func SafeGetFloat64(buf []byte) (float64, error) {
	x, err := SafeGetUint64(buf)
	if err != nil {
		return float64(0), fmt.Errorf("Bounds check failed for returning safe float64 from SafeGetUint64")
	}
	return math.Float64frombits(x), nil
}

// GetUOffsetT decodes a little-endian UOffsetT from a byte slice.
func GetUOffsetT(buf []byte) UOffsetT {
	return UOffsetT(GetUint32(buf))
}

// SafeGetUOffsetT bounds checks the byte slice to ensure a length of 4
// then decodes a little-endian UOffsetT from the byte slice
func SafeGetUOffsetT(buf []byte) (UOffsetT, error) {
	x, err := SafeGetUint32(buf)
	if err != nil {
		return UOffsetT(0), fmt.Errorf("Bounds check failed for returning safe UOffsetT from SafeGetUint32")
	}
	return UOffsetT(x), nil
}

// GetSOffsetT decodes a little-endian SOffsetT from a byte slice.
func GetSOffsetT(buf []byte) SOffsetT {
	return SOffsetT(GetInt32(buf))
}

// SafeGetSOffsetT bounds checks the byte slice to ensure a length of 4
// then decodes a little-endian SOffsetT from the byte slice
func SafeGetSOffsetT(buf []byte) (SOffsetT, error) {
	x, err := SafeGetUint32(buf)
	if err != nil {
		return SOffsetT(0), fmt.Errorf("Bounds check failed for returning safe SOffsetT from SafeGetUint32")
	}
	return SOffsetT(x), nil
}

// GetVOffsetT decodes a little-endian VOffsetT from a byte slice.
func GetVOffsetT(buf []byte) VOffsetT {
	return VOffsetT(GetUint16(buf))
}

// SafeGetVOffsetT bounds checks the byte slice to ensure a length of 4
// then decodes a little-endian VOffsetT from the byte slice
func SafeGetVOffsetT(buf []byte) (VOffsetT, error) {
	x, err := SafeGetUint32(buf)
	if err != nil {
		return VOffsetT(0), fmt.Errorf("Bounds check failed for returning safe VOffsetT from SafeGetUint32")
	}
	return VOffsetT(x), nil
}

// WriteByte encodes a little-endian uint8 into a byte slice.
func WriteByte(buf []byte, n byte) {
	WriteUint8(buf, uint8(n))
}

// WriteBool encodes a little-endian bool into a byte slice.
func WriteBool(buf []byte, b bool) {
	buf[0] = 0
	if b {
		buf[0] = 1
	}
}

// WriteUint8 encodes a little-endian uint8 into a byte slice.
func WriteUint8(buf []byte, n uint8) {
	buf[0] = byte(n)
}

// WriteUint16 encodes a little-endian uint16 into a byte slice.
func WriteUint16(buf []byte, n uint16) {
	_ = buf[1] // Force one bounds check. See: golang.org/issue/14808
	buf[0] = byte(n)
	buf[1] = byte(n >> 8)
}

// WriteUint32 encodes a little-endian uint32 into a byte slice.
func WriteUint32(buf []byte, n uint32) {
	_ = buf[3] // Force one bounds check. See: golang.org/issue/14808
	buf[0] = byte(n)
	buf[1] = byte(n >> 8)
	buf[2] = byte(n >> 16)
	buf[3] = byte(n >> 24)
}

// WriteUint64 encodes a little-endian uint64 into a byte slice.
func WriteUint64(buf []byte, n uint64) {
	_ = buf[7] // Force one bounds check. See: golang.org/issue/14808
	buf[0] = byte(n)
	buf[1] = byte(n >> 8)
	buf[2] = byte(n >> 16)
	buf[3] = byte(n >> 24)
	buf[4] = byte(n >> 32)
	buf[5] = byte(n >> 40)
	buf[6] = byte(n >> 48)
	buf[7] = byte(n >> 56)
}

// WriteInt8 encodes a little-endian int8 into a byte slice.
func WriteInt8(buf []byte, n int8) {
	buf[0] = byte(n)
}

// WriteInt16 encodes a little-endian int16 into a byte slice.
func WriteInt16(buf []byte, n int16) {
	_ = buf[1] // Force one bounds check. See: golang.org/issue/14808
	buf[0] = byte(n)
	buf[1] = byte(n >> 8)
}

// WriteInt32 encodes a little-endian int32 into a byte slice.
func WriteInt32(buf []byte, n int32) {
	_ = buf[3] // Force one bounds check. See: golang.org/issue/14808
	buf[0] = byte(n)
	buf[1] = byte(n >> 8)
	buf[2] = byte(n >> 16)
	buf[3] = byte(n >> 24)
}

// WriteInt64 encodes a little-endian int64 into a byte slice.
func WriteInt64(buf []byte, n int64) {
	_ = buf[7] // Force one bounds check. See: golang.org/issue/14808
	buf[0] = byte(n)
	buf[1] = byte(n >> 8)
	buf[2] = byte(n >> 16)
	buf[3] = byte(n >> 24)
	buf[4] = byte(n >> 32)
	buf[5] = byte(n >> 40)
	buf[6] = byte(n >> 48)
	buf[7] = byte(n >> 56)
}

// WriteFloat32 encodes a little-endian float32 into a byte slice.
func WriteFloat32(buf []byte, n float32) {
	WriteUint32(buf, math.Float32bits(n))
}

// WriteFloat64 encodes a little-endian float64 into a byte slice.
func WriteFloat64(buf []byte, n float64) {
	WriteUint64(buf, math.Float64bits(n))
}

// WriteVOffsetT encodes a little-endian VOffsetT into a byte slice.
func WriteVOffsetT(buf []byte, n VOffsetT) {
	WriteUint16(buf, uint16(n))
}

// WriteSOffsetT encodes a little-endian SOffsetT into a byte slice.
func WriteSOffsetT(buf []byte, n SOffsetT) {
	WriteInt32(buf, int32(n))
}

// WriteUOffsetT encodes a little-endian UOffsetT into a byte slice.
func WriteUOffsetT(buf []byte, n UOffsetT) {
	WriteUint32(buf, uint32(n))
}
