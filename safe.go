// +build !unsafe

package flatbuffers

// byteSliceToString converts a []byte to string.
func byteSliceToString(b []byte) string {
	return string(b)
}
