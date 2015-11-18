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
import java.nio.charset.Charset
import java.util.Arrays

/**
 * Class that helps you build a FlatBuffer.  See the section
 * ["Use in Java"](http://google.github.io/flatbuffers/md__java_usage.html) in the
 * main FlatBuffers documentation.
 */
class FlatBufferBuilder {
    internal var bb: ByteBuffer = EMPTY_BYTEBUFFER         // Where we construct the FlatBuffer.
    internal var space: Int = 0              // Remaining space in the ByteBuffer.
    internal var minalign = 1       // Minimum alignment encountered so far.
    internal var vtable: IntArray? = null    // The vtable for the current table.
    internal var vtable_in_use = 0  // The amount of fields we're actually using.
    internal var nested = false // Whether we are currently serializing a table.
    internal var object_start: Int = 0       // Starting offset of the current struct/table.
    internal var vtables = IntArray(16)  // List of offsets of all vtables.
    internal var num_vtables = 0          // Number of entries in `vtables` in use.
    internal var vector_num_elems = 0     // For the current vector being built.
    internal var force_defaults = false // False omits default values from the serialized data

    /**
     * Start with a buffer of size `initial_size`, then grow as required.

     * @param initial_size The initial size of the internal buffer to use
     */
    @JvmOverloads constructor(initial_size: Int = 1024) {
        val initSize = if (initial_size <= 0) 1 else initial_size
        space = initSize
        bb = newByteBuffer(initSize)
    }

    /**
     * Alternative constructor allowing reuse of [ByteBuffer]s.  The builder
     * can still grow the buffer as necessary.  User classes should make sure
     * to call [.dataBuffer] to obtain the resulting encoded message

     * @param existing_bb The byte buffer to reuse
     */
    constructor(existing_bb: ByteBuffer) {
        init(existing_bb)
    }

    /**
     * Alternative initializer that allows reusing this object on an existing
     * ByteBuffer. This method resets the builder's internal state, but keeps
     * objects that have been allocated for temporary storage.

     * @param existing_bb The byte buffer to reuse
     * *
     * @return this
     */
    fun init(existing_bb: ByteBuffer): FlatBufferBuilder {
        bb = existing_bb
        bb.clear()
        bb.order(ByteOrder.LITTLE_ENDIAN)
        minalign = 1
        space = bb.capacity()
        vtable_in_use = 0
        nested = false
        object_start = 0
        num_vtables = 0
        vector_num_elems = 0
        return this
    }

    /**
     * Offset relative to the end of the buffer.

     * @return Offset relative to the end of the buffer.
     */
    fun offset(): Int = bb.capacity() - space

    /**
     * Add zero valued bytes to prepare a new entry to be added

     * @param byte_size Number of bytes to add.
     */
    fun pad(byte_size: Int) {
        for (i in 0 until byte_size) bb.put(--space, 0.toByte())
    }

    /**
     * Prepare to write an element of `size` after `additional_bytes`
     * have been written, e.g. if you write a string, you need to align such
     * the int length field is aligned to [Constants.SIZEOF_INT], and
     * the string data follows it directly.  If all you need to do is alignment, `additional_bytes`
     * will be 0.

     * @param size This is the of the new element to write
     * *
     * @param additional_bytes The padding size
     */
    fun prep(size: Int, additional_bytes: Int) {
        // Track the biggest thing we've ever aligned to.
        if (size > minalign) minalign = size
        // Find the amount of alignment needed such that `size` is properly
        // aligned after `additional_bytes`
        val align_size = (((bb.capacity() - space + additional_bytes).inv()) + 1) and (size - 1)
        // Reallocate the buffer if needed.
        while (space < align_size + size + additional_bytes) {
            val old_buf_size = bb.capacity()
            bb = growByteBuffer(bb)
            space += bb.capacity() - old_buf_size
        }
        pad(align_size)
    }

    // Add a scalar to the buffer, backwards from the current location.
    // Doesn't align nor check for space.
    fun putBoolean(x: Boolean) {
        bb.put(--space, (if (x) 1 else 0).toByte())
    }

    fun putByte(x: Byte) {
        bb.put(--space, x)
    }

    fun putShort(x: Short) {
        space -= 2
        bb.putShort(space, x)
    }

    fun putInt(x: Int) {
        space -= 4
        bb.putInt(space, x)
    }

    fun putLong(x: Long) {
        space -= 8
        bb.putLong(space, x)
    }

    fun putFloat(x: Float) {
        space -= 4
        bb.putFloat(space, x)
    }

    fun putDouble(x: Double) {
        space -= 8
        bb.putDouble(space, x)
    }

    // Adds a scalar to the buffer, properly aligned, and the buffer grown
    // if needed.
    fun addBoolean(x: Boolean) {
        prep(1, 0)
        putBoolean(x)
    }

    fun addByte(x: Byte) {
        prep(1, 0)
        putByte(x)
    }

    fun addShort(x: Short) {
        prep(2, 0)
        putShort(x)
    }

    fun addInt(x: Int) {
        prep(4, 0)
        putInt(x)
    }

    fun addLong(x: Long) {
        prep(8, 0)
        putLong(x)
    }

    fun addFloat(x: Float) {
        prep(4, 0)
        putFloat(x)
    }

    fun addDouble(x: Double) {
        prep(8, 0)
        putDouble(x)
    }

    /**
     * Adds on offset, relative to where it will be written.

     * @param off The offset to add
     */
    fun addOffset(off: Int) {
        prep(SIZEOF_INT, 0)  // Ensure alignment is already done.
        assert(off <= offset())
        putInt(offset() - off + SIZEOF_INT)
    }

    /**
     * Start a new array/vector of objects.  Users usually will not call
     * this directly.  The `FlatBuffers` compiler will create a start/end
     * method for vector types in generated code.
     *
     *
     * The expected sequence of calls is:
     *
     *  1. Start the array using this method.
     *  1. Call [.addOffset] `num_elems` number of times to set
     * the offset of each element in the array.
     *  1. Call [.endVector] to retrieve the offset of the array.
     *
     *
     *
     * For example, to create an array of strings, do:
     * `// Need 10 strings
     * FlatBufferBuilder builder = new FlatBufferBuilder(existingBuffer);
     * int[] offsets = new int[10];

     * for (int i = 0; i &lt; 10; i++) {
     * offsets[i] = fbb.createString(&quot; &quot; + i);
     * }

     * // Have the strings in the buffer, but don&#39;t have a vector.
     * // Add a vector that references the newly created strings:
     * builder.startVector(4, offsets.length, 4);

     * // Add each string to the newly created vector
     * // The strings are added in reverse order since the buffer
     * // is filled in back to front
     * for (int i = offsets.length - 1; i &gt;= 0; i--) {
     * builder.addOffset(offsets[i]);
     * }

     * // Finish off the vector
     * int offsetOfTheVector = fbb.endVector();
    ` *

     * @param elem_size The size of each element in the array
     * *
     * @param num_elems The number of elements in the array
     * *
     * @param alignment The alignment of the array
     */
    fun startArray(elem_size: Int, num_elems: Int, alignment: Int) {
        notNested()
        vector_num_elems = num_elems
        prep(SIZEOF_INT, elem_size * num_elems)
        prep(alignment, elem_size * num_elems) // Just in case alignment > int.
    }

    /**
     * Finish off the creation of an array and all its elements.  The array
     * must be created with [.startVector].

     * @return The offset at which the newly created array starts.
     * *
     * @see .startVector
     */
    fun endArray(): Int {
        putInt(vector_num_elems)
        return offset()
    }

    /**
     * Encode the string `s` in the buffer using UTF-8.

     * @param s The string to encode
     * *
     * @return The offset in the buffer where the encoded string starts
     */
    fun createString(s: String): Int {
        val utf8 = s.toByteArray(utf8charset)
        addByte(0.toByte())
        startArray(1, utf8.size, 1)
        space -= utf8.size
        bb.position(space)
        bb.put(utf8, 0, utf8.size)
        return endArray()
    }

    /**
     * Encode the string `s` in the buffer using UTF-8.

     * @param s An already encoded UTF-8 string
     * *
     * @return The offset in the buffer where the encoded string starts
     */
    fun createString(s: ByteBuffer): Int {
        val length = s.remaining()
        addByte(0.toByte())
        startArray(1, length, 1)
        space -= length
        bb.position(space)
        bb.put(s)
        return endArray()
    }

    /**
     * Should not be creating any other object, string or vector
     * while an object is being constructed
     */
    fun notNested() {
        if (nested) throw AssertionError("FlatBuffers: object serialization must not be nested.")
    }

    /**
     * Structures are always stored inline, they need to be created right
     * where they're used.  You'll get this assertion failure if you
     * created it elsewhere.

     * @param obj The offset of the created object
     */
    fun Nested(obj: Int) {
        if (obj != offset()) throw AssertionError("FlatBuffers: struct must be serialized inline.")
    }

    /**
     * Start encoding a new object in the buffer.  Users will not usually need to
     * call this directly. The `FlatBuffers` compiler will generate helper methods
     * that call this method internally.
     *
     *
     * For example, using the "Monster" code found on the
     * [landing page](http://google.github.io/flatbuffers/md__java_usage.html). An
     * object of type `Monster` can be created using the following code:

     * `int testArrayOfString = Monster.createTestarrayofstringVector(fbb, new int[] {
     * fbb.createString(&quot;test1&quot;),
     * fbb.createString(&quot;test2&quot;)
     * });

     * Monster.startMonster(fbb);
     * Monster.addPos(fbb, Vec3.createVec3(fbb, 1.0f, 2.0f, 3.0f, 3.0,
     * Color.Green, (short)5, (byte)6));
     * Monster.addHp(fbb, (short)80);
     * Monster.addName(fbb, str);
     * Monster.addInventory(fbb, inv);
     * Monster.addTestType(fbb, (byte)Any.Monster);
     * Monster.addTest(fbb, mon2);
     * Monster.addTest4(fbb, test4);
     * Monster.addTestarrayofstring(fbb, testArrayOfString);
     * int mon = Monster.endMonster(fbb);
    ` *
     *
     *
     * Here:
     *
     *  * The call to `Monster#startMonster(FlatBufferBuilder)` will call this
     * method with the right number of fields set.
     *  * `Monster#endMonster(FlatBufferBuilder)` will ensure [.endObject] is called.
     *
     *
     *
     * It's not recommended to call this method directly.  If it's called manually, you must ensure
     * to audit all calls to it whenever fields are added or removed from your schema.  This is
     * automatically done by the code generated by the `FlatBuffers` compiler.

     * @param numfields The number of fields found in this object.
     */
    fun startObject(numfields: Int) {
        notNested()
        if (vtable == null || vtable!!.size() < numfields) vtable = IntArray(numfields)
        vtable_in_use = numfields
        Arrays.fill(vtable, 0, vtable_in_use, 0)
        nested = true
        object_start = offset()
    }

    // Add a scalar to a table at `o` into its vtable, with value `x` and default `d`
    fun addBoolean(o: Int, x: Boolean, d: Boolean) {
        if (force_defaults || x != d) {
            addBoolean(x)
            slot(o)
        }
    }

    fun addByte(o: Int, x: Byte, d: Int) {
        if (force_defaults || x.toInt() != d) {
            addByte(x)
            slot(o)
        }
    }

    fun addShort(o: Int, x: Short, d: Int) {
        if (force_defaults || x.toInt() != d) {
            addShort(x)
            slot(o)
        }
    }

    fun addInt(o: Int, x: Int, d: Int) {
        if (force_defaults || x != d) {
            addInt(x)
            slot(o)
        }
    }

    fun addLong(o: Int, x: Long, d: Long) {
        if (force_defaults || x != d) {
            addLong(x)
            slot(o)
        }
    }

    fun addFloat(o: Int, x: Float, d: Double) {
        if (force_defaults || x.toDouble() != d) {
            addFloat(x)
            slot(o)
        }
    }

    fun addDouble(o: Int, x: Double, d: Double) {
        if (force_defaults || x != d) {
            addDouble(x)
            slot(o)
        }
    }

    fun addOffset(o: Int, x: Int, d: Int) {
        if (force_defaults || x != d) {
            addOffset(x)
            slot(o)
        }
    }

    // Structs are stored inline, so nothing additional is being added. `d` is always 0.
    fun addStruct(voffset: Int, x: Int, d: Int) {
        if (x != d) {
            Nested(x)
            slot(voffset)
        }
    }

    // Set the current vtable at `voffset` to the current location in the buffer.
    fun slot(voffset: Int) {
        vtable!![voffset] = offset()
    }

    /**
     * Finish off writing the object that is under construction.

     * @return The offset to the object inside [.dataBuffer]
     * *
     * @see .startObject
     */
    fun endObject(): Int {
        if (vtable == null || !nested)
            throw AssertionError("FlatBuffers: endObject called without startObject")
        addInt(0)
        val vtableloc = offset()
        // Write out the current vtable.
        for (i in vtable_in_use - 1 downTo 0) {
            // Offset relative to the start of the table.
            val off = (if (vtable!![i] != 0) vtableloc - vtable!![i] else 0).toShort()
            addShort(off)
        }

        val standard_fields = 2 // The fields below:
        addShort((vtableloc - object_start).toShort())
        addShort(((vtable_in_use + standard_fields) * SIZEOF_SHORT).toShort())

        // Search for an existing vtable that matches the current one.
        var existing_vtable = 0
        outer_loop@ for (i in 0..num_vtables - 1) {
            val vt1 = bb.capacity() - vtables[i]
            val vt2 = space
            val len = bb.getShort(vt1)
            if (len == bb.getShort(vt2)) {
                var j = SIZEOF_SHORT
                while (j < len) {
                    if (bb.getShort(vt1 + j) != bb.getShort(vt2 + j)) {
                        continue@outer_loop
                    }
                    j += SIZEOF_SHORT
                }
                existing_vtable = vtables[i]
                break@outer_loop
            }
        }

        if (existing_vtable != 0) {
            // Found a match:
            // Remove the current vtable.
            space = bb.capacity() - vtableloc
            // Point table to existing vtable.
            bb.putInt(space, existing_vtable - vtableloc)
        } else {
            // No match:
            // Add the location of the current vtable to the list of vtables.
            if (num_vtables == vtables.size()) vtables = Arrays.copyOf(vtables, num_vtables * 2)
            vtables[num_vtables++] = offset()
            // Point table to current vtable.
            bb.putInt(bb.capacity() - vtableloc, offset() - vtableloc)
        }

        nested = false
        return vtableloc
    }

    // This checks a required field has been set in a given table that has
    // just been constructed.
    fun required(table: Int, field: Int) {
        val table_start = bb.capacity() - table
        val vtable_start = table_start - bb.getInt(table_start)
        val ok = bb.getShort(vtable_start + field) != 0.toShort()
        // If this fails, the caller will show what field needs to be set.
        if (!ok)
            throw AssertionError("FlatBuffers: field $field must be set")
    }

    fun finish(root_table: Int) {
        prep(minalign, SIZEOF_INT)
        addOffset(root_table)
        bb.position(space)
    }

    fun finish(root_table: Int, file_identifier: String) {
        prep(minalign, SIZEOF_INT + FILE_IDENTIFIER_LENGTH)
        if (file_identifier.length != FILE_IDENTIFIER_LENGTH)
            throw AssertionError("FlatBuffers: file identifier must be length " + FILE_IDENTIFIER_LENGTH)
        for (i in FILE_IDENTIFIER_LENGTH - 1 downTo 0) {
            addByte(file_identifier[i].toByte())
        }
        finish(root_table)
    }

    /**
     * In order to save space, fields that are set to their default value
     * don't get serialized into the buffer. Forcing defaults provides a
     * way to manually disable this optimization.

     * @param forceDefaults true always serializes default values
     * *
     * @return this
     */
    fun forceDefaults(forceDefaults: Boolean): FlatBufferBuilder {
        this.force_defaults = forceDefaults
        return this
    }

    // Get the ByteBuffer representing the FlatBuffer. Only call this after you've
    // called finish(). The actual data starts at the ByteBuffer's current position,
    // not necessarily at 0.
    fun dataBuffer(): ByteBuffer = bb

    /**
     * The FlatBuffer data doesn't start at offset 0 in the [ByteBuffer], but
     * now the `ByteBuffer`'s position is set to that location upon [.finish].

     * @return The [position][ByteBuffer.position] the data starts in [.dataBuffer]
     * *
     */

    @Deprecated("This method should not be needed anymore, but is left\n     here for the moment to document this API change. It will be removed in the future.")
    private fun dataStart(): Int = space

    /**
     * Utility function for copying a byte array from `start` to
     * `start` + `length`

     * @param start Start copying at this offset
     * *
     * @param length How many bytes to copy
     * *
     * @return A range copy of the [data buffer][.dataBuffer]
     * *
     * @throws IndexOutOfBoundsException If the range of bytes is ouf of bound
     */
    @JvmOverloads fun sizedByteArray(start: Int = space, length: Int = bb.capacity() - space): ByteArray {
        val array = ByteArray(length)
        bb.position(start)
        bb.get(array)
        return array
    }

    companion object {
        internal val utf8charset = Charset.forName("UTF-8")

        internal fun newByteBuffer(capacity: Int): ByteBuffer {
            val newbb = ByteBuffer.allocate(capacity)
            newbb.order(ByteOrder.LITTLE_ENDIAN)
            return newbb
        }

        /**
         * Doubles the size of the backing {link ByteBuffer} and copies the old data towards the
         * end of the new buffer (since we build the buffer backwards).

         * @param bb The current buffer with the existing data
         * *
         * @return A new byte buffer with the old data copied copied to it.  The data is
         * * located at the end of the buffer.
         */
        internal fun growByteBuffer(bb: ByteBuffer): ByteBuffer {
            val old_buf_size = bb.capacity()
            if ((old_buf_size and -1073741824) != 0)
            // Ensure we don't grow beyond what fits in an int.
                throw AssertionError("FlatBuffers: cannot grow buffer beyond 2 gigabytes.")
            val new_buf_size = old_buf_size shl 1
            bb.position(0)
            val nbb = newByteBuffer(new_buf_size)
            nbb.position(new_buf_size - old_buf_size)
            nbb.put(bb)
            return nbb
        }
    }
}
/**
 * Start with a buffer of 1KiB, then grow as required.
 */
/**
 * Utility function for copying a byte array that starts at 0.

 * @return A full copy of the [data buffer][.dataBuffer]
 */
