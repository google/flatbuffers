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
import java.nio.CharBuffer;
import java.nio.charset.CharacterCodingException;
import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CoderResult;

/// @cond FLATBUFFERS_INTERNAL

/**
 * All tables in the generated code derive from this class, and add their own accessors.
 */
public class Table {
  private final static ThreadLocal<CharsetDecoder> UTF8_DECODER = new ThreadLocal<CharsetDecoder>() {
    @Override
    protected CharsetDecoder initialValue() {
      return Charset.forName("UTF-8").newDecoder();
    }
  };
  private final static ThreadLocal<CharBuffer> CHAR_BUFFER = new ThreadLocal<CharBuffer>();
  /** Used to hold the position of the `bb` buffer. */
  protected int bb_pos;
  /** The underlying ByteBuffer to hold the data of the Table. */
  protected ByteBuffer bb;

  /**
   * Get the underlying ByteBuffer.
   *
   * @return Returns the Table's ByteBuffer.
   */
  public ByteBuffer getByteBuffer() { return bb; }

  /**
   * Look up a field in the vtable.
   *
   * @param vtable_offset An `int` offset to the vtable in the Table's ByteBuffer.
   * @return Returns an offset into the object, or `0` if the field is not present.
   */
  protected int __offset(int vtable_offset) { return __offset(vtable_offset, bb_pos - bb.getInt(bb_pos), bb, false); }

  protected static int __offset(int vtable_offset, int vtable, ByteBuffer _bb, boolean invoked_static) {
    if (!invoked_static) return vtable_offset < _bb.getShort(vtable) ? _bb.getShort(vtable + vtable_offset) : 0;
    else return _bb.getShort(vtable + vtable_offset - _bb.getInt(vtable)) + vtable;
  }

  /**
   * Retrieve a relative offset.
   *
   * @param offset An `int` index into the Table's ByteBuffer containing the relative offset.
   * @return Returns the relative offset stored at `offset`.
   */
  protected int __indirect(int offset) {
    return offset + bb.getInt(offset);
  }

  /**
   * Create a Java `String` from UTF-8 data stored inside the FlatBuffer.
   *
   * This allocates a new string and converts to wide chars upon each access,
   * which is not very efficient. Instead, each FlatBuffer string also comes with an
   * accessor based on __vector_as_bytebuffer below, which is much more efficient,
   * assuming your Java program can handle UTF-8 data directly.
   *
   * @param offset An `int` index into the Table's ByteBuffer.
   * @return Returns a `String` from the data stored inside the FlatBuffer at `offset`.
   */
  protected String __string(int offset) {
    return __string(offset, bb);
  }

  protected static String __string(int offset, ByteBuffer _bb) {
    CharsetDecoder decoder = UTF8_DECODER.get();
    decoder.reset();

    offset += _bb.getInt(offset);
    ByteBuffer src = _bb.duplicate().order(ByteOrder.LITTLE_ENDIAN);
    int length = src.getInt(offset);
    src.position(offset + SIZEOF_INT);
    src.limit(offset + SIZEOF_INT + length);

    int required = (int)((float)length * decoder.maxCharsPerByte());
    CharBuffer dst = CHAR_BUFFER.get();
    if (dst == null || dst.capacity() < required) {
      dst = CharBuffer.allocate(required);
      CHAR_BUFFER.set(dst);
    }

    dst.clear();

    try {
      CoderResult cr = decoder.decode(src, dst, true);
      if (!cr.isUnderflow()) {
        cr.throwException();
      }
    } catch (CharacterCodingException x) {
      throw new Error(x);
    }

    return dst.flip().toString();
  }

  /**
   * Get the length of a vector.
   *
   * @param offset An `int` index into the Table's ByteBuffer.
   * @return Returns the length of the vector whose offset is stored at `offset`.
   */
  protected int __vector_len(int offset) {
    offset += bb_pos;
    offset += bb.getInt(offset);
    return bb.getInt(offset);
  }

  /**
   * Get the start data of a vector.
   *
   * @param offset An `int` index into the Table's ByteBuffer.
   * @return Returns the start of the vector data whose offset is stored at `offset`.
   */
  protected int __vector(int offset) {
    offset += bb_pos;
    return offset + bb.getInt(offset) + SIZEOF_INT;  // data starts after the length
  }

  /**
   * Get a whole vector as a ByteBuffer.
   *
   * This is efficient, since it only allocates a new {@link ByteBuffer} object,
   * but does not actually copy the data, it still refers to the same bytes
   * as the original ByteBuffer. Also useful with nested FlatBuffers, etc.
   *
   * @param vector_offset The position of the vector in the byte buffer
   * @param elem_size The size of each element in the array
   * @return The {@link ByteBuffer} for the array
   */
  protected ByteBuffer __vector_as_bytebuffer(int vector_offset, int elem_size) {
    int o = __offset(vector_offset);
    if (o == 0) return null;
    ByteBuffer bb = this.bb.duplicate().order(ByteOrder.LITTLE_ENDIAN);
    int vectorstart = __vector(o);
    bb.position(vectorstart);
    bb.limit(vectorstart + __vector_len(o) * elem_size);
    return bb;
  }

  /**
   * Initialize any Table-derived type to point to the union at the given `offset`.
   *
   * @param t A `Table`-derived type that should point to the union at `offset`.
   * @param offset An `int` index into the Table's ByteBuffer.
   * @return Returns the Table that points to the union at `offset`.
   */
  protected Table __union(Table t, int offset) {
    offset += bb_pos;
    t.bb_pos = offset + bb.getInt(offset);
    t.bb = bb;
    return t;
  }

  /**
   * Check if a {@link ByteBuffer} contains a file identifier.
   *
   * @param bb A {@code ByteBuffer} to check if it contains the identifier
   * `ident`.
   * @param ident A `String` identifier of the FlatBuffer file.
   * @return True if the buffer contains the file identifier
   */
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

/// @endcond
