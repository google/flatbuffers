// +build unsafe

package flatbuffers

import "unsafe"

package flatbuffers

// byteSliceToString converts a []byte to string without a heap allocation.
func byteSliceToString(b []byte) string {
	return *(*string)(unsafe.Pointer(&b))
}
