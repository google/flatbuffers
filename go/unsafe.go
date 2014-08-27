package flatbuffers

import "unsafe"

var (
	// See http://golang.org/ref/spec#Numeric_types

	// SizeUint8 is the byte size of a uint8.
	SizeUint8 = int(unsafe.Sizeof(uint8(0)))
	// SizeUint16 is the byte size of a uint16.
	SizeUint16 = int(unsafe.Sizeof(uint16(0)))
	// SizeUint32 is the byte size of a uint32.
	SizeUint32 = int(unsafe.Sizeof(uint32(0)))
	// SizeUint64 is the byte size of a uint64.
	SizeUint64 = int(unsafe.Sizeof(uint64(0)))

	// SizeInt8  is the byte size of a int8.
	SizeInt8 = int(unsafe.Sizeof(int8(0)))
	// SizeInt16 is the byte size of a int16.
	SizeInt16 = int(unsafe.Sizeof(int16(0)))
	// SizeInt32 is the byte size of a int32.
	SizeInt32 = int(unsafe.Sizeof(int32(0)))
	// SizeInt64 is the byte size of a int64.
	SizeInt64 = int(unsafe.Sizeof(int64(0)))

	// SizeFloat32 is the byte size of a float32.
	SizeFloat32 = int(unsafe.Sizeof(float32(0)))
	// SizeFloat64 is the byte size of a float64.
	SizeFloat64 = int(unsafe.Sizeof(float64(0)))

	// SizeByte is the byte size of a byte.
	// The `byte` type is aliased (by Go definition) to uint8.
	SizeByte = SizeUint8

	// SizeBool is the byte size of a bool.
	// The `bool` type is aliased (by flatbuffers convention) to uint8.
	SizeBool = SizeUint8

	// SizeSOffsetT is the byte size of an SOffsetT.
	SizeSOffsetT = int(unsafe.Sizeof(SOffsetT(0)))
	// SizeUOffsetT is the byte size of an UOffsetT.
	SizeUOffsetT = int(unsafe.Sizeof(UOffsetT(0)))
	// SizeVOffsetT is the byte size of an VOffsetT.
	SizeVOffsetT = int(unsafe.Sizeof(VOffsetT(0)))
)
