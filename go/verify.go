package flatbuffers

import (
	"reflect"
)

// Verifier options
type Options struct {
	/// Maximum depth of nested tables allowed in a valid flatbuffer.
	maxDepth int
	/// Maximum number of tables allowed in a valid flatbuffer.
	maxTables int
	/// Check that string contains its null terminator
	stringEndCheck bool
	/// Check alignment od elements
	alignmentCheck bool
}

// Verifier a is buffer related verification container.
// It holds verification options and processing statistics to calculate object complexity.
type Verifier struct {
	// Buffer to be verified
	Buf []byte

	// Beggining of flatbuffer data
	Pos UOffsetT

	// Verification options
	options Options

	// Counters
	depth     int
	numTables int
}

// Type used for size calculations
type USizeT uint64

// Type definition of function paramter used for table object verification
type VerifyTableFunc func(verifier *Verifier, pos UOffsetT) bool

// Type definition of function paramter used for union object verification
type VerifyUnionFunc func(verifier *Verifier, typeId byte, valueOffset UOffsetT) bool

const (
	FLATBUFFERS_MAX_BUFFER_SIZE USizeT = (USizeT(1) << 31) - 1
	DEFAULT_MAX_DEPTH                  = 64
	DEFAULT_MAX_TABLES                 = 1000000
)

// Get buffer identifier so it can be validated if neccessary
func getBufferIdentifier(buf []byte, sizePrefixed bool) []byte {
	pos := SizeUOffsetT
	if sizePrefixed {
		pos += SizeUOffsetT
	}
	return buf[pos : pos+fileIdentifierLength]
}

// Check if there is identifier in buffer
func bufferHasIdentifier(buf []byte, identifier []byte, sizePrefixed bool) bool {
	bufferIdentifier := getBufferIdentifier(buf, sizePrefixed)
	return reflect.DeepEqual(identifier, bufferIdentifier)
}

// Get UOffsetT from the begining of buffer - it must be verified before read
func (verifier *Verifier) readUOffsetT(buf []byte) UOffsetT {
	return GetUOffsetT(buf)
}

// Get SOffsetT from the begining of buffer - it must be verified before read
func (verifier *Verifier) readSOffsetT(buf []byte) SOffsetT {
	return GetSOffsetT(buf)
}

// Get VOffsetT from the begining of buffer - it must be verified before read
func (verifier *Verifier) readVOffsetT(buf []byte) VOffsetT {
	return GetVOffsetT(buf)
}

// Get table data area relative offset from vtable. Result is relative to a table start
// Fields which are deprecated are ignored by checking against the vtable's length.
func (verifier *Verifier) getVRelOffset(tablePos UOffsetT, vtableOffset VOffsetT) VOffsetT {
	// First, get vtable offset
	vtable := UOffsetT(SOffsetT(tablePos) - GetSOffsetT(verifier.Buf[tablePos:]))
	// Check that offset points to vtable area (is smaller than vtable size)
	if vtableOffset < GetVOffsetT(verifier.Buf[vtable:]) {
		// Now, we can read offset value - TODO check this value against size of table data
		return GetVOffsetT(verifier.Buf[vtable+UOffsetT(vtableOffset):])
	}
	return 0
}

// Get table data area absolute offset from vtable. Result is an absolute buffer position.
// The value offset cannot be 0 (pointing to itself) so after validation this method returnes 0
// value as a marker for missing optional entry
func (verifier *Verifier) getVOffset(tablePos UOffsetT, vtableOffset VOffsetT) UOffsetT {
	var offset UOffsetT
	// First, get vtable relative offset
	relOffset := verifier.getVRelOffset(tablePos, vtableOffset)
	if relOffset != 0 {
		// Calculate position based on table postion and offset
		offset = tablePos + UOffsetT(relOffset)
	} else {
		offset = 0
	}
	return offset
}

// Check flatbuffer complexity (tables depth, elements counter and so on)
// If complexity is too high function returns false for verification error
func (verifier *Verifier) checkComplexity() bool {
	return verifier.depth <= verifier.options.maxDepth && verifier.numTables <= verifier.options.maxTables
}

// Check aligment of an element.
func (verifier *Verifier) checkAlignment(pos UOffsetT, align USizeT) bool {
	return (USizeT(pos)&(align-1)) == 0 || !verifier.options.alignmentCheck
}

// Check if element is valid in the buffer area.
func (verifier *Verifier) checkElement(pos UOffsetT, elementSize USizeT) bool {
	return (elementSize < USizeT(len(verifier.Buf))) && (USizeT(pos) <= (USizeT(len(verifier.Buf)) - elementSize))
}

// Check if element is a valid scalar.
//
//	pos - position of scalar
func (verifier *Verifier) checkScalar(pos UOffsetT, scalarSize USizeT) bool {
	return verifier.checkAlignment(pos, scalarSize) && verifier.checkElement(pos, scalarSize)
}

// Check offset. It is a scalar with size of UOffsetT.
func (verifier *Verifier) checkOffset(pos UOffsetT) bool {
	return verifier.checkScalar(pos, SizeUOffsetT)
}

// Common verification code among vectors and strings.
func (verifier *Verifier) checkVectorOrString(pos UOffsetT, elementSize USizeT, end *UOffsetT) bool {
	// Check we can read the vector/string size field (it is of uoffset size).
	if !verifier.checkScalar(pos, SizeUOffsetT) {
		return false
	}
	// Check the whole array. If this is a string, the byte past the array
	// must be 0.
	size := USizeT(verifier.readUOffsetT(verifier.Buf[pos:]))
	maxElementsNo := FLATBUFFERS_MAX_BUFFER_SIZE / elementSize
	if size >= maxElementsNo {
		return false // Protect against byteSize overflowing.
	}
	byteSize := SizeUOffsetT + elementSize*size
	if end != nil {
		*end = pos + UOffsetT(byteSize)
	}
	return verifier.checkElement(pos, byteSize)
}

// Verify the string at given position.
func (verifier *Verifier) checkString(pos UOffsetT) bool {
	var end UOffsetT
	return verifier.checkVectorOrString(pos, 1, &end) &&
		verifier.checkScalar(end, 1) && // Must have terminator
		verifier.Buf[end] == 0 // Terminating byte must be 0.
}

// Verify the vector of data elements of given size
func (verifier *Verifier) checkVector(pos UOffsetT, elementSize USizeT) bool {
	return verifier.checkVectorOrString(pos, elementSize, nil)
}

// Verify table content using structure dependent generated function
func (verifier *Verifier) checkTable(tablePos UOffsetT, verifyObjFunc VerifyTableFunc) bool {
	return verifyObjFunc(verifier, tablePos)
}

// String check wrapper funnction to be used in vector of strings check
func checkStringFunc(verifier *Verifier, pos UOffsetT) bool {
	return verifier.checkString(pos)
}

// Check vector of objects. Use generated object verification function
func (verifier *Verifier) checkVectorOfObjects(pos UOffsetT, verifyObjFunc VerifyTableFunc) bool {
	result := verifier.checkVector(pos, SizeUOffsetT)
	if result {
		size := USizeT(verifier.readUOffsetT(verifier.Buf[pos:]))
		// Vector data starts just after vector size/length
		vectorStart := pos + SizeUOffsetT
		// Iterate offsets and verify referenced objects
		for i := USizeT(0); i < size && result; i++ {
			// get offset to vector item
			offsetPos := vectorStart + UOffsetT(i*SizeUOffsetT)
			// Check if the offset postition points to a valid offset
			result = verifier.checkIndirectOffset(offsetPos)
			if result {
				itemPos := verifier.getIndirectOffset(offsetPos)
				result = verifyObjFunc(verifier, itemPos)
			}
		}
	}
	return result
}

// Check if the offset referenced by pos is the valid offset pointing to buffer
//
//	pos - position of offset data
func (verifier *Verifier) checkIndirectOffset(pos UOffsetT) bool {
	var result bool
	// Check if the input position is valid
	result = verifier.checkScalar(pos, SizeUOffsetT)
	if result {
		// Get indirect offset
		offset := verifier.readUOffsetT(verifier.Buf[pos:])
		// May not point to itself neither wrap around  (buffers are max 2GB)
		if offset != 0 && USizeT(offset) < FLATBUFFERS_MAX_BUFFER_SIZE {
			// Must be inside the buffer
			result = verifier.checkElement(pos+offset, 1)
		} else {
			result = false
		}
	}
	return result
}

// Check flatbuffer content using generated object verification function
func (verifier *Verifier) checkBufferFromStart(identifier []byte, start UOffsetT, verifyObjFunc VerifyTableFunc) bool {
	var result bool
	if len(identifier) == 0 || (USizeT(len(verifier.Buf)) >= USizeT(verifier.Pos) && (USizeT(len(verifier.Buf))-USizeT(verifier.Pos)) >= (SizeUOffsetT+fileIdentifierLength) && bufferHasIdentifier(verifier.Buf[start:], identifier, false)) {
		result = verifier.checkIndirectOffset(start)
		if result {
			tablePos := verifier.getIndirectOffset(start)
			result = verifier.checkTable(tablePos, verifyObjFunc) //  && GetComputedSize()
		}
	} else {
		result = false
	}
	return result
}

// Get indirect offset. It is an offset referenced pos
func (verifier *Verifier) getIndirectOffset(pos UOffsetT) UOffsetT {
	// Get indirect offset referenced by pos
	offset := pos + verifier.readUOffsetT(verifier.Buf[pos:])
	return offset
}

// Verify beginning of table
//
// (this method is used internally by generated verification functions)
func (verifier *Verifier) VerifyTableStart(tablePos UOffsetT) bool {
	// Starting new table verification increases complexity of structure
	verifier.depth++
	verifier.numTables++

	if !verifier.checkScalar(tablePos, SizeSOffsetT) {
		return false
	}
	vtable := UOffsetT(int64(tablePos) - int64(verifier.readSOffsetT(verifier.Buf[tablePos:])))
	return verifier.checkComplexity() &&
		verifier.checkScalar(vtable, SizeVOffsetT) &&
		verifier.checkAlignment(UOffsetT(verifier.readVOffsetT(verifier.Buf[vtable:])), SizeVOffsetT) &&
		verifier.checkElement(vtable, USizeT(verifier.readVOffsetT(verifier.Buf[vtable:])))
}

// Verify end of table. In practice, this function does not check buffer but handles
// verification statistics update
//
// (this method is used internally by generated verification functions)
func (verifier *Verifier) VerifyTableEnd(tablePos UOffsetT) bool {
	verifier.depth--
	return true
}

// Verifiy static/inlined data area field
//
// (this method is used internally by generated verification functions)
func (verifier *Verifier) VerifyField(tablePos UOffsetT, offsetId VOffsetT, elementSize USizeT, align USizeT, required bool) bool {
	var result bool
	pos := verifier.getVOffset(tablePos, offsetId)
	if pos != 0 {
		result = verifier.checkAlignment(pos, align) && verifier.checkElement(pos, elementSize)
	} else {
		result = !required // it is OK if field is not required
	}
	return result
}

// Verify string
//
// (this method is used internally by generated verification functions)
func (verifier *Verifier) VerifyString(tablePos UOffsetT, offsetId VOffsetT, required bool) bool {
	var result bool

	pos := verifier.getVOffset(tablePos, offsetId)
	if pos != 0 {
		result = verifier.checkIndirectOffset(pos)
		if result {
			pos := verifier.getIndirectOffset(pos)
			result = verifier.checkString(pos)
		}
	} else {
		result = !required
	}
	return result
}

// Verify array of strings
//
// (this method is used internally by generated verification functions)
func (verifier *Verifier) VerifyVectorOfStrings(tablePos UOffsetT, offsetId VOffsetT, required bool) bool {
	var result bool

	pos := verifier.getVOffset(tablePos, offsetId)
	if pos != 0 {
		result = verifier.checkIndirectOffset(pos)
		if result {
			pos := verifier.getIndirectOffset(pos)
			result = verifier.checkVectorOfObjects(pos, checkStringFunc)
		}
	} else {
		result = !required
	}
	return result
}

// Verify vector of tables (objects). Tables are verified using generated verifyObjFunc
//
// (this method is used internally by generated verification functions)
func (verifier *Verifier) VerifyVectorOfTables(tablePos UOffsetT, offsetId VOffsetT, verifyObjFunc VerifyTableFunc, required bool) bool {
	var result bool

	pos := verifier.getVOffset(tablePos, offsetId)
	if pos != 0 {
		result = verifier.checkIndirectOffset(pos)
		if result {
			pos := verifier.getIndirectOffset(pos)
			result = verifier.checkVectorOfObjects(pos, verifyObjFunc)
		}
	} else {
		result = !required
	}
	return result
}

// Verify table object using generated verification function.
//
// (this method is used internally by generated verification functions)
func (verifier *Verifier) VerifyTable(tablePos UOffsetT, offsetId VOffsetT, verifyObjFunc VerifyTableFunc, required bool) bool {
	var result bool

	pos := verifier.getVOffset(tablePos, offsetId)
	if pos != 0 {
		result = verifier.checkIndirectOffset(pos)
		if result {
			pos = verifier.getIndirectOffset(pos)
			result = verifier.checkTable(pos, verifyObjFunc)
		}
	} else {
		result = !required
	}
	return result
}

// Verify vector of fixed size structures and scalars
//
// (this method is used internally by generated verification functions)
func (verifier *Verifier) VerifyVectorOfData(tablePos UOffsetT, offsetId VOffsetT, elementSize USizeT, required bool) bool {
	var result bool

	pos := verifier.getVOffset(tablePos, offsetId)
	if pos != 0 {
		result = verifier.checkIndirectOffset(pos)
		if result {
			pos := verifier.getIndirectOffset(pos)
			result = verifier.checkVector(pos, elementSize)
		}
	} else {
		result = !required
	}
	return result
}

// Verify nested buffer object. When verifyObjFunc is provided, it is used to verify object structure.
//
// (this method is used internally by generated verification functions)
func (verifier *Verifier) VerifyNestedBuffer(tablePos UOffsetT, offsetId VOffsetT, verifyObjFunc VerifyTableFunc, required bool) bool {
	var result bool

	pos := verifier.getVOffset(tablePos, offsetId)
	if pos != 0 {
		result = verifier.checkIndirectOffset(pos)
		if result {
			pos = verifier.getIndirectOffset(pos)
			result = verifier.checkVector(pos, SizeByte)
			if result && verifyObjFunc != nil {
				vectorLength := USizeT(verifier.readUOffsetT(verifier.Buf[pos:]))
				// Buffer begins after vector length
				vectorStart := pos + SizeUOffsetT
				nestedBuffer := verifier.Buf[vectorStart : vectorStart + UOffsetT(vectorLength)]
				nestedVerifier := &Verifier{Buf: nestedBuffer, Pos: 0, options: verifier.options}
				// There is no iternal identifier - use empty one
				var identifier []byte
				result = nestedVerifier.checkBufferFromStart(identifier, 0, verifyObjFunc)
			}
		}
	} else {
		result = !required
	}
	return result
}

// Method verifies union object using verification function
//
// (this method is used internally by generated verification functions)
func (verifier *Verifier) VerifyUnion(tablePos UOffsetT, typeIdVOffset VOffsetT, valueVOffset VOffsetT,
	verifyObjFunc VerifyUnionFunc, required bool) bool {
	var result bool

	// Check the union type id
	pos := verifier.getVOffset(tablePos, typeIdVOffset)
	if pos != 0 {
		result = verifier.checkAlignment(pos, SizeByte) && verifier.checkElement(pos, SizeByte)
		if result {
			// Take type id
			typeId := GetByte(verifier.Buf[pos:])
			// Check union data
			pos = verifier.getVOffset(tablePos, valueVOffset)
			if pos != 0 {
				result = verifier.checkIndirectOffset(pos)
				if result {
					// Take data position and validate union structure
					pos = verifier.getIndirectOffset(pos)
					result = verifyObjFunc(verifier, typeId, pos)
				}
			} else {
				// When value data is not present, allow union verification function to deal with illegal offset
				result = verifyObjFunc(verifier, typeId, UOffsetT(len(verifier.Buf)))
			}
		}
	} else {
		result = !required
	}
	return result
}

// Method verifies flatbuffer data using generated Table verification function.
// The data buffer is already provided when creating [Verifier] object (see [NewVerifier])
//
//   - identifier - the expected identifier of buffer data.
//     When empty identifier is provided the identifier validation is skipped.
//   - sizePrefixed - this flag should be true when buffer is prefixed with content size
//   - verifyObjFunc - function to be used for verification. This function is generated by compiler and included in each table definition file with name "<Tablename>Verify"
//
// Example:
//
//	/* Verify Monster table. Ignore buffer name and assume buffer does not contain data length prefix */
//	isValid := verifier.VerifyBuffer([]byte{}, false, MonsterVerify)
//
//	/* Verify Monster table. Buffer name is 'MONS' and contains data length prefix */
//	isValid := verifier.VerifyBuffer("MONS", true, MonsterVerify)
func (verifier *Verifier) VerifyBuffer(identifier []byte, sizePrefixed bool, verifyObjFunc VerifyTableFunc) bool {
	var start UOffsetT
	var result bool

	// Reset counters - starting verification from beginning
	verifier.depth = 0
	verifier.numTables = 0

	if sizePrefixed {
		start = verifier.Pos + sizePrefixLength
		result = verifier.checkScalar(verifier.Pos, sizePrefixLength)
		if result {
			size := USizeT(verifier.readUOffsetT(verifier.Buf[verifier.Pos:]))
			result = size == (USizeT(len(verifier.Buf)) - USizeT(start))
		}
	} else {
		result = true
		start = verifier.Pos
	}
	result = result && verifier.checkBufferFromStart(identifier, start, verifyObjFunc)
	return result
}

// Specify maximum depth of valid structure
func (verifier *Verifier) SetMaxDepth(value int) *Verifier {
	verifier.options.maxDepth = value
	return verifier
}

// Specify maximum number of tables in structure
func (verifier *Verifier) SetMaxTables(value int) *Verifier {
	verifier.options.maxTables = value
	return verifier
}

// Enable/disable buffer content alignment check
func (verifier *Verifier) SetAlignmentCheck(value bool) *Verifier {
	verifier.options.alignmentCheck = value
	return verifier
}

// Enable/disable checking of string termination '0' charakter
func (verifier *Verifier) SetStringCheck(value bool) *Verifier {
	verifier.options.stringEndCheck = value
	return verifier
}

// Create and initialize [Verifier] object that is then used by [Verifier.VerifyBuffer] method to verify flatbuffer data.
// Verifier checks data provided as buffer and optionally uses an offset of data position (default: 0).
//
// The created object is initialized with default verification options. They can be adjusted
// by appropriate fluent methods: SetMaxDepth, SetMaxTables, SetAlignmentCheck and SetStringCheck.
//
// Examples:
//
//	v := NewVerifier(flatbufferData).SetMaxTables(1234)
//	---
//	dataOffset := 456
//	v := NewVerifier(flatbufferData, dataOffset).SetMaxTables(1234)
func NewVerifier(buffer []byte, offset ...UOffsetT) *Verifier {
	verifier := new(Verifier)
	verifier.Buf = buffer
	if len(offset) > 0 {
		verifier.Pos = offset[0]
	} else {
		verifier.Pos = 0
	}
	verifier.SetMaxTables(DEFAULT_MAX_TABLES)
	verifier.SetMaxDepth(DEFAULT_MAX_DEPTH)
	verifier.SetStringCheck(true)
	verifier.SetAlignmentCheck(true)
	return verifier
}
