// Package flatbuffers provides the Go runtime for reading, writing, and
// verifying FlatBuffers-encoded data.
//
// # Buffer Verification
//
// The [Verifier] type validates the structural integrity of a raw FlatBuffer
// byte slice before any data is read from it. Verification should be performed
// on all untrusted input (e.g., data received over the network or read from
// disk) to prevent memory corruption bugs, denial-of-service via depth bombs
// or table bombs, and reads from out-of-bounds offsets.
//
// In normal usage callers do not construct a Verifier directly. The flatc
// compiler emits a VerifyRootAs* function alongside every generated table type.
// Use that generated function:
//
//	buf := receivedBytes  // untrusted wire data
//	if err := myschema.VerifyRootAsMonster(buf); err != nil {
//	    var ve *flatbuffers.VerificationError
//	    if errors.As(err, &ve) {
//	        log.Printf("verification failed: kind=%d offset=%d field=%q", ve.Kind, ve.Offset, ve.Field)
//	    }
//	    return err
//	}
//	monster := myschema.GetRootAsMonster(buf, 0)
//	// safe to read from monster now
//
// When you need to tune verification limits (e.g., to allow deeper nesting for
// a known-safe internal message), construct a [Verifier] explicitly via
// [NewVerifier] and call the generated VerifyAs* method directly.
package flatbuffers

import (
	"fmt"
	"unicode/utf8"
)

// VerifierOptions configures the safety limits enforced by a [Verifier].
// All fields are optional; zero values are replaced with safe defaults by
// [NewVerifier].
type VerifierOptions struct {
	// MaxDepth is the maximum allowed nesting depth of tables and vectors.
	// Exceeding this limit returns [ErrDepthLimitReached].
	// Default: 64.
	MaxDepth int

	// MaxTables is the maximum total number of tables that may be visited
	// across the entire verification pass.
	// Exceeding this limit returns [ErrTooManyTables].
	// Default: 1 000 000.
	MaxTables int

	// MaxApparentSize is the maximum number of bytes that any single
	// range check may address within the buffer.
	// Exceeding this limit returns [ErrApparentSizeTooLarge].
	// Default: 1 073 741 824 (1 GiB).
	MaxApparentSize int

	// IgnoreNullTerminator skips the check that every FlatBuffers string ends
	// with a zero byte. Set this only when interoperating with encoders that
	// are known to omit null terminators.
	// Default: false.
	IgnoreNullTerminator bool
}

// ErrorKind categorizes the type of structural violation found during
// verification. It is exposed as [VerificationError].Kind so callers can
// switch on the failure reason without parsing the error string.
type ErrorKind int

const (
	// ErrMissingRequiredField is returned when a field marked as required in
	// the schema is absent from the vtable.
	ErrMissingRequiredField ErrorKind = iota

	// ErrRangeOutOfBounds is returned when an offset or length would address
	// bytes outside the buffer slice.
	ErrRangeOutOfBounds

	// ErrDepthLimitReached is returned when recursive table/vector nesting
	// exceeds [VerifierOptions].MaxDepth.
	ErrDepthLimitReached

	// ErrTooManyTables is returned when the total number of tables visited
	// during verification exceeds [VerifierOptions].MaxTables.
	ErrTooManyTables

	// ErrApparentSizeTooLarge is returned when a range check would address
	// more bytes than [VerifierOptions].MaxApparentSize allows.
	ErrApparentSizeTooLarge

	// ErrUtf8Error is returned when the byte content of a FlatBuffers string
	// is not valid UTF-8.
	ErrUtf8Error

	// ErrMissingNullTerminator is returned when a FlatBuffers string does not
	// end with a zero byte and [VerifierOptions].IgnoreNullTerminator is false.
	ErrMissingNullTerminator

	// ErrUnaligned is returned when a scalar or offset field is not naturally
	// aligned within the buffer.
	ErrUnaligned

	// ErrInconsistentUnion is returned when a union type field and its
	// corresponding value field are not both present or both absent.
	ErrInconsistentUnion

	// ErrInvalidVtable is returned when a vtable's size field is malformed,
	// too small, or not a multiple of the VOffsetT size.
	ErrInvalidVtable

	// ErrInvalidVectorLength is returned when a vector's stored element count
	// would overflow the buffer or produce a negative length.
	ErrInvalidVectorLength

	// ErrSignedOffsetOutOfBounds is returned when a signed offset (SOffsetT)
	// used to locate a vtable resolves to a position outside the buffer.
	ErrSignedOffsetOutOfBounds
)

// TraceKind identifies the kind of location in a buffer graph.
type TraceKind int

const (
	TraceTableField TraceKind = iota
	TraceVectorElement
	TraceUnionVariant
)

// TraceDetail records a position in the buffer graph where an error was found.
type TraceDetail struct {
	Kind     TraceKind
	Name     string // field name, variant name, or empty
	Index    int    // vector element index, or -1
	Position int    // buffer offset
}

// VerificationError is the concrete error type returned by all Verifier
// methods. It carries the failure category, the name of the offending field
// (when applicable), and the byte offset within the buffer where the violation
// was detected.
//
// The error message format produced by Error() is:
//
//	"<human description> at offset <N>"
//	"missing required field <name> at offset <N>"   // ErrMissingRequiredField
//	"inconsistent union field <name> at offset <N>" // ErrInconsistentUnion
//
// To inspect the failure programmatically, unwrap with errors.As:
//
//	var ve *flatbuffers.VerificationError
//	if errors.As(err, &ve) {
//	    switch ve.Kind {
//	    case flatbuffers.ErrDepthLimitReached:
//	        // increase MaxDepth or reject the message
//	    case flatbuffers.ErrMissingRequiredField:
//	        log.Printf("required field %q missing", ve.Field)
//	    }
//	}
type VerificationError struct {
	// Kind identifies the category of the verification failure.
	Kind ErrorKind

	// Field is the schema field name associated with the error, or empty when
	// the failure is not field-specific (e.g., range or alignment errors).
	Field string

	// Offset is the byte offset within the buffer where the violation was
	// detected.
	Offset int

	// Trace records the path through the buffer graph from the root to the
	// failing location. It is populated only when the verifier is run via a
	// generated VerifyAs* function that propagates trace context.
	Trace []TraceDetail
}

func (e *VerificationError) Error() string {
	switch e.Kind {
	case ErrMissingRequiredField:
		return fmt.Sprintf("missing required field %q at offset %d", e.Field, e.Offset)
	case ErrRangeOutOfBounds:
		return fmt.Sprintf("range out of bounds at offset %d", e.Offset)
	case ErrDepthLimitReached:
		return fmt.Sprintf("nested table depth limit reached at offset %d", e.Offset)
	case ErrTooManyTables:
		return fmt.Sprintf("too many tables at offset %d", e.Offset)
	case ErrApparentSizeTooLarge:
		return fmt.Sprintf("apparent size too large at offset %d", e.Offset)
	case ErrUtf8Error:
		return fmt.Sprintf("invalid UTF-8 string at offset %d", e.Offset)
	case ErrMissingNullTerminator:
		return fmt.Sprintf("string missing null terminator at offset %d", e.Offset)
	case ErrUnaligned:
		return fmt.Sprintf("unaligned access at offset %d", e.Offset)
	case ErrInconsistentUnion:
		return fmt.Sprintf("inconsistent union field %q at offset %d", e.Field, e.Offset)
	case ErrInvalidVtable:
		return fmt.Sprintf("invalid vtable at offset %d", e.Offset)
	case ErrInvalidVectorLength:
		return fmt.Sprintf("invalid vector length at offset %d", e.Offset)
	case ErrSignedOffsetOutOfBounds:
		return fmt.Sprintf("signed offset out of bounds at offset %d", e.Offset)
	default:
		return fmt.Sprintf("verification error (kind %d) at offset %d", int(e.Kind), e.Offset)
	}
}

// Verifier validates the structural integrity of a raw FlatBuffer byte slice.
// It enforces configurable limits on nesting depth, total table count, and
// apparent buffer size to guard against denial-of-service payloads.
//
// Verifier is not safe for concurrent use. Create one per goroutine or per
// message that needs to be verified.
//
// In most cases callers should use the generated VerifyRootAs* functions
// rather than constructing a Verifier directly. See the package-level
// documentation for a complete usage example.
type Verifier struct {
	buf       []byte
	opts      VerifierOptions
	depth     int
	numTables int
}

// NewVerifier creates a Verifier for buf using the supplied options.
// If opts is nil, or if any numeric field in opts is zero, the corresponding
// default is applied (MaxDepth=64, MaxTables=1 000 000,
// MaxApparentSize=1 073 741 824). The returned Verifier is ready to use and
// holds no references to opts after this call returns.
func NewVerifier(buf []byte, opts *VerifierOptions) *Verifier {
	v := &Verifier{buf: buf}
	if opts != nil {
		v.opts = *opts
	}
	if v.opts.MaxDepth == 0 {
		v.opts.MaxDepth = 64
	}
	if v.opts.MaxTables == 0 {
		v.opts.MaxTables = 1_000_000
	}
	if v.opts.MaxApparentSize == 0 {
		v.opts.MaxApparentSize = 1_073_741_824
	}
	return v
}

// verifyErr constructs a VerificationError on the error path only.
func verifyErr(kind ErrorKind, field string, offset int) error {
	return &VerificationError{Kind: kind, Field: field, Offset: offset}
}

// CheckAlignment verifies that pos is aligned to align bytes within the
// buffer. It returns [ErrUnaligned] if pos % align != 0. An alignment value
// of 1 (or less) is always considered aligned and never produces an error.
func (v *Verifier) CheckAlignment(pos int, align int) error {
	if align > 1 && pos%align != 0 {
		return verifyErr(ErrUnaligned, "", pos)
	}
	return nil
}

// CheckRange verifies that the half-open byte range [pos, pos+size) is
// entirely within the buffer and does not exceed [VerifierOptions].MaxApparentSize.
// It returns [ErrRangeOutOfBounds] if the range extends past the end of the
// buffer, or [ErrApparentSizeTooLarge] if the range exceeds the size limit.
// Integer overflow is avoided by using subtraction rather than addition when
// checking the upper bound.
func (v *Verifier) CheckRange(pos int, size int) error {
	if size < 0 || pos < 0 {
		return verifyErr(ErrRangeOutOfBounds, "", pos)
	}
	// Use subtraction to avoid integer overflow on pos+size.
	if size > len(v.buf)-pos {
		return verifyErr(ErrRangeOutOfBounds, "", pos)
	}
	if size > v.opts.MaxApparentSize-pos {
		return verifyErr(ErrApparentSizeTooLarge, "", pos)
	}
	return nil
}

// CheckUOffsetT reads the 4-byte little-endian UOffsetT value at pos, verifies
// that both the offset field itself and the target address (pos + value) lie
// within the buffer, and returns the raw offset value. It returns
// [ErrRangeOutOfBounds] if either the field bytes or the target address are
// out of range.
func (v *Verifier) CheckUOffsetT(pos int) (uint32, error) {
	if err := v.CheckRange(pos, SizeUOffsetT); err != nil {
		return 0, err
	}
	val := uint32(GetUOffsetT(v.buf[pos:]))
	// Check that the target position is reachable without overflow.
	if int(val) > len(v.buf)-pos {
		return 0, verifyErr(ErrRangeOutOfBounds, "", pos)
	}
	return val, nil
}

// CheckSOffsetT reads the 4-byte little-endian SOffsetT value at pos, verifies
// that the signed-offset target address (pos + value) lies within the buffer,
// and returns the raw signed value. Signed offsets are used to locate vtables
// relative to their owning table. It returns [ErrSignedOffsetOutOfBounds] if
// the target address is outside the buffer.
func (v *Verifier) CheckSOffsetT(pos int) (int32, error) {
	if err := v.CheckRange(pos, SizeSOffsetT); err != nil {
		return 0, err
	}
	val := int32(GetSOffsetT(v.buf[pos:]))
	target := pos + int(val)
	if target < 0 || target >= len(v.buf) {
		return 0, verifyErr(ErrSignedOffsetOutOfBounds, "", pos)
	}
	return val, nil
}

// vtablePos computes the vtable position from a table position.
// Returns the vtable position and any error.
func (v *Verifier) vtablePos(tablePos int) (int, error) {
	if err := v.CheckRange(tablePos, SizeSOffsetT); err != nil {
		return 0, err
	}
	soffset := int32(GetSOffsetT(v.buf[tablePos:]))
	vtable := tablePos - int(soffset)
	if vtable < 0 || vtable >= len(v.buf) {
		return 0, verifyErr(ErrSignedOffsetOutOfBounds, "", tablePos)
	}
	return vtable, nil
}

// CheckVtable verifies the vtable reachable from the table at tablePos.
// It follows the SOffsetT stored at tablePos backward to the vtable and
// validates that the vtable's size field is at least 4 bytes, is a multiple
// of SizeVOffsetT, and that the entire vtable lies within the buffer. It
// returns [ErrInvalidVtable] if any of these conditions are violated.
func (v *Verifier) CheckVtable(tablePos int) error {
	vt, err := v.vtablePos(tablePos)
	if err != nil {
		return err
	}
	// Vtable must have at least 4 bytes: vtable-size (VOffsetT) + object-size (VOffsetT).
	if err = v.CheckRange(vt, 2*SizeVOffsetT); err != nil {
		return verifyErr(ErrInvalidVtable, "", vt)
	}
	vtSize := int(GetVOffsetT(v.buf[vt:]))
	// Minimum vtable size is 4 (two VOffsetT fields).
	if vtSize < 2*SizeVOffsetT {
		return verifyErr(ErrInvalidVtable, "", vt)
	}
	// Vtable size must be even (all entries are 2-byte VOffsetT).
	if vtSize%SizeVOffsetT != 0 {
		return verifyErr(ErrInvalidVtable, "", vt)
	}
	// The entire vtable must be in bounds.
	if err = v.CheckRange(vt, vtSize); err != nil {
		return verifyErr(ErrInvalidVtable, "", vt)
	}
	return nil
}

// CheckTable verifies the table at tablePos by incrementing the table counter
// (returning [ErrTooManyTables] if the limit is exceeded) and then validating
// the vtable structure via [Verifier.CheckVtable]. Generated VerifyAs* functions
// call this method once per table they visit.
func (v *Verifier) CheckTable(tablePos int) error {
	if err := v.CountTable(); err != nil {
		return err
	}
	return v.CheckVtable(tablePos)
}

// CheckString verifies the FlatBuffers string whose offset field is stored at
// pos. It follows the UOffsetT indirection, reads the 4-byte length prefix,
// confirms all bytes are valid UTF-8, and (unless
// [VerifierOptions].IgnoreNullTerminator is set) checks that the byte
// immediately after the string body is zero. Returns [ErrUtf8Error] or
// [ErrMissingNullTerminator] on failure, in addition to range errors.
func (v *Verifier) CheckString(pos int) error {
	uoff, err := v.CheckUOffsetT(pos)
	if err != nil {
		return err
	}
	strStart := pos + int(uoff)
	// Read the 4-byte string length.
	if err = v.CheckRange(strStart, SizeUOffsetT); err != nil {
		return err
	}
	strLen := int(uint32(GetUOffsetT(v.buf[strStart:])))
	bodyStart := strStart + SizeUOffsetT
	// Validate the body bytes plus the null terminator.
	if err = v.CheckRange(bodyStart, strLen+1); err != nil {
		return err
	}
	body := v.buf[bodyStart : bodyStart+strLen]
	if !utf8.Valid(body) {
		return verifyErr(ErrUtf8Error, "", bodyStart)
	}
	if !v.opts.IgnoreNullTerminator && v.buf[bodyStart+strLen] != 0 {
		return verifyErr(ErrMissingNullTerminator, "", bodyStart+strLen)
	}
	return nil
}

// CheckVector verifies the FlatBuffers vector whose offset field is stored at
// pos. elementSize is the size in bytes of each inline element (e.g., 1 for
// byte vectors, 4 for uint32 vectors). It follows the UOffsetT indirection,
// reads the 4-byte element-count prefix, guards against integer overflow in
// the total byte calculation, and confirms the entire element array lies within
// the buffer. It returns the element count on success, or an error if any
// check fails.
func (v *Verifier) CheckVector(pos int, elementSize int) (int, error) {
	uoff, err := v.CheckUOffsetT(pos)
	if err != nil {
		return 0, err
	}
	vecStart := pos + int(uoff)
	if err = v.CheckRange(vecStart, SizeUOffsetT); err != nil {
		return 0, err
	}
	vecLen := int(uint32(GetUOffsetT(v.buf[vecStart:])))
	if vecLen < 0 {
		return 0, verifyErr(ErrInvalidVectorLength, "", vecStart)
	}
	elemStart := vecStart + SizeUOffsetT
	totalBytes := vecLen * elementSize
	if elementSize > 0 && totalBytes/elementSize != vecLen {
		// Overflow check.
		return 0, verifyErr(ErrInvalidVectorLength, "", vecStart)
	}
	if err = v.CheckRange(elemStart, totalBytes); err != nil {
		return 0, err
	}
	return vecLen, nil
}

// CheckVectorOfTables verifies a FlatBuffers vector-of-tables whose offset
// field is stored at pos. For each element, it follows the per-element
// UOffsetT indirection, increments the depth counter via [Verifier.PushDepth],
// and delegates per-table validation to verifyElem. verifyElem receives the
// Verifier and the absolute buffer position of the element's table. It returns
// the first error encountered, or nil if all elements are valid.
func (v *Verifier) CheckVectorOfTables(pos int, verifyElem func(v *Verifier, pos int) error) error {
	uoff, err := v.CheckUOffsetT(pos)
	if err != nil {
		return err
	}
	vecStart := pos + int(uoff)
	if err = v.CheckRange(vecStart, SizeUOffsetT); err != nil {
		return err
	}
	vecLen := int(uint32(GetUOffsetT(v.buf[vecStart:])))
	elemStart := vecStart + SizeUOffsetT
	if err = v.CheckRange(elemStart, vecLen*SizeUOffsetT); err != nil {
		return err
	}
	for i := 0; i < vecLen; i++ {
		elemPos := elemStart + i*SizeUOffsetT
		elemUoff, uoffErr := v.CheckUOffsetT(elemPos)
		if uoffErr != nil {
			return uoffErr
		}
		tablePos := elemPos + int(elemUoff)
		if err = v.PushDepth(); err != nil {
			return err
		}
		if err = verifyElem(v, tablePos); err != nil {
			v.PopDepth()
			return err
		}
		v.PopDepth()
	}
	return nil
}

// CheckScalarField verifies that the scalar field at vtable slot vOffset within
// the table at tablePos is in bounds. size is the width of the scalar in bytes
// (e.g., 1, 2, 4, or 8). If the vtable slot is zero (field absent), the method
// returns nil because absent scalar fields take their default value without
// occupying space in the buffer.
func (v *Verifier) CheckScalarField(tablePos int, vOffset VOffsetT, size int) error {
	fieldRelOff, err := v.vtableFieldOffset(tablePos, vOffset)
	if err != nil {
		return err
	}
	if fieldRelOff == 0 {
		return nil // absent; default value applies
	}
	return v.CheckRange(tablePos+int(fieldRelOff), size)
}

// CheckOffsetField looks up the offset field at vtable slot vOffset within the
// table at tablePos, follows the UOffsetT indirection, and returns the absolute
// buffer position of the referenced data. If the vtable slot is zero (field
// absent), it returns 0, nil. The returned position can be passed directly to
// [Verifier.CheckString], [Verifier.CheckVector], or similar methods to verify
// the pointed-to data structure.
func (v *Verifier) CheckOffsetField(tablePos int, vOffset VOffsetT) (int, error) {
	fieldRelOff, err := v.vtableFieldOffset(tablePos, vOffset)
	if err != nil {
		return 0, err
	}
	if fieldRelOff == 0 {
		return 0, nil // field absent
	}
	fieldDataPos := tablePos + int(fieldRelOff)
	if err = v.CheckRange(fieldDataPos, SizeUOffsetT); err != nil {
		return 0, err
	}
	uoff := uint32(GetUOffsetT(v.buf[fieldDataPos:]))
	targetPos := fieldDataPos + int(uoff)
	if targetPos < 0 || targetPos > len(v.buf) {
		return 0, verifyErr(ErrRangeOutOfBounds, "", fieldDataPos)
	}
	return targetPos, nil
}

// CheckRequiredField returns [ErrMissingRequiredField] if the field at vtable
// slot vOffset within the table at tablePos is absent (vtable entry is zero).
// fieldName is included in the error message and in [VerificationError].Field
// for diagnostics. This method is called by generated code for every field
// that is annotated as required in the schema.
func (v *Verifier) CheckRequiredField(tablePos int, vOffset VOffsetT, fieldName string) error {
	fieldRelOff, err := v.vtableFieldOffset(tablePos, vOffset)
	if err != nil {
		return err
	}
	if fieldRelOff == 0 {
		return verifyErr(ErrMissingRequiredField, fieldName, tablePos)
	}
	return nil
}

// PushDepth increments the nesting depth counter and returns
// [ErrDepthLimitReached] if the counter exceeds [VerifierOptions].MaxDepth.
// It must be paired with a corresponding [Verifier.PopDepth] call, even when
// an error is returned. Generated code calls PushDepth before recursing into
// a nested table or vector-of-tables.
func (v *Verifier) PushDepth() error {
	v.depth++
	if v.depth > v.opts.MaxDepth {
		return verifyErr(ErrDepthLimitReached, "", 0)
	}
	return nil
}

// PopDepth decrements the nesting depth counter. It is the counterpart to
// [Verifier.PushDepth] and must be called after returning from each recursive
// table or vector-of-tables verification, regardless of whether verification
// succeeded or failed.
func (v *Verifier) PopDepth() {
	if v.depth > 0 {
		v.depth--
	}
}

// CountTable increments the total table counter and returns
// [ErrTooManyTables] if the count exceeds [VerifierOptions].MaxTables.
// It is called once per table by [Verifier.CheckTable]. Callers that invoke
// CheckTable directly do not need to call CountTable themselves.
func (v *Verifier) CountTable() error {
	v.numTables++
	if v.numTables > v.opts.MaxTables {
		return verifyErr(ErrTooManyTables, "", 0)
	}
	return nil
}

// vtableFieldOffset returns the field's relative offset from the vtable at the given slot.
// Returns 0 if the slot is beyond the vtable size (field absent).
func (v *Verifier) vtableFieldOffset(tablePos int, vOffset VOffsetT) (VOffsetT, error) {
	vt, err := v.vtablePos(tablePos)
	if err != nil {
		return 0, err
	}
	if err = v.CheckRange(vt, SizeVOffsetT); err != nil {
		return 0, err
	}
	vtSize := int(GetVOffsetT(v.buf[vt:]))
	if int(vOffset) >= vtSize {
		return 0, nil // field slot beyond vtable — absent
	}
	if err = v.CheckRange(vt+int(vOffset), SizeVOffsetT); err != nil {
		return 0, err
	}
	return GetVOffsetT(v.buf[vt+int(vOffset):]), nil
}
