# Copyright 2014 Google Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from . import encode
from . import number_types as N


# Default values 
FLATBUFFERS_MAX_BUFFER_SIZE = N.Int32Flags.max_val
DEFAULT_MAX_DEPTH  = 64
DEFAULT_MAX_TABLES = 1000000


# Verifier options
class Options:

    def __init__(self):
        # Maximum depth of nested tables allowed in a valid flatbuffer.
        self.maxDepth = DEFAULT_MAX_DEPTH
        # Maximum number of tables allowed in a valid flatbuffer.
        self.maxTables = DEFAULT_MAX_TABLES
        # Check that string contains its null terminator
        self.stringEndCheck = True
        # Check alignment od elements
        self.alignmentCheck = True


# Verifier is a buffer related verification container.
# It holds verification options and processing statistics to calculate object complexity.
class Verifier:
    __slots__ = ("Buf", "Pos", "options", "depth", "numTables")
    
    def __init__(self, buf, offset = 0, options = Options()):
        # Preserve buffer to be verified
        self.Buf = buf
        self.Pos = offset
        
        # Assign verification options 
        self.options = options

        # Counters
        self.depth = 0
        self.numTables = 0


    def Get(self, flags, Bytes, off):
        """
        Get retrieves a value of the type specified by `flags`  at the
        given offset.
        """
        N.enforce_number(off, N.UOffsetTFlags)
        return flags.py_type(encode.Get(flags.packer_type, Bytes, off))

    
    def getBufferIdentifier(self, buf, start):
        """
        Get buffer identifier so it can be validated if neccessary
        """
        pos = start + N.UOffsetTFlags.bytewidth
        return buf[pos : pos + encode.FILE_IDENTIFIER_LENGTH]


    def bufferHasIdentifier(self, buf, start, identifier):
        """
        Check if there is identifier in buffer
        """
        bufferIdentifier = self.getBufferIdentifier(buf, start)
        return identifier == bufferIdentifier


    def readUOffsetT(self, buf, pos):
        """
        Get UOffsetT from the buffer - position must be verified before read
        """
        return self.Get(N.UOffsetTFlags, buf, pos)

    def readSOffsetT(self, buf, pos):
        """
        Get SOffsetT from the buffer - position must be verified before read
        """
        return self.Get(N.SOffsetTFlags, buf, pos)


    def readVOffsetT(self, buf, pos):
        """ 
        Get VOffsetT from the buffer - position must be verified before read
        """
        return self.Get(N.VOffsetTFlags, buf, pos)

    
    def readByte(self, buf, pos):
        """ 
        Get VOffsetT from given the buffer - position must be verified before read
        """
        return self.Get(N.Uint8Flags, buf, pos)

    
    def getVRelOffset(self, tablePos, vtableOffset):
        """
        Get table data area relative offset from vtable. Result is relative to table start
        Fields which are deprecated are ignored by checking against the vtable's length.
        """
        # First, get vtable offset
        vtable = tablePos - self.readSOffsetT(self.Buf, tablePos)
        # Check that offset points to vtable area (is smaller than vtable size)
        if vtableOffset < self.readVOffsetT(self.Buf, vtable):
            # Now, we can read offset value - TODO check this value against size of table data
            return self.readVOffsetT(self.Buf, vtable + vtableOffset)
        return 0


    def getVOffset(self, tablePos, vtableOffset):
        """
        Get table data area absolute offset from vtable. Result is an absolute buffer position.
        The value offset cannot bo 0 (pointing to itself) so after validation this method returnes 0
        value as a marker for missing optional entry
        """
        # First, get vtable relative offset
        relOffset = self.getVRelOffset(tablePos, vtableOffset)
        if relOffset != 0:
            # Calculate position based on table postion and offset
            offset = tablePos + relOffset
        else:
            offset = 0
            
        return offset


    def checkComplexity(self):
        """
        Check flatbuffer complexity (tables depth, elements counter and so on)
        If complexity is too high function returns False for verification error
        """
        return self.depth <= self.options.maxDepth and self.numTables <= self.options.maxTables


    def checkAlignment(self, pos, align):
        """
        Check aligment of an element.
        """
        return (pos & (align - 1)) == 0 or not self.options.alignmentCheck


    def checkElement(self, pos, elementSize):
        """
        Check if element is valid in the buffer area.
        """
        return (elementSize < len(self.Buf)) and (pos <= (len(self.Buf) - elementSize))


    def checkScalar(self, pos, elementSize):
        """
        Check if element is a valid scalar.
           elem - posiution of scalar
        """
        return self.checkAlignment(pos, elementSize) and self.checkElement(pos, elementSize)


    def checkOffset(self, pos):
        """
        Check offset. It is a scalar with size of UOffsetT.
        """
        return self.checkScalar(pos, N.UOffsetTFlags.bytewidth)


    def checkVectorOrString(self, pos, elementSize):
        """ 
        Common verification code among vectors and strings.
        """
        # Check we can read the vector/string size field (it is of uoffset size).
        if not self.checkScalar(pos, N.UOffsetTFlags.bytewidth):
            return False, 0

        # Check the whole array. If this is a string, the byte past the array
        # must be 0.
        size = self.readUOffsetT(self.Buf, pos)
        maxElementsNo = FLATBUFFERS_MAX_BUFFER_SIZE / elementSize
        if size >= maxElementsNo:
            return False, 0

        byteSize = N.UOffsetTFlags.bytewidth + elementSize * size
        endPos = pos + byteSize
        return self.checkElement(pos, byteSize), endPos


    def checkString(self, pos):
        """
        Verify the string at given position.
        """
        result, end = self.checkVectorOrString(pos, N.Uint8Flags.bytewidth)
        if self.options.stringEndCheck:
            result = result and self.checkScalar(end, 1)  # Must have terminator
            result = result and self.Buf[end] == 0  # Terminating byte must be 0.
        return result


    def checkVector(self, pos, elementSize):
        """
        Verify the vector of data elements of given size
        """
        result, _ = self.checkVectorOrString(pos, elementSize)
        return result


    def checkTable(self, tablePos, verifyObjFunc):
        """
        Verify table content using structure dependent generated function
        """
        return verifyObjFunc(self, tablePos)


    def checkVectorOfObjects(self, pos, verifyObjFunc):
        """
        Check vector of objects. Use generated object verification function
        """
        result = self.checkVector(pos, N.UOffsetTFlags.bytewidth)
        if result:
            size = self.readUOffsetT(self.Buf, pos)
            # Vector data starts just after vector size/length
            vectorStart = pos + N.UOffsetTFlags.bytewidth
            # Iterate offsets and verify referenced objects
            i = 0
            while i < size and result:
                # get offset to vector item
                offsetPos = vectorStart + i * N.UOffsetTFlags.bytewidth
                # Check if the offset postition points to a valid offset
                result = self.checkIndirectOffset(offsetPos)
                if result:
                    itemPos = self.getIndirectOffset(offsetPos)
                    result = verifyObjFunc(self, itemPos)
                i += 1
        return result

    
    def checkIndirectOffset(self, pos):
        """
        Check if the offset referenced by pos is the valid offset pointing to buffer
            pos - position of offset data
        """
        #Check the input offset is valid
        result = self.checkScalar(pos, N.UOffsetTFlags.bytewidth)
        if result:
            # Get indirect offset
            offset = self.readUOffsetT(self.Buf, pos)
            # May not point to itself neither wrap around  (buffers are max 2GB)
            if offset != 0 and offset < FLATBUFFERS_MAX_BUFFER_SIZE:
                # Must be inside the buffer
                result = self.checkElement(pos + offset, 1)
            else:
                result = False
        return result


    def checkBufferFromStart(self, identifier, start, verifyObjFunc):
        """
        Check flatbuffer content using generated object verification function
        """
        if identifier is None or len(identifier) == 0 or ((len(self.Buf) >= self.Pos) and (len(self.Buf) - self.Pos) >= (N.UOffsetTFlags.bytewidth + encode.FILE_IDENTIFIER_LENGTH) and self.bufferHasIdentifier(self.Buf, start, identifier)):
            # Call T::Verify, which must be in the generated code for this type.
            result = self.checkIndirectOffset(start)
            if result :
                tablePos = self.getIndirectOffset(start)
                result = self.checkTable(tablePos, verifyObjFunc) 
        else:
            result = False
        return result

    
    def getIndirectOffset(self, pos):
        """
        Get indirect offset. It is an offset referenced by pos
        """
        offset = pos + self.readUOffsetT(self.Buf, pos)
        return offset


      
    def VerifyTableStart(self, tablePos):
        """
        Verify beginning of table
        
        (this method is used internally by generated verification functions)
        """
        # Starting new table verification increases complexity of structure
        self.depth += 1
        self.numTables += 1

        if not self.checkScalar(tablePos, N.SOffsetTFlags.bytewidth):
            return False

        vtable = tablePos - self.readSOffsetT(self.Buf, tablePos)
        return (self.checkComplexity() and
                self.checkScalar(vtable, N.VOffsetTFlags.bytewidth) and
                self.checkAlignment(self.readVOffsetT(self.Buf, vtable), N.VOffsetTFlags.bytewidth) and
                self.checkElement(vtable, self.readVOffsetT(self.Buf, vtable)))

                
    def VerifyTableEnd(self, tablePos):
        """
        Verify end of table. In practice, this function does not check buffer but handles
        verification statistics update
        
        (this method is used internally by generated verification functions)
        """
        self.depth -= 1
        return True


    def VerifyField(self, tablePos, offsetId, elementSize, align, required):
        """
        Verifiy static/inlined data area field
        
        (this method is used internally by generated verification functions)
        """
        pos = self.getVOffset(tablePos, offsetId)
        if pos != 0:
            result = self.checkAlignment(pos, align) and self.checkElement(pos, elementSize)
        else:
            result = not required # it is OK if field is not required
        return result


    def VerifyString(self, tablePos, offsetId, required):
        """
        Verify string
        
        (this method is used internally by generated verification functions)
        """
        pos = self.getVOffset(tablePos, offsetId)
        if pos != 0:
            result = self.checkIndirectOffset(pos)
            if result:
                pos = self.getIndirectOffset(pos)
                result = self.checkString(pos)
        else:
            result = not required
        return result


    def VerifyVectorOfStrings(self, tablePos, offsetId, required):
        """
        Verify array of strings
        
        (this method is used internally by generated verification functions)
        """

        pos = self.getVOffset(tablePos, offsetId)
        if pos != 0:
            result = self.checkIndirectOffset(pos)
            if result:
                pos = self.getIndirectOffset(pos)
                result = self.checkVectorOfObjects(pos, checkStringFunc)
        else:
            result = not required
        return result


    def VerifyVectorOfTables(self, tablePos, offsetId, verifyObjFunc, required):
        """
        Verify vector of tables (objects). Tables are verified using generated verifyObjFunc
        
        (this method is used internally by generated verification functions)
        """

        pos = self.getVOffset(tablePos, offsetId)
        if pos != 0:
            result = self.checkIndirectOffset(pos)
            if result:
                pos = self.getIndirectOffset(pos)
                result = self.checkVectorOfObjects(pos, verifyObjFunc)
        else:
            result = not required
        return result


    def VerifyTable(self, tablePos, offsetId, verifyObjFunc, required):
        """
        Verify table object using generated verification function.
        
        (this method is used internally by generated verification functions)
        """

        pos = self.getVOffset(tablePos, offsetId)
        if pos != 0:
            result = self.checkIndirectOffset(pos)
            if result:
                pos = self.getIndirectOffset(pos)
                result = self.checkTable(pos, verifyObjFunc)
        else:
            result = not required
        return result


    def VerifyVectorOfData(self, tablePos, offsetId, elementSize, required):
        """
        Verify vector of fixed size structures and scalars
        
        (this method is used internally by generated verification functions)
        """

        pos = self.getVOffset(tablePos, offsetId)
        if pos != 0:
            result = self.checkIndirectOffset(pos)
            if result:
                pos = self.getIndirectOffset(pos)
                result = self.checkVector(pos, elementSize)
        else:
            result = not required
        return result


    def VerifyNestedBuffer(self, tablePos, offsetId, verifyObjFunc, required):
        """
        Verify nested buffer object. When verifyObjFunc is provided, it is used to verify object structure.
        
        (this method is used internally by generated verification functions)
        """

        pos = self.getVOffset(tablePos, offsetId)
        if pos != 0:
            result = self.checkIndirectOffset(pos)
            if result:
                pos = self.getIndirectOffset(pos)
                result = self.checkVector(pos, N.Uint8Flags.bytewidth)
                if result and verifyObjFunc != None:
                    vectorLength = self.readUOffsetT(self.Buf, pos)
                    # Buffer begins after vector length
                    vectorStart = pos + N.UOffsetTFlags.bytewidth
                    nestedBuffer = self.Buf[vectorStart : vectorStart + vectorLength]
                    nestedVerifier = Verifier(nestedBuffer, 0, self.options)
                    # nestedVerifier = Verifier(self.Buf, vectorStart, vectorStart + vectorLength, self.options)
                    # There is no iternal identifier - use empty one
                    result = nestedVerifier.checkBufferFromStart("", 0, verifyObjFunc)

        else:
            result = not required
        return result


    def VerifyUnion(self, tablePos, typeIdVOffset, valueVOffset, verifyObjFunc, required):
        """
        Method verifies union object using verification function
        
        (this method is used internally by generated verification functions)
        """
        # Check the union type id
        pos = self.getVOffset(tablePos, typeIdVOffset)
        if pos != 0:
            result = self.checkAlignment(pos, N.Uint8Flags.bytewidth) and self.checkElement(pos, N.Uint8Flags.bytewidth)
            if result:
                # Take type id
                typeId = self.readByte(self.Buf, pos)
                # Check union data
                pos = self.getVOffset(tablePos, valueVOffset)
                if pos != 0:
                    result = self.checkIndirectOffset(pos)
                    if result:
                        # Take data position and validate union structure
                        pos = self.getIndirectOffset(pos)
                        result = verifyObjFunc(self, typeId, pos)
                else:
                    # When value data is not present, allow union verification function to deal with illegal offset
                    result = verifyObjFunc(self, typeId, len(self.Buf))
        else:
            result = not required
        return result


    def VerifyBuffer(self, identifier, sizePrefixed, verifyObjFunc):
        """
        Method verifies flatbuffer data using generated Table verification function.
        The data buffer is already provided when creating [Verifier] object (see [NewVerifier])
        
        - identifier - the expected identifier of buffer data.
        When empty identifier is provided the identifier validation is skipped.
        - sizePrefixed - this flag should be True when buffer is prefixed with content size
        - verifyObjFunc - function to be used for verification. This function is generated by compiler and included in each table definition file with name "<Tablename>Verify"
        
        Example:
        
           /* Verify Monster table. Ignore buffer name and assume buffer does not contain data length prefix */
           isValid = verifier.VerifyBuffer([]byte{}, False, MonsterVerify)

           /* Verify Monster table. Buffer name is 'MONS' and contains data length prefix */
           isValid = verifier.VerifyBuffer("MONS", True, MonsterVerify)
        """
        # Reset counters - starting verification from beginning
        self.depth = 0
        self.numTables = 0

        if sizePrefixed:
            start = self.Pos + N.Uint32Flags.bytewidth
            result = self.checkScalar(self.Pos, N.Uint32Flags.bytewidth)
            if result:
                size = self.readUOffsetT(self.Buf, self.Pos)
                result = size == (len(self.Buf) - start)
        else:
            result = True
            start = self.Pos
        result = result and self.checkBufferFromStart(identifier, start, verifyObjFunc)
        return result


    def SetMaxDepth(self, value):
        """
        Specify maximum depth of valid structure
        """
        self.options.maxDepth = value
        return self


    def SetMaxTables(self, value):
        """
        Specify maximum number of tables in structure
        """
        self.options.maxTables = value
        return self


    def SetAlignmentCheck(self, value):
        """
        Enable/disable buffer content alignment check
        """
        self.options.alignmentCheck = value
        return self



    def SetStringCheck(self, value):
        """
        Enable/disable checking of string termination '0' charakter
        """
        self.options.stringEndCheck = value
        return self


def checkStringFunc(verifier, offset):
    """
    String check wrapper funnction to be used in vector of strings check
    """
    return verifier.checkString(offset)

    
def NewVerifier(buffer, offset = 0):
    """
    Create and initialize [Verifier] object that is then used by [Verifier.VerifyBuffer] method to verify flatbuffer data.
    Verifier checks data contained in a buffer and optionally uses an offset of data position (default: 0).
        
    The created object is initialized with default verification options. They can be adjusted
    by appropriate fluent methods: SetMaxDepth, SetMaxTables, SetAlignmentCheck and SetStringCheck.
        
    Examples:

     v = NewVerifier(flatbufferData).SetMaxTables(1234)
     --- 
     dataOffset = 456
     v = NewVerifier(flatbufferData, dataOffset).SetMaxTables(1234)
    """
    verifier = Verifier(buffer, offset)
    verifier.SetMaxTables(DEFAULT_MAX_TABLES)
    verifier.SetMaxDepth(DEFAULT_MAX_DEPTH)
    verifier.SetStringCheck(True)
    verifier.SetAlignmentCheck(True)
    return verifier

