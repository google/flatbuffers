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

package com.google.flatbuffers;

import static com.google.flatbuffers.Constants.*;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

// All tables in the generated code derive from this class, and add their own accessors.
public class Table {
  protected int bb_pos;
  protected ByteBuffer bb;

  public ByteBuffer getByteBuffer() { return bb; }

  // Look up a field in the vtable, return an offset into the object, or 0 if the field is not
  // present.
  protected int __offset(int vtable_offset) {
    int vtable = bb_pos - bb.getInt(bb_pos);
    return vtable_offset < bb.getShort(vtable) ? bb.getShort(vtable + vtable_offset) : 0;
  }

  // Retrieve the relative offset stored at "offset"
  protected int __indirect(int offset) {
    return offset + bb.getInt(offset);
  }

  // Create a java String from UTF-8 data stored inside the flatbuffer.
  // This allocates a new string and converts to wide chars upon each access,
  // which is not very efficient. Instead, each FlatBuffer string also comes with an
  // accessor based on __vector_as_bytebuffer below, which is much more efficient,
  // assuming your Java program can handle UTF-8 data directly.
  protected String __string(int offset) {
    offset += bb.getInt(offset);
    if (bb.hasArray()) {
      return new String(bb.array(), bb.arrayOffset() + offset + SIZEOF_INT, bb.getInt(offset), FlatBufferBuilder.utf8charset);
    } else {
      // We can't access .array(), since the ByteBuffer is read-only,
      // off-heap or a memory map
      ByteBuffer bb = this.bb.duplicate().order(ByteOrder.LITTLE_ENDIAN);
      // We're forced to make an extra copy:
      byte[] copy = new byte[bb.getInt(offset)];
      bb.position(offset + SIZEOF_INT);
      bb.get(copy);
      return new String(copy, 0, copy.length, FlatBufferBuilder.utf8charset);
    }
  }

  // Get the length of a vector whose offset is stored at "offset" in this object.
  protected int __vector_len(int offset) {
    offset += bb_pos;
    offset += bb.getInt(offset);
    return bb.getInt(offset);
  }

  // Get the start of data of a vector whose offset is stored at "offset" in this object.
  protected int __vector(int offset) {
    offset += bb_pos;
    return offset + bb.getInt(offset) + SIZEOF_INT;  // data starts after the length
  }

  // Get a whole vector as a ByteBuffer. This is efficient, since it only allocates a new
  // bytebuffer object, but does not actually copy the data, it still refers to the same
  // bytes as the original ByteBuffer.
  // Also useful with nested FlatBuffers etc.
  protected ByteBuffer __vector_as_bytebuffer(int vector_offset, int elem_size) {
    int o = __offset(vector_offset);
    if (o == 0) return null;
    ByteBuffer bb = this.bb.duplicate().order(ByteOrder.LITTLE_ENDIAN);
    int vectorstart = __vector(o);
    bb.position(vectorstart);
    bb.limit(vectorstart + __vector_len(o) * elem_size);
    return bb;
  }

  // Initialize any Table-derived type to point to the union at the given offset.
  protected Table __union(Table t, int offset) {
    offset += bb_pos;
    t.bb_pos = offset + bb.getInt(offset);
    t.bb = bb;
    return t;
  }

  protected static boolean __has_identifier(ByteBuffer bb, String ident) {
    if (ident.length() != FILE_IDENTIFIER_LENGTH)
        throw new AssertionError("FlatBuffers: file identifier must be length " +
                                 FILE_IDENTIFIER_LENGTH);
    for (int i = 0; i < FILE_IDENTIFIER_LENGTH; i++) {
      if (ident.charAt(i) != (char)bb.get(bb.position() + SIZEOF_INT + i)) return false;
    }
    return true;
  }
}
