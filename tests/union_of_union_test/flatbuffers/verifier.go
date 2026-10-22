package flatbuffers

import (
	"fmt"
	"unicode/utf8"
)

// VerifierOptions configures verification limits.
// Zero values use sensible defaults.
type VerifierOptions struct {
	MaxDepth             int // default: 64
	MaxTables            int // default: 1000000
	MaxApparentSize      int // default: 1073741824 (1 GB)
	IgnoreNullTerminator bool
}

// ErrorKind categorizes verification failures.
type ErrorKind int

const (
	ErrMissingRequiredField ErrorKind = iota
	ErrRangeOutOfBounds
	ErrDepthLimitReached
	ErrTooManyTables
	ErrApparentSizeTooLarge
	ErrUtf8Error
	ErrMissingNullTerminator
	ErrUnaligned
	ErrInconsistentUnion
	ErrInvalidVtable
	ErrInvalidVectorLength
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

// VerificationError is a concrete error with location information.
type VerificationError struct {
	Kind   ErrorKind
	Field  string
	Offset int
	Trace  []TraceDetail
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

// Verifier validates FlatBuffer structural integrity.
type Verifier struct {
	buf       []byte
	opts      VerifierOptions
	depth     int
	numTables int
}

// NewVerifier creates a Verifier for the given buffer.
// If opts is nil or any field is zero, sensible defaults are applied.
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

// CheckAlignment verifies that pos is aligned to the given alignment.
func (v *Verifier) CheckAlignment(pos int, align int) error {
	if align > 1 && pos%align != 0 {
		return verifyErr(ErrUnaligned, "", pos)
	}
	return nil
}

// CheckRange verifies that [pos, pos+size) is within the buffer bounds
// and within MaxApparentSize.
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

// CheckUOffsetT reads a 4-byte little-endian uint32 at pos, verifies that the
// target address (pos + value) is also within the buffer, and returns the value.
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

// CheckSOffsetT reads a 4-byte little-endian int32 at pos, verifies that the
// signed-offset target is within buffer bounds, and returns the raw value.
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

// CheckVtable verifies the vtable structure at the table located at tablePos.
// It follows the SOffsetT backward to the vtable and validates the vtable size field.
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

// CheckTable verifies that the table at tablePos has a valid vtable and increments
// the table counter.
func (v *Verifier) CheckTable(tablePos int) error {
	if err := v.CountTable(); err != nil {
		return err
	}
	return v.CheckVtable(tablePos)
}

// CheckString verifies the string at pos: follows the UOffsetT indirection,
// validates UTF-8, and checks for the null terminator.
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

// CheckVector verifies the vector at pos and returns the element count.
// elementSize is the size in bytes of each element.
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

// CheckVectorOfTables verifies a vector of tables at pos.
// verifyElem is called for each element's table position.
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

// CheckScalarField verifies that the scalar field at vtable slot vOffset is in bounds.
// If the field is absent (vtable slot is zero), no error is returned (scalar default applies).
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

// CheckOffsetField looks up the offset field at vtable slot vOffset and returns the
// absolute buffer position of the referenced data, or 0 if the field is absent.
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

// CheckRequiredField returns an error if the field at vtable slot vOffset is absent.
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

// PushDepth increments the nesting depth counter and returns an error if the
// depth limit is exceeded.
func (v *Verifier) PushDepth() error {
	v.depth++
	if v.depth > v.opts.MaxDepth {
		return verifyErr(ErrDepthLimitReached, "", 0)
	}
	return nil
}

// PopDepth decrements the nesting depth counter.
func (v *Verifier) PopDepth() {
	if v.depth > 0 {
		v.depth--
	}
}

// CountTable increments the table counter and returns an error if the limit is exceeded.
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
