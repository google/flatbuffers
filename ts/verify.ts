import { FILE_IDENTIFIER_LENGTH, SIZEOF_INT, SIZEOF_SHORT, DEFAULT_MAX_DEPTH, DEFAULT_MAX_TABLES, FLATBUFFERS_MAX_BUFFER_SIZE, SIZE_PREFIX_LENGTH } from "./constants.js";
import { int32, isLittleEndian, float32, float64 } from "./utils.js";
import { Encoding } from "./encoding.js";
import { ByteBuffer } from "./byte-buffer.js";
import { Offset } from "./types.js";


// UOffset type definition for better type validation
export type UOffset = number;

// SOffset type definition for better type validation
export type SOffset = number;

// VOffset type definition for better type validation
export type VOffset = number;

// VOffset type definition for better type validation
type USize = number;

// Size of UOffset
const SizeUOffset = SIZEOF_INT

// Size of SOffset
const SizeSOffset = SIZEOF_INT

// Size of VOffset
const SizeVOffset = SIZEOF_SHORT

// Size of VOffset
const SizeByte = 1

// Type alias of function paramter used for table object verification
type VerifyTableFunc = (verifier: Verifier, tablePos: UOffset) => boolean;

// Type alias of function paramter used for union object verification
type VerifyUnionFunc = (verifier: Verifier, typeId: number, tablePos: UOffset) => boolean;


// Verifier options
export class Options {

    /**
     * Create verifier Options object providing following parameters:
     - maxDepth - maximum depth of nested tables allowed in a valid flatbuffer (DEFAULT_MAX_DEPTH)
     - maxTables - maximum number of tables allowed in a valid flatbuffer.
     - stringEndCheck - check that string contains its null terminator
     - alignmentCheck - check alignment of elements
    */
    constructor(public maxDepth : number = DEFAULT_MAX_DEPTH,
                public maxTables : number = DEFAULT_MAX_TABLES,
                public stringEndCheck : boolean = true,
                public alignmentCheck : boolean = true) { }
}


/**
 * Verifier is a buffer related verification container.
 * It holds verification options and processing statistics to calculate object complexity.
 */
export class Verifier {
    private buf : ByteBuffer;
    private options : Options;
    private depth : number;
    private numTables : number;
  
    /**
     * Create a new Verifier with given buffer (`ByteBuffer`)
     */
    constructor(bytes: ByteBuffer, options : Options = new Options()) {
        this.buf = bytes;
        this.options = options
        this.depth = 0;
        this.numTables = 0;
    }

    
    /**
     * Check if there is identifier in the buffer
     */
    bufferHasIdentifier(buf : ByteBuffer, start : Offset, identifier : string) : boolean {
        if (identifier.length != FILE_IDENTIFIER_LENGTH) {
            throw new Error('FlatBuffers: file identifier must be of length ' + FILE_IDENTIFIER_LENGTH);
        }
        for (let i = 0; i < FILE_IDENTIFIER_LENGTH; i++) {
            if (identifier.charCodeAt(i) != this.buf.readInt8(SIZEOF_INT + i + start)) {
                return false;
            }
        }
        return true;
    }

    
    // Get UOffsetT from the buffer - position must be verified before read
    readUOffsetT(buf: ByteBuffer, pos: Offset): UOffset {
        return buf.readUint32(pos)
    }

    // Get SOffsetT from the buffer - position it must be verified before read
    readSOffsetT(buf: ByteBuffer, pos: Offset): SOffset {
        return buf.readInt32(pos)
    }

    // Get VOffsetT from the buffer - position it must be verified before read
    readVOffsetT(buf: ByteBuffer, pos: Offset): VOffset {
        return buf.readUint16(pos)
    }

    /**
     * Get table data area relative offset from vtable. Result is relative to a table start
     * Fields which are deprecated are ignored by checking against the vtable's length.
     */
    getVRelOffset(tablePos : UOffset, vtableOffset : VOffset): VOffset {
        // First, get vtable offset
        const vtable: VOffset = tablePos - this.readSOffsetT(this.buf, tablePos)
        // Check that offset points to vtable area (is smaller than vtable size)
        if (vtableOffset < this.readVOffsetT(this.buf, vtable)) {
            // Now, we can read offset value - TODO check this value against size of table data
            return this.readVOffsetT(this.buf, vtable + vtableOffset)
        }
        return 0
    }

    // Get table data area absolute offset from vtable. Result is an absolute buffer position.
    // The value offset cannot bo 0 (pointing to itself) so after validation this method returnes 0
    // value as a marker for missing optional entry
    getVOffset(tablePos: UOffset, vtableOffset: VOffset): UOffset {
        let offset: UOffset
        // First, get vtable relative offset
        const relOffset: VOffset = this.getVRelOffset(tablePos, vtableOffset)
        if (relOffset != 0) {
            // Calculate position based on table postion and offset
            offset = tablePos + relOffset
        } else {
            offset = 0
        }
        return offset
    }

    // Check flatbuffer complexity (tables depth, elements counter and so on)
    // If complexity is too high function returns false for verification error
    checkComplexity(): boolean {
        return this.depth <= this.options.maxDepth && this.numTables <= this.options.maxTables
    }

    // Check aligment of an element.
    checkAlignment(pos: UOffset, align: USize): boolean {
        return (pos & (align-1)) == 0 || !this.options.alignmentCheck
    }

    // Check if element is valid in the buffer area.
    checkElement(pos: UOffset, elementSize: USize): boolean {
        return (elementSize < this.buf.capacity()) && (pos <= (this.buf.capacity() - elementSize));
    }

    // Check if element is a valid scalar.
    //
    //  pos - position of scalar
    checkScalar(pos: UOffset, elementSize: USize): boolean {
        return this.checkAlignment(pos, elementSize) && this.checkElement(pos, elementSize)
    }

    // Check offset. It is a scalar with size of UOffsetT.
    checkOffset(pos: UOffset): boolean {
        return this.checkScalar(pos, SizeUOffset)
    }

    // Common verification code among vectors and strings.
    checkVectorOrString(pos: UOffset, elementSize: USize): [boolean, Offset] {
        // Check we can read the vector/string size field (it is of uoffset size).
        if (! this.checkScalar(pos, SizeUOffset)) {
            return [false, 0]
        }
        // Check the whole array. If this is a string, the byte past the array
        // must be 0.
        const size = this.readUOffsetT(this.buf, pos);
        const maxElementsNo = FLATBUFFERS_MAX_BUFFER_SIZE / elementSize;
        if (size >= maxElementsNo) {
            return [false, 0] // Protect against byteSize overflowing.
        }
        const byteSize = SizeUOffset + elementSize*size
        const end = pos + byteSize
        return [this.checkElement(pos, byteSize), end]
    }

    // Verify the string at given position.
    checkString(pos: UOffset): boolean {
        let [result, end] = this.checkVectorOrString(pos, 1)
        if (this.options.stringEndCheck) {
            result = result && this.checkScalar(end, 1);  // Must have terminator
            result = result && this.buf.readInt8(end) == 0; // Terminating byte must be 0.
        }
        return result;
    }

    // Verify the vector of data elements of given size
    checkVector(pos: UOffset, elementSize: USize): boolean {
        const [result, end] = this.checkVectorOrString(pos, elementSize);
        return result;
    }

    // Verify table content using structure dependent generated function
    checkTable(tablePos: UOffset, verifyObjFunc: VerifyTableFunc): boolean {
        return verifyObjFunc(this, tablePos)
    }

    // String check wrapper funnction to be used in vector of strings check
    checkStringFunc(verifier: Verifier, pos: UOffset): boolean {
        return verifier.checkString(pos)
    }

    // Check vector of objects. Use generated object verification function
    checkVectorOfObjects(pos: UOffset, verifyObjFunc: VerifyTableFunc): boolean {
        let result = this.checkVector(pos, SizeUOffset)
        if (result) {
            const size =  this.readUOffsetT(this.buf, pos)
            // Vector data starts just after vector size/length
            const vectorStart = pos + SizeUOffset
            // Iterate offsets and verify referenced objects
            for (let i = 0; i < size && result; i++) {
                // get offset to vector item
                const offsetPos = vectorStart + i * SizeUOffset;
                // Check if the offset postition points to a valid offset
                result = this.checkIndirectOffset(offsetPos);
                if (result) {
                    const itemPos = this.getIndirectOffset(offsetPos);
                    result = verifyObjFunc(this, itemPos);
                }
            }
        };
        return result;
    }

    // Check if the offset referenced by pos is the valid offset pointing to buffer
    //    pos - position of offset data
    checkIndirectOffset(pos: UOffset): boolean {
        let result: boolean
        // Check the input offset is valid
        result = this.checkScalar(pos, SizeUOffset);
        if (result) {
            // Get indirect offset
            const offset = this.readUOffsetT(this.buf, pos)
            // May not point to itself neither wrap around  (buffers are max 2GB)
            if (offset != 0 && offset < FLATBUFFERS_MAX_BUFFER_SIZE) {
                // Must be inside the buffer
                result = this.checkElement(pos + offset, 1)
            } else {
                result = false
            }
        }
        return result
    }
    
    // Check flatbuffer content using generated object verification function
    checkBufferFromStart(identifier: string | null, start: UOffset, verifyObjFunc: VerifyTableFunc): boolean {
        let result: boolean
        if (identifier == null ||
            identifier.length == 0 ||
            (this.buf.position() <= this.buf.capacity() && (this.buf.capacity() - this.buf.position()) >= (SizeUOffset + FILE_IDENTIFIER_LENGTH) && this.bufferHasIdentifier(this.buf, start, identifier))) {
            result = this.checkIndirectOffset(start)
            if (result) {
                const tablePos = this.getIndirectOffset(start)
                result = this.checkTable(tablePos, verifyObjFunc) //  && GetComputedSize()
            }
        } else {
            result = false
        }
        return result
    }

    // Get indirect offset. It is an offset referenced by pos
    getIndirectOffset(pos: UOffset): UOffset {
        // Get indirect offset referenced by offsetPos
        const offset = pos + this.readUOffsetT(this.buf, pos)
        return offset
    }
    
    // Verify beginning of table
    //
    // (this method is used internally by generated verification functions)
    verifyTableStart(tablePos: UOffset): boolean {
        // Starting new table verification increases complexity of structure
        this.depth++;
        this.numTables++;
        
        if (!this.checkScalar(tablePos, SizeSOffset)) {
            return false;
        };
        const vtable = tablePos - this.readSOffsetT(this.buf, tablePos)
        return this.checkComplexity() &&
            this.checkScalar(vtable, SizeVOffset) &&
            this.checkAlignment(this.readVOffsetT(this.buf, vtable), SizeVOffset) &&
            this.checkElement(vtable, this.readVOffsetT(this.buf, vtable));
    }

    
    // Verify end of table. In practice, this function does not check buffer but handles
    // verification statistics update
    //
    // (this method is used internally by generated verification functions)
    verifyTableEnd(tablePos: UOffset): boolean {
        this.depth--
        return true
    }

    // Verifiy static/inlined data area field
    //
    // (this method is used internally by generated verification functions)
    verifyField(tablePos: UOffset, offsetId: VOffset, elementSize: USize, align: USize, required: boolean): boolean {
        let result:  boolean
        const pos = this.getVOffset(tablePos, offsetId)
        if (pos != 0) {
            result = this.checkAlignment(pos, align) && this.checkElement(pos, elementSize)
        } else {
            result = !required // it is OK if field is not required
        }
        return result
    }

    // Verify string
    //
    // (this method is used internally by generated verification functions)
    verifyString(tablePos: UOffset, offsetId: VOffset, required: boolean): boolean {
        let result: boolean
        
        let pos = this.getVOffset(tablePos, offsetId)
        if (pos != 0) {
            result = this.checkIndirectOffset(pos)
            if (result) {
                pos = this.getIndirectOffset(pos)
                result = this.checkString(pos)
            }
        } else {
            result = !required
        }
        return result
    }

    // Verify array of strings
    //
    // (this method is used internally by generated verification functions)
    verifyVectorOfStrings(tablePos: UOffset, offsetId: VOffset, required: boolean): boolean {
        let result: boolean;
        
        let pos = this.getVOffset(tablePos, offsetId);
        if (pos != 0) {
            result = this.checkIndirectOffset(pos)
            if (result) {
                pos = this.getIndirectOffset(pos)
                result = this.checkVectorOfObjects(pos, this.checkStringFunc)
            }
        } else {
            result = !required
        }
        return result
    }
    

    // Verify vector of tables (objects). Tables are verified using generated verifyObjFunc
    //
    // (this method is used internally by generated verification functions)
    verifyVectorOfTables(tablePos: UOffset, offsetId: VOffset, verifyObjFunc: VerifyTableFunc, required: boolean): boolean {
        let result: boolean

        let pos = this.getVOffset(tablePos, offsetId)
        if (pos != 0) {
            result = this.checkIndirectOffset(pos)
            if (result) {
                pos = this.getIndirectOffset(pos)
                result = this.checkVectorOfObjects(pos, verifyObjFunc)
            }
        } else {
            result = !required
        }
        return result
    }

    
    // Verify table object using generated verification function.
    //
    // (this method is used internally by generated verification functions)
    verifyTable(tablePos: UOffset, offsetId: VOffset, verifyObjFunc: VerifyTableFunc, required: boolean): boolean {
        let result: boolean;

        let pos = this.getVOffset(tablePos, offsetId);
        if (pos != 0) {
            result = this.checkIndirectOffset(pos);
            if (result) {
                pos = this.getIndirectOffset(pos);
                result = this.checkTable(pos, verifyObjFunc);
            }
        } else {
            result = !required
        }
        return result
    }

    // Verify vector of fixed size structures and scalars
    //
    // (this method is used internally by generated verification functions)
    verifyVectorOfData(tablePos: UOffset, offsetId: VOffset, elementSize: USize, required: boolean): boolean {
        let result: boolean;

        let pos = this.getVOffset(tablePos, offsetId);
        if (pos != 0) {
            result = this.checkIndirectOffset(pos);
            if (result) {
                pos = this.getIndirectOffset(pos);
                result = this.checkVector(pos, elementSize);
            }
        } else {
            result = !required
        }
        return result
    }

    // Verify nested buffer object. When verifyObjFunc is provided it is used to verify object structure.
    //
    // (this method is used internally by generated verification functions)
    verifyNestedBuffer(tablePos: UOffset, offsetId: VOffset, verifyObjFunc: VerifyTableFunc | null, required: boolean): boolean {
        let result: boolean
        
        let pos = this.getVOffset(tablePos, offsetId);
        if (pos != 0) {
            result = this.checkIndirectOffset(pos);
            if (result) {
                pos = this.getIndirectOffset(pos);
                result = this.checkVector(pos, SizeByte);
                if (result && verifyObjFunc != null) {
                    const vectorLength = this.readUOffsetT(this.buf, pos);
                    // Buffer begins after vector length
                    const vectorStart = pos + SizeUOffset;
                    const nestedBufferBytes = this.buf.bytes().slice(vectorStart, vectorStart + vectorLength)
                    const nestedBuffer = new ByteBuffer(nestedBufferBytes)
                    const nestedVerifier = new Verifier(nestedBuffer, this.options)
                    // There is no iternal identifier - use empty one
                    result = nestedVerifier.checkBufferFromStart("", 0, verifyObjFunc)
                }
            }
        } else {
            result = !required
        }
        return result
    }

    // Verifiy union member static/inlined data area
    //
    // (this method is used internally by generated verification functions)
    verifyUnionData(pos: UOffset, elementSize: USize, align: USize): boolean {
        const result = this.checkAlignment(pos, align) && this.checkElement(pos, elementSize)
        return result
    }

    // Verify union memeber string
    //
    // (this method is used internally by generated verification functions)
    verifyUnionString(pos: UOffset): boolean {
        const result = this.checkString(pos)
        return result
    }

    // Method verifies union object using generated verification function
    //
    // (this method is used internally by generated verification functions)
    verifyUnion(tablePos: UOffset, typeIdVOffset: VOffset, valueVOffset: VOffset, verifyObjFunc: VerifyUnionFunc, required: boolean): boolean {
        let result: boolean
        
        // Check the union type id
        let pos = this.getVOffset(tablePos, typeIdVOffset)
        if (pos != 0) {
            result = this.checkAlignment(pos, SizeByte) && this.checkElement(pos, SizeByte)
            if (result) {
			    // Take type id
                const typeId = this.buf.readUint8(pos);
                // Check union data
                pos = this.getVOffset(tablePos, valueVOffset)
                if (pos != 0) {
                    result = this.checkIndirectOffset(pos)
                    if (result) {
                        // Take data position and validate union structure
                        pos = this.getIndirectOffset(pos)
                        result = verifyObjFunc(this, typeId, pos)
                    }
                } else {
				    // When value data is not present, allow union verification function to deal with illegal pos
                    result = verifyObjFunc(this, typeId, this.buf.capacity())
                }
            }
        } else {
            result = !required
        }
        return result
    }
    
    // Verify vector of unions (objects). Unions are verified using generated verifyObjFunc
    //
    // (this method is used internally by generated verification functions)
    verifyVectorOfUnions(tablePos: UOffset, typeOffsetId: VOffset, offsetId: VOffset, verifyObjFunc: VerifyUnionFunc, required: boolean): boolean {
        let result = true;
        let pos: UOffset;
        // type id pos must be valid
        pos = this.getVOffset(tablePos, typeOffsetId);
        if (pos != 0) {
            result = this.checkIndirectOffset(pos);
            if (result) {
                // Get type id table absolute pos
                const typeIdVectorPos = this.getIndirectOffset(pos);
                // values pos must be valid
                pos = this.getVOffset(tablePos, offsetId);
                result = this.checkIndirectOffset(pos);
                if (result) {
                    const valueVectorPos = this.getIndirectOffset(pos);
                    // validate referenced vectors
                    result = result && this.checkVector(typeIdVectorPos, SizeByte);
                    result = result && this.checkVector(valueVectorPos, SizeUOffset);
                    if (result) {
                        // Both vectors should have the same length
                        const typeIdVectorLength = this.readUOffsetT(this.buf, typeIdVectorPos);
                        const valueVectorLength = this.readUOffsetT(this.buf, valueVectorPos);
                        if (typeIdVectorLength == valueVectorLength) {
                            // Verify each union item from vector by loping over type id and value data offsets
                            const typeIdStart = typeIdVectorPos + SizeUOffset;
                            const valueStart = valueVectorPos + SizeUOffset;
                            for (let i = 0; result && (i < typeIdVectorLength); i++) {
                                // Get type id
                                const typeId = this.buf.readUint8(typeIdStart + i * SizeUOffset);
                                // get position of vector item offset
                                const itemOffsetPos = valueStart + i * SizeUOffset;
                                // Check the vector item has a proper offset
                                result = this.checkIndirectOffset(itemOffsetPos);
                                if (result) {
                                    const itemPos = this.getIndirectOffset(itemOffsetPos);
                                    // Verify object
                                    result = verifyObjFunc(this, typeId, itemPos);
                                }
                            }
                        } else {
                            result = false;
                        }
                    }
                }
            }
        } else {
            result = ! required;
        }
        return result;
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
    //  /* Verify Monster table. Ignore buffer name and assume buffer does not contain data length prefix */
    //  isValid = verifier.verifyBuffer(bb, false, MonsterVerify)
    //
    //  /* Verify Monster table. Buffer name is 'MONS' and contains data length prefix */
    //  isValid = verifier.verifyBuffer("MONS", true, MonsterVerify)
    verifyBuffer(identifier: string | null, sizePrefixed: boolean, verifyObjFunc: VerifyTableFunc): boolean {
        let start: UOffset;
        let result: boolean;
        
        // Reset counters - starting verification from beginning
        this.depth = 0;
        this.numTables = 0;
        
        if (sizePrefixed) {
            start = this.buf.position() + SIZE_PREFIX_LENGTH;
            result = this.checkScalar(0, SIZE_PREFIX_LENGTH);
            if (result) {
                const size = this.readUOffsetT(this.buf, this.buf.position());
                result = size == (this.buf.capacity() - start);
            }
        } else {
            result = true
            start = this.buf.position()
        }
        result = result && this.checkBufferFromStart(identifier, start, verifyObjFunc)
        return result
    }

    // Specify maximum depth of valid structure
    setMaxDepth(value: number): Verifier {
        this.options.maxDepth = value
        return this
    }
    
    // Specify maximum number of tables in structure
    setMaxTables(value: number): Verifier {
        this.options.maxTables = value
        return this
    }

    // Enable/disable buffer content alignment check
    setAlignmentCheck(value: boolean): Verifier {
        this.options.alignmentCheck = value
        return this
    }

    // Enable/disable checking of string termination '0' charakter
    setStringCheck(value: boolean): Verifier {
        this.options.stringEndCheck = value
        return this
    }

}


// Create and initialize [Verifier] object that is then used by [Verifier.VerifyBuffer] method to verify flatbuffer data.
// Verifier checks data contained in a buffer. The [ByteBuffer] data offset is used to get head of data.
// 
// The created object is initialized with default verification options. They can be adjusted
// by appropriate fluent methods: SetMaxDepth, SetMaxTables, SetAlignmentCheck and SetStringCheck.
//
// Examples:
//
//  let v = newVerifier(flatbufferData).setMaxTables(1234)
//  --- 
//  let dataOffset = 456
//  let v = newVerifier(flatbufferData, dataOffset).setMaxTables(1234)
export function newVerifier(buf: ByteBuffer) : Verifier{
    const verifier = new Verifier(buf)
    verifier.setMaxTables(DEFAULT_MAX_TABLES)
    verifier.setMaxDepth(DEFAULT_MAX_DEPTH)
    verifier.setStringCheck(true)
    verifier.setAlignmentCheck(true)
    return verifier
}
