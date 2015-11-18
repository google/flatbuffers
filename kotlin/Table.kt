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
open class Table {
    protected var _position: Int = 0
    var _byteBuffer: ByteBuffer = EMPTY_BYTEBUFFER
        protected set

    // Look up a field in the vtable, return an offset into the object, or 0 if the field is not
    // present.
    protected fun _offset(vtable_offset: Int): Int {
        val vtable = _position - _byteBuffer.getInt(_position)
        return if (vtable_offset < _byteBuffer.getShort(vtable)) _byteBuffer.getShort(vtable + vtable_offset).toInt() else 0
    }

    // Retrieve the relative offset stored at "offset"
    protected fun _indirect(offset: Int): Int =offset + _byteBuffer.getInt(offset)

    // Create a java String from UTF-8 data stored inside the flatbuffer.
    // This allocates a new string and converts to wide chars upon each access,
    // which is not very efficient. Instead, each FlatBuffer string also comes with an
    // accessor based on __vector_as_bytebuffer below, which is much more efficient,
    // assuming your Java program can handle UTF-8 data directly.
    protected fun _string(offset: Int): String {
        var offset = offset
        offset += _byteBuffer.getInt(offset)
        return if (_byteBuffer.hasArray()) String(_byteBuffer.array(), _byteBuffer.arrayOffset() + offset + SIZEOF_INT, _byteBuffer.getInt(offset), FlatBufferBuilder.utf8charset) else {
            // We can't access .array(), since the ByteBuffer is read-only,
            // off-heap or a memory map
            val bb = this._byteBuffer.duplicate().order(ByteOrder.LITTLE_ENDIAN)
            // We're forced to make an extra copy:
            val copy = ByteArray(bb.getInt(offset))
            bb.position(offset + SIZEOF_INT)
            bb.get(copy)
            String(copy, 0, copy.size, FlatBufferBuilder.utf8charset)
        }
    }

    // Get the length of a vector whose offset is stored at "offset" in this object.
    protected fun _arraySize(offset: Int): Int {
        var offset = offset
        offset += _position
        offset += _byteBuffer.getInt(offset)
        return _byteBuffer.getInt(offset)
    }

    // Get the start of data of a vector whose offset is stored at "offset" in this object.
    protected fun _array(offset: Int): Int {
        val offset = offset + _position
        return offset + _byteBuffer.getInt(offset) + SIZEOF_INT  // data starts after the length
    }

    // Get a whole vector as a ByteBuffer. This is efficient, since it only allocates a new
    // bytebuffer object, but does not actually copy the data, it still refers to the same
    // bytes as the original ByteBuffer.
    // Also useful with nested FlatBuffers etc.
    protected fun _vector_as_bytebuffer(vector_offset: Int, elem_size: Int): ByteBuffer? {
        val o = _offset(vector_offset)
        val bb = this._byteBuffer.duplicate().order(ByteOrder.LITTLE_ENDIAN)
        val vectorstart = _array(o)
        bb.position(vectorstart)
        bb.limit(vectorstart + _arraySize(o) * elem_size)
        return bb
    }

    // Initialize any Table-derived type to point to the union at the given offset.
    protected fun _union(t: com.google.flatbuffers.kotlin.Table, offset: Int): com.google.flatbuffers.kotlin.Table {
        val offset = offset + _position
        t._position = offset + _byteBuffer.getInt(offset)
        t._byteBuffer = _byteBuffer
        return t
    }

    companion object {

        protected fun _has_identifier(bb: ByteBuffer, ident: String): Boolean {
            if (ident.length != FILE_IDENTIFIER_LENGTH) throw AssertionError("FlatBuffers: file identifier must be length " + FILE_IDENTIFIER_LENGTH)
            for (i in 0 until FILE_IDENTIFIER_LENGTH) {
                if (ident[i] != bb.get(bb.position() + SIZEOF_INT + i).toChar()) return false
            }
            return true
        }
    }
}
