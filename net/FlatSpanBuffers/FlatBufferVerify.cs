/*
 * Copyright 2014 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
using System;
using Google.FlatSpanBuffers.Operations;

namespace Google.FlatSpanBuffers
{
  /// <summary>
  /// The Class of the Verifier Options
  /// </summary>
  public struct Options
  {
    public const int DEFAULT_MAX_DEPTH = 64;
    public const int DEFAULT_MAX_TABLES = 1000000;

    /// <summary> Maximum depth of nested tables allowed in a valid flatbuffer. </summary>
    public int MaxDepth { get; set; }
    /// <summary> Maximum number of tables allowed in a valid flatbuffer. </summary>
    public int MaxTables { get; set; }
    /// <summary> Check that string contains its null terminator </summary>
    public bool StringEndCheck { get; set; }
    /// <summary> Check alignment of elements </summary>
    public bool AlignmentCheck { get; set; }

    public Options()
      : this(maxDepth: DEFAULT_MAX_DEPTH, maxTables: DEFAULT_MAX_TABLES, 
            stringEndCheck: true, alignmentCheck: true)
    {
    }

    public Options(int maxDepth, int maxTables, bool stringEndCheck, bool alignmentCheck)
    {
      this.MaxDepth = maxDepth;
      this.MaxTables = maxTables;
      this.StringEndCheck = stringEndCheck;
      this.AlignmentCheck = alignmentCheck;
    }

  }

  public struct CheckElementStruct
  {
    public bool elementValid;
    public uint elementOffset;
  }

  public delegate bool VerifyTableAction(ref Verifier verifier, uint tablePos);
  public delegate bool VerifyUnionAction(ref Verifier verifier, byte typeId, uint tablePos);

  /// <summary>
  /// The Main ref struct of the FlatBuffer Verifier,
  /// Can be constructed from either ByteBuffer or ByteSpanBuffer.
  /// </summary>
  public ref struct Verifier
  {
    public const int SIZE_BYTE = 1;
    public const int SIZE_INT = 4;
    public const int SIZE_U_OFFSET = 4;
    public const int SIZE_S_OFFSET = 4;
    public const int SIZE_V_OFFSET = 2;
    public const int SIZE_PREFIX_LENGTH = FlatBufferConstants.SizePrefixLength;
    public const int FLATBUFFERS_MAX_BUFFER_SIZE = int.MaxValue;
    public const int FILE_IDENTIFIER_LENGTH = FlatBufferConstants.FileIdentifierLength;

    private ByteSpanBuffer _buf;
    private Options _options;
    private int _depth;
    private int _numTables;

    /// <summary> Bytes Buffer for Verify</summary>
    public ByteSpanBuffer Buf { get => _buf; set => _buf = value; }
    /// <summary> Options of the Verifier </summary>
    public Options Options => _options;
    /// <summary> Counter of tables depth in a tested flatbuffer </summary>
    public int Depth => _depth;
    /// <summary> Counter of tables in a tested flatbuffer </summary>
    public int NumTables => _numTables;


    /// <summary> The Constructor of the Verifier object with input parameters: ByteBuffer and/or Options </summary>
    /// <param name="buf"> Input flat byte buffer defined as ByteBuffer type</param>
    /// <param name="options"> Options object with settings for the coniguration the Verifier </param>
    public Verifier(ByteSpanBuffer buf, Options options)
    {
      _buf = buf;
      _options = options;
      _depth = 0;
      _numTables = 0;
    }

    public Verifier(ByteSpanBuffer buf)
      : this(buf, new Options())
    {
    }

    public Verifier(ByteBuffer buf, Options options)
      : this(new ByteSpanBuffer(buf.ToSpan(0, buf.Length)), options)
    {
      // Preserve the Position for offset calculations
      _buf.Position = buf.Position;
    }

    public Verifier(ByteBuffer buf)
      : this(buf, new Options())
    {
    }

    /// <summary> Check if there is identifier in buffer </summary>
    /// <param name="startPos">Start position of data in the Byte Buffer</param>
    /// <param name="identifier"> Identifier for the Byte Buffer</param>
    /// <returns> Return True when the Byte Buffer Identifier is present</returns>
    private bool BufferHasIdentifier(uint startPos, string identifier)
    {
      return TableOperations.HasIdentifier(_buf, identifier, (int)startPos);
    }

    /// <summary> Get UOffsetT from buffer at given position - it must be verified before read </summary>
    /// <param name="pos"> Position of data in the Byte Buffer</param>
    /// <returns> Return the UOffset Value (Unsigned Integer type - 4 bytes) in pos </returns>
    private uint ReadUOffsetT(uint pos)
    {
      return _buf.Get<uint>((int)pos);
    }
    /// <summary> Get SOffsetT from buffer at given position - it must be verified before read </summary>
    /// <param name="pos"> Position of data in the Byte Buffer</param>
    /// <returns> Return the SOffset Value (Signed Integer type - 4 bytes) in pos </returns>
    private int ReadSOffsetT(int pos)
    {
      return _buf.Get<int>(pos);
    }
    /// <summary> Get VOffsetT from buffer at given position - it must be verified before read </summary>
    /// <param name="pos"> Position of data in the Byte Buffer</param>
    /// <returns> Return the VOffset Value (Short type - 2 bytes) in pos </returns>
    private short ReadVOffsetT(int pos)
    {
      return _buf.Get<short>(pos);
    }

    /// <summary> Get table data area absolute offset from vtable. Result is the absolute buffer offset.
    /// The result value offset cannot be '0' (pointing to itself) so after validation this method returns '0'
    /// value as a marker for missing optional entry </summary>
    /// <param name="tablePos"> Table Position value in the Byte Buffer </param>
    /// <param name="vtableOffset"> offset value in the Table</param>
    /// <returns> Return the absolute UOffset Value </returns>
    private uint GetVOffset(uint tablePos, short vtableOffset)
    {
      var relPos = TableOperations.GetOffset(vtableOffset, (int)tablePos, _buf);
      return relPos != 0 ? tablePos + (uint)relPos : 0;
    }

    /// <summary> Check flatbuffer complexity (tables depth, elements counter and so on) </summary>
    /// <returns> If complexity is too high function returns false as verification error </returns>
    private bool CheckComplexity()
    {
      return (_depth <= _options.MaxDepth) && (_numTables <= _options.MaxTables);
    }

    /// <summary> Check alignment of element. </summary>
    /// <returns> Return True when alignment of the element is correct</returns>
    private bool CheckAlignment(uint element, ulong align)
    {
      return ((element & (align - 1)) == 0) || (!_options.AlignmentCheck);
    }

    /// <summary> Check if element is valid in buffer area. </summary> 
    /// <param name="pos"> Value defines the offset/position to element</param>
    /// <param name="elementSize"> Size of element</param>
    /// <returns> Return True when Element is correct </returns>
    private bool CheckElement(uint pos, ulong elementSize)
    {
      return (elementSize < (ulong)_buf.Length) &&
             (pos <= ((uint)_buf.Length - elementSize));
    }
    /// <summary> Check if element is a valid scalar. </summary>
    /// <param name="pos"> Value defines the offset to scalar</param>
    /// <param name="elementSize"> Size of element</param>
    /// <returns> Return True when Scalar Element is correct </returns>
    private bool CheckScalar(uint pos, ulong elementSize)
    {
      return CheckAlignment(pos, elementSize) && CheckElement(pos, elementSize);
    }

    private CheckElementStruct CheckVectorOrString(uint pos, ulong elementSize)
    {
      var result = new CheckElementStruct
      {
        elementValid = false,
        elementOffset = 0
      };

      uint vectorPos = pos;
      // Check we can read the vector/string size field (it is of uoffset size)
      if (!CheckScalar(vectorPos, SIZE_U_OFFSET))
      {
        // result.elementValid = false; result.elementOffset = 0;
        return result;
      }
      // Check the whole array. If this is a string, the byte past the array
      // must be 0.
      uint size = ReadUOffsetT(vectorPos);
      ulong max_elements = FLATBUFFERS_MAX_BUFFER_SIZE / elementSize;
      if (size >= max_elements)
      {
        // Protect against byte_size overflowing. 
        // result.elementValid = false; result.elementOffset = 0;
        return result;
      }

      uint bytes_size = SIZE_U_OFFSET + ((uint)elementSize * size);
      uint buffer_end_pos = vectorPos + bytes_size;
      result.elementValid = CheckElement(vectorPos, bytes_size);
      result.elementOffset = buffer_end_pos;
      return result;
    }

    /// <summary>Verify a string at given position.</summary>
    private bool CheckString(uint pos)
    {
      var result = CheckVectorOrString(pos, SIZE_BYTE);
      if (_options.StringEndCheck)
      {
        result.elementValid = result.elementValid && CheckScalar(result.elementOffset, 1); // Must have terminator
        result.elementValid = result.elementValid && (_buf.GetSbyte((int)result.elementOffset) == 0); // Terminating byte must be 0.
      }
      return result.elementValid;
    }

    /// <summary> Verify the vector of elements of given size </summary>
    private bool CheckVector(uint pos, ulong elementSize)
    {
      var result = CheckVectorOrString(pos, elementSize);
      return result.elementValid;
    }

    /// <summary> String check wrapper function to be used in vector of strings check </summary>
    private static bool CheckStringFunc(ref Verifier verifier, uint pos)
    {
      return verifier.CheckString(pos);
    }

    /// <summary> Check vector of objects. Use generated object verification function </summary>
    private bool CheckVectorOfObjects(uint pos, VerifyTableAction verifyAction)
    {
      if (!CheckVector(pos, SIZE_U_OFFSET))
      {
        return false;
      }
      uint size = ReadUOffsetT(pos);
      // Vector data starts just after vector size/length
      uint vecStart = pos + SIZE_U_OFFSET;
      // Iterate offsets and verify referenced objects
      for (uint i = 0; i < size; i++)
      {
        uint vecOff = vecStart + (i * SIZE_U_OFFSET);
        if (!CheckIndirectOffset(vecOff))
        {
          return false;
        }
        uint objOffset = GetIndirectOffset(vecOff);
        if (!verifyAction(ref this, objOffset))
        {
          return false;
        }
      }
      return true;
    }

    /// <summary> Check if the offset referenced by offsetPos is the valid offset pointing to buffer</summary>
    //  offsetPos - offset to offset data
    private bool CheckIndirectOffset(uint pos)
    {
      // Check the input offset is valid
      if (!CheckScalar(pos, SIZE_U_OFFSET))
      {
        return false;
      }
      // Get indirect offset
      uint offset = ReadUOffsetT(pos);
      // May not point to itself neither wrap around  (buffers are max 2GB)
      if ((offset == 0) || (offset >= FLATBUFFERS_MAX_BUFFER_SIZE))
      {
        return false;
      }
      // Must be inside the buffer
      return CheckElement(pos + offset, 1);
    }

    /// <summary> Check flatbuffer content using generated object verification function </summary>
    private bool CheckBufferFromStart(string identifier, uint startPos, VerifyTableAction verifyAction)
    {
      if (!string.IsNullOrEmpty(identifier) &&
         ((_buf.Length < (SIZE_U_OFFSET + FILE_IDENTIFIER_LENGTH)) || (!BufferHasIdentifier(startPos, identifier))))
      {
        return false;
      }
      if (!CheckIndirectOffset(startPos))
      {
        return false;
      }
      uint offset = GetIndirectOffset(startPos);
      return verifyAction(ref this, offset); //  && GetComputedSize()
    }

    /// <summary> Get indirect offset. It is an offset referenced by offset Pos </summary>
    private uint GetIndirectOffset(uint pos)
    {
      return (uint)TableOperations.GetIndirect((int)pos, _buf);
    }

    /// <summary> Verify beginning of table </summary>
    /// <param name="tablePos"> Position in the Table </param>
    /// <returns> Return True when the verification of the beginning of the table is passed</returns> 
    // (this method is used internally by generated verification functions)
    public bool VerifyTableStart(uint tablePos)
    {
      // Starting new table verification increases complexity of structure
      _depth++;
      _numTables++;

      if (!CheckScalar(tablePos, SIZE_S_OFFSET))
      {
        return false;
      }
      uint vtable = (uint)(tablePos - ReadSOffsetT((int)tablePos));
      return CheckComplexity() &&
             CheckScalar(vtable, SIZE_V_OFFSET) &&
             CheckAlignment((uint)ReadVOffsetT((int)vtable), SIZE_V_OFFSET) &&
             CheckElement(vtable, (ulong)ReadVOffsetT((int)vtable));
    }

    /// <summary> Verify end of table. In practice, this function does not check buffer but handles
    /// verification statistics update </summary>
    /// <param name="tablePos"> Position in the Table</param>
    /// <returns>Return True when the verification of the Table End is passed</returns>
    // (this method is used internally by generated verification functions)
    public bool VerifyTableEnd(uint tablePos)
    {
      _depth--;
      return true;
    }

    /// <summary> Verifiy static/inlined data area field </summary>
    /// <param name="tablePos"> Position in the Table</param>
    /// <param name="offsetId"> Offset to the static/inlined data element </param>
    /// <param name="elementSize"> Size of the element </param>
    /// <param name="align"> Alignment bool value </param>
    /// <param name="required"> Required Value when the offset == 0 </param>
    /// <returns>Return True when the verification of the static/inlined data element is passed</returns>
    // (this method is used internally by generated verification functions)
    public bool VerifyField(uint tablePos, short offsetId, ulong elementSize, ulong align, bool required)
    {
      uint offset = GetVOffset(tablePos, offsetId);
      if (offset != 0)
      {
        return CheckAlignment(offset, align) && CheckElement(offset, elementSize);
      }
      return !required; // it is OK if field is not required
    }

    /// <summary> Verify string </summary> 
    /// <param name="tablePos"> Position in the Table</param>
    /// <param name="vOffset"> Offset to the String element </param>
    /// <param name="required"> Required Value when the offset == 0 </param>
    /// <returns>Return True when the verification of the String is passed</returns>
    // (this method is used internally by generated verification functions)
    public bool VerifyString(uint tablePos, short vOffset, bool required)
    {
      var offset = GetVOffset(tablePos, vOffset);
      if (offset == 0)
      {
        return !required;
      }
      if (!CheckIndirectOffset(offset))
      {
        return false;
      }
      var strOffset = GetIndirectOffset(offset);
      return CheckString(strOffset);
    }

    /// <summary> Verify vector of fixed size structures and scalars </summary>
    /// <param name="tablePos"> Position in the Table</param>
    /// <param name="vOffset"> Offset to the Vector of Data </param>
    /// <param name="elementSize"> Size of the element</param>
    /// <param name="required"> Required Value when the offset == 0 </param>
    /// <returns>Return True when the verification of the Vector of Data passed</returns>
    // (this method is used internally by generated verification functions)
    public bool VerifyVectorOfData(uint tablePos, short vOffset, ulong elementSize, bool required)
    {
      var offset = GetVOffset(tablePos, vOffset);
      if (offset == 0)
      {
        return !required;
      }
      if (!CheckIndirectOffset(offset))
      {
        return false;
      }
      var vecOffset = GetIndirectOffset(offset);
      return CheckVector(vecOffset, elementSize);
    }

    /// <summary> Verify array of strings </summary>
    /// <param name="tablePos"> Position in the Table</param>
    /// <param name="offsetId"> Offset to the Vector of String </param>
    /// <param name="required"> Required Value when the offset == 0 </param>
    /// <returns>Return True when the verification of the Vector of String passed</returns>
    // (this method is used internally by generated verification functions)
    public bool VerifyVectorOfStrings(uint tablePos, short offsetId, bool required)
    {
      var offset = GetVOffset(tablePos, offsetId);
      if (offset == 0)
      {
        return !required;
      }
      if (!CheckIndirectOffset(offset))
      {
        return false;
      }
      var vecOffset = GetIndirectOffset(offset);
      return CheckVectorOfObjects(vecOffset, CheckStringFunc);
    }

    /// <summary> Verify vector of tables (objects). Tables are verified using generated verifyObjFunc </summary>
    /// <param name="tablePos"> Position in the Table</param>
    /// <param name="offsetId"> Offset to the Vector of Table </param>
    /// <param name="verifyAction"> Method used to the verification Table </param>
    /// <param name="required"> Required Value when the offset == 0 </param>
    /// <returns>Return True when the verification of the Vector of Table passed</returns>
    // (this method is used internally by generated verification functions)
    public bool VerifyVectorOfTables(uint tablePos, short offsetId, VerifyTableAction verifyAction, bool required)
    {
      var offset = GetVOffset(tablePos, offsetId);
      if (offset == 0)
      {
        return !required;
      }
      if (!CheckIndirectOffset(offset))
      {
        return false;
      }
      var vecOffset = GetIndirectOffset(offset);
      return CheckVectorOfObjects(vecOffset, verifyAction);
    }

    /// <summary> Verify table object using generated verification function. </summary>
    /// <param name="tablePos"> Position in the Table</param>
    /// <param name="offsetId"> Offset to the Table </param>
    /// <param name="verifyAction"> Method used to the verification Table </param>
    /// <param name="required"> Required Value when the offset == 0 </param>
    /// <returns>Return True when the verification of the Table passed</returns>
    // (this method is used internally by generated verification functions)
    public bool VerifyTable(uint tablePos, short offsetId, VerifyTableAction verifyAction, bool required)
    {
      var offset = GetVOffset(tablePos, offsetId);
      if (offset == 0)
      {
        return !required;
      }
      if (!CheckIndirectOffset(offset))
      {
        return false;
      }
      var tabOffset = GetIndirectOffset(offset);
      return verifyAction(ref this, tabOffset);
    }

    /// <summary> Verify nested buffer object. When verifyObjFunc is provided, it is used to verify object structure. </summary>
    /// <param name="tablePos"> Position in the Table </param>
    /// <param name="offsetId"> Offset to the Table </param>
    /// <param name="verifyAction">  Method used to the verification Table </param>
    /// <param name="required"> Required Value when the offset == 0  </param>
    /// <returns>Return True when the verification of the nested buffer passed</returns>
    // (this method is used internally by generated verification functions)
    public bool VerifyNestedBuffer(uint tablePos, short offsetId, VerifyTableAction verifyAction, bool required)
    {
      var offset = GetVOffset(tablePos, offsetId);
      if (offset == 0)
      {
        return !required;
      }
      uint vecOffset = GetIndirectOffset(offset);
      if (!CheckVector(vecOffset, SIZE_BYTE))
      {
        return false;
      }
      if (verifyAction != null)
      {
        var vecLength = ReadUOffsetT(vecOffset);
        // Buffer begins after vector length
        var vecStart = vecOffset + SIZE_U_OFFSET;
        // Create nested buffer from part of Verify Buffer
        var nestedByteBuffer = new ByteSpanBuffer(_buf.ToSpan((int)vecStart, (int)vecLength));
        var nestedVerifier = new Verifier(nestedByteBuffer, _options);
        // There is no internal identifier - use empty one
        if (!nestedVerifier.CheckBufferFromStart("", 0, verifyAction))
        {
          return false;
        }
      }
      return true;
    }

    /// <summary> Verifiy static/inlined data area at absolute offset </summary>
    /// <param name="pos"> Position of static/inlined data area in the Byte Buffer</param>
    /// <param name="elementSize"> Size of the union data</param>
    /// <param name="align"> Alignment bool value </param>
    /// <returns>Return True when the verification of the Union Data is passed</returns>
    // (this method is used internally by generated verification functions)
    public bool VerifyUnionData(uint pos, ulong elementSize, ulong align)
    {
      bool result = CheckAlignment(pos, align) && CheckElement(pos, elementSize);
      return result;
    }

    /// <summary> Verify string referenced by absolute offset value </summary>
    /// <param name="pos"> Position of Union String in the Byte Buffer</param>
    /// <returns>Return True when the verification of the Union String is passed</returns> 
    // (this method is used internally by generated verification functions)
    public bool VerifyUnionString(uint pos)
    {
      bool result = CheckString(pos);
      return result;
    }

    /// <summary> Method verifies union object using generated verification function </summary>
    /// <param name="tablePos"> Position in the Table</param>
    /// <param name="typeIdVOffset"> Offset in the Table</param>
    /// <param name="valueVOffset"> Offset to Element</param>
    /// <param name="verifyAction"> Verification Method used for Union</param>
    /// <param name="required"> Required Value when the offset == 0 </param>
    /// <returns>Return True when the verification of the Union passed</returns>
    // (this method is used internally by generated verification functions)
    public bool VerifyUnion(uint tablePos, short typeIdVOffset, short valueVOffset, VerifyUnionAction verifyAction, bool required)
    {
      // Check the union type index
      var offset = GetVOffset(tablePos, typeIdVOffset);
      if (offset == 0)
      {
        return !required;
      }
      if (!(CheckAlignment(offset, SIZE_BYTE) && CheckElement(offset, SIZE_BYTE)))
      {
        return false;
      }
      // Take type id
      var typeId = _buf.Get((int)offset);
      // Check union data
      offset = GetVOffset(tablePos, valueVOffset);
      if (offset == 0)
      {
        // When value data is not present, allow union verification function to deal with illegal offset
        return verifyAction(ref this, typeId, (uint)_buf.Length);
      }
      if (!CheckIndirectOffset(offset))
      {
        return false;
      }
      // Take value offset and validate union structure
      uint unionOffset = GetIndirectOffset(offset);
      return verifyAction(ref this, typeId, unionOffset);
    }

    /// <summary> Verify vector of unions (objects). Unions are verified using generated verifyObjFunc </summary>
    /// <param name="tablePos"> Position of the Table</param>
    /// <param name="typeOffsetId"> Offset in the Table (Union type id)</param>
    /// <param name="offsetId"> Offset to vector of Data Stucture offset</param>
    /// <param name="verifyAction"> Verification Method used for Union</param>
    /// <param name="required"> Required Value when the offset == 0 </param>
    /// <returns>Return True when the verification of the Vector of Unions passed</returns>
    // (this method is used internally by generated verification functions)
    public bool VerifyVectorOfUnion(uint tablePos, short typeOffsetId, short offsetId, VerifyUnionAction verifyAction, bool required)
    {
      // type id offset must be valid
      var offset = GetVOffset(tablePos, typeOffsetId);
      if (offset == 0)
      {
        return !required;
      }
      if (!CheckIndirectOffset(offset))
      {
        return false;
      }
      // Get type id table absolute offset
      var typeIdVectorOffset = GetIndirectOffset(offset);
      // values offset must be valid
      offset = GetVOffset(tablePos, offsetId);
      if (!CheckIndirectOffset(offset))
      {
        return false;
      }
      var valueVectorOffset = GetIndirectOffset(offset);
      // validate referenced vectors
      if (!CheckVector(typeIdVectorOffset, SIZE_BYTE) ||
          !CheckVector(valueVectorOffset, SIZE_U_OFFSET))
      {
        return false;
      }
      // Both vectors should have the same length
      var typeIdVectorLength = ReadUOffsetT(typeIdVectorOffset);
      var valueVectorLength = ReadUOffsetT(valueVectorOffset);
      if (typeIdVectorLength != valueVectorLength)
      {
        return false;
      }
      // Verify each union from vectors
      var typeIdStart = typeIdVectorOffset + SIZE_U_OFFSET;
      var valueStart = valueVectorOffset + SIZE_U_OFFSET;
      for (uint i = 0; i < typeIdVectorLength; i++)
      {
        // Get type id
        byte typeId = _buf.Get((int)(typeIdStart + i * SIZE_BYTE));
        // get offset to vector item
        uint off = valueStart + i * SIZE_U_OFFSET;
        // Check the vector item has a proper offset
        if (!CheckIndirectOffset(off))
        {
          return false;
        }
        uint valueOffset = GetIndirectOffset(off);
        // Verify object
        if (!verifyAction(ref this, typeId, valueOffset))
        {
          return false;
        }
      }
      return true;
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
    /// <summary> Method verifies flatbuffer data using generated Table verification function </summary>
    /// 
    /// <param name="identifier"> The expected identifier of buffer data</param>
    /// <param name="sizePrefixed"> Flag should be true when buffer is prefixed with content size</param>
    /// <param name="verifyAction"> Function to be used for verification. This function is generated by compiler and included in each table definition file</param>
    /// <returns> Return True when verification of FlatBuffer passed</returns>
    /// <example>
    /// Example 1. Verify Monster table. Ignore buffer name and assume buffer does not contain data length prefix 
    /// <code>  isValid = verifier.VerifyBuffer(bb, false, MonsterVerify)</code>
    /// Example 2. Verify Monster table. Buffer name is 'MONS' and contains data length prefix 
    /// <code>  isValid = verifier.VerifyBuffer("MONS", true, MonsterVerify)</code>
    /// </example>
    public bool VerifyBuffer(string identifier, bool sizePrefixed, VerifyTableAction verifyAction)
    {
      // Reset counters - starting verification from beginning
      _depth = 0;
      _numTables = 0;

      var start = (uint)_buf.Position;
      if (sizePrefixed)
      {
        start = (uint)_buf.Position + SIZE_PREFIX_LENGTH;
        if (!CheckScalar((uint)_buf.Position, SIZE_PREFIX_LENGTH))
        {
          return false;
        }
        uint size = ReadUOffsetT((uint)_buf.Position);
        if (size != ((uint)_buf.Length - start))
        {
          return false;
        }
      }
      return CheckBufferFromStart(identifier, start, verifyAction);
    }
  }

}
