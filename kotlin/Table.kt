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

package com.google.flatbuffers.kotlin

import java.nio.ByteBuffer
import java.nio.ByteOrder

// All tables in the generated code derive from this class, and add their own accessors.
abstract class Table(protected var bb: ByteBuffer, protected var bb_pos: Int) {
    public val byteBuffer:ByteBuffer get() =  bb

    // Look up a field in the vtable, return an offset into the object, or 0 if the field is not
    // present.
    protected fun __offset(vtable_offset: Int): Int {
        val vtable = bb_pos - bb.getInt(bb_pos)
        return if (vtable_offset < bb.getShort(vtable)) bb.getShort(vtable + vtable_offset).toInt() else 0
    }

    // Retrieve the relative offset stored at "offset"
    protected fun __indirect(offset: Int): Int =offset + bb.getInt(offset)

    // Create a java String from UTF-8 data stored inside the flatbuffer.
    // This allocates a new string and converts to wide chars upon each access,
    // which is not very efficient. Instead, each FlatBuffer string also comes with an
    // accessor based on __vector_as_bytebuffer below, which is much more efficient,
    // assuming your Java program can handle UTF-8 data directly.
    protected fun __string(offset: Int): String {
        val off = offset + bb.getInt(offset)
        return if (bb.hasArray()) String(bb.array(), bb.arrayOffset() + off + SIZEOF_INT, bb.getInt(off), FlatBufferBuilder.utf8charset) else {
            // We can't access .array(), since the ByteBuffer is read-only,
            // off-heap or a memory map
            val bb = this.bb.duplicate().order(ByteOrder.LITTLE_ENDIAN)
            // We're forced to make an extra copy:
            val copy = ByteArray(bb.getInt(off))
            bb.position(off + SIZEOF_INT)
            bb.get(copy)
            String(copy, 0, copy.size, FlatBufferBuilder.utf8charset)
        }
    }

    // Get the length of a vector whose offset is stored at "offset" in this object.
    protected fun __vector_len(offset: Int): Int {
        val off = offset + bb_pos
        return bb.getInt(off + bb.getInt(off))
    }

    // Get the start of data of a vector whose offset is stored at "offset" in this object.
    protected fun __vector(offset: Int): Int {
        val off = offset + bb_pos
        return off + bb.getInt(off) + SIZEOF_INT  // data starts after the length
    }

    // Get a whole vector as a ByteBuffer. This is efficient, since it only allocates a new
    // bytebuffer object, but does not actually copy the data, it still refers to the same
    // bytes as the original ByteBuffer.
    // Also useful with nested FlatBuffers etc.
    protected fun __vector_as_bytebuffer(vector_offset: Int, elem_size: Int): ByteBuffer {
        val o = __offset(vector_offset)
        val bb = this.bb.duplicate().order(ByteOrder.LITTLE_ENDIAN)
        val vectorstart = __vector(o)
        bb.position(vectorstart)
        bb.limit(vectorstart + __vector_len(o) * elem_size)
        return bb
    }

    // Initialize any Table-derived type to point to the union at the given offset.
    protected fun __union(t: com.google.flatbuffers.kotlin.Table, offset: Int): com.google.flatbuffers.kotlin.Table {
        val off = offset + bb_pos
        t.bb_pos = off + bb.getInt(off)
        t.bb = bb
        return t
    }

    companion object {
        public fun hasIdentifier(byteBuffer: ByteBuffer, fileIdentifier: String): Boolean {
            if (fileIdentifier.length != FILE_IDENTIFIER_LENGTH) throw AssertionError("FlatBuffers: file identifier has length ${fileIdentifier.length} instead of $FILE_IDENTIFIER_LENGTH")
            for (i in 0 until FILE_IDENTIFIER_LENGTH) if (fileIdentifier[i] != byteBuffer.get(byteBuffer.position() + SIZEOF_INT + i).toChar()) return false
            return true
        }
    }
}
