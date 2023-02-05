/*
 * Copyright 2021 Google Inc. All rights reserved.
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

import kotlin.jvm.JvmOverloads

/**
 * Class that helps you build a FlatBuffer.  See the section
 * "Use in Kotlin" in the main FlatBuffers documentation.
 */
public class FlatBufferBuilder @JvmOverloads constructor(
  private val initialSize: Int = 1024,
  private var buffer: ReadWriteBuffer = ArrayReadWriteBuffer(initialSize)
) {
  // Remaining space in the ByteBuffer.
  private var space: Int = buffer.capacity

  // Minimum alignment encountered so far.
  private var minalign: Int = 1

  // The vtable for the current table.
  private var vtable: IntArray? = null

  // The amount of fields we're actually using.
  private var vtableInUse: Int = 0

  // Whether we are currently serializing a table.
  private var nested: Boolean = false

  // Whether the buffer is finished.
  private var finished: Boolean = false

  // Starting offset of the current struct/table.
  private var objectStart: Int = 0

  // List of offsets of all vtables.
  private var vtables = IntArray(16)

  // Number of entries in `vtables` in use.
  private var numVtables = 0

  // For the current vector being built.
  private var vectorNumElems = 0

  // False omits default values from the serialized data.
  private var forceDefaults = false

  // map used to cache shared strings.
  private var stringPool: MutableMap<CharSequence, Int>? = null

  /**
   * Reset the FlatBufferBuilder by purging all data that it holds.
   */
  public fun clear() {
    space = buffer.capacity
    buffer.clear()
    minalign = 1
    vtable?.fill(0, 0, vtableInUse)
    vtableInUse = 0
    nested = false
    finished = false
    objectStart = 0
    numVtables = 0
    vectorNumElems = 0
    stringPool?.clear()
  }

  /**
   * Offset relative to the end of the buffer.
   *
   * @return Offset relative to the end of the buffer.
   */
  public fun offset(): Int = buffer.capacity - space

  /**
   * Add zero valued bytes to prepare a new entry to be added.
   *
   * @param byteSize Number of bytes to add.
   */
  public fun pad(byteSize: Int) {
    for (i in 0 until byteSize) {
      buffer[--space] = 0.toByte()
    }
  }

  /**
   * Prepare to write an element of `size` after `additional_bytes`
   * have been written, e.g. if you write a string, you need to align such
   * the int length field is aligned to [com.google.flatbuffers.Int.SIZE_BYTES], and
   * the string data follows it directly.  If all you need to do is alignment, `additional_bytes`
   * will be 0.
   *
   * @param size This is the of the new element to write.
   * @param additionalBytes The padding size.
   */
  public fun prep(size: Int, additionalBytes: Int) {
    // Track the biggest thing we've ever aligned to.
    if (size > minalign) minalign = size
    // Find the amount of alignment needed such that `size` is properly
    // aligned after `additional_bytes`
    val alignSize: Int = ((buffer.capacity - space + additionalBytes).inv() + 1).and(size - 1)
    // Reallocate the buffer if needed.
    while (space < alignSize + size + additionalBytes) {
      val oldBufSize: Int = buffer.capacity
      buffer.moveWrittenDataToEnd(buffer.capacity + alignSize + size + additionalBytes)
      space += buffer.capacity - oldBufSize
    }
    pad(alignSize)
  }

  /**
   * Add a `boolean` to the buffer, backwards from the current location. Doesn't align nor
   * check for space.
   *
   * @param x A `boolean` to put into the buffer.
   */
  public fun putBoolean(x: Boolean) {
    buffer[Byte.SIZE_BYTES.let { space -= it; space }] = (if (x) 1 else 0).toByte()
  }

  /**
   * Add a `byte` to the buffer, backwards from the current location. Doesn't align nor
   * check for space.
   *
   * @param x A `byte` to put into the buffer.
   */
  public fun putByte(x: Byte): Unit = run {
    buffer[Byte.SIZE_BYTES.let { space -= it; space }] = x }

  /**
   * Add a `short` to the buffer, backwards from the current location. Doesn't align nor
   * check for space.
   *
   * @param x A `short` to put into the buffer.
   */
  public fun putShort(x: Short): Unit = run {
    buffer.set(Short.SIZE_BYTES.let { space -= it; space }, x) }

  /**
   * Add an `int` to the buffer, backwards from the current location. Doesn't align nor
   * check for space.
   *
   * @param x An `int` to put into the buffer.
   */
  public fun putInt(x: Int): Unit = run {
    buffer.set(Int.SIZE_BYTES.let { space -= it; space }, x) }

  /**
   * Add a `long` to the buffer, backwards from the current location. Doesn't align nor
   * check for space.
   *
   * @param x A `long` to put into the buffer.
   */
  public fun putLong(x: Long): Unit = run {
    buffer.set(Long.SIZE_BYTES.let { space -= it; space }, x) }

  /**
   * Add a `float` to the buffer, backwards from the current location. Doesn't align nor
   * check for space.
   *
   * @param x A `float` to put into the buffer.
   */
  public fun putFloat(x: Float): Unit = run {
    buffer.set(Float.SIZE_BYTES.let { space -= it; space }, x) }

  /**
   * Add a `double` to the buffer, backwards from the current location. Doesn't align nor
   * check for space.
   *
   * @param x A `double` to put into the buffer.
   */
  public fun putDouble(x: Double): Unit = run {
    buffer.set(Double.SIZE_BYTES.let { space -= it; space }, x) }

  /**
   * Add a `boolean` to the buffer, properly aligned, and grows the buffer (if necessary).
   *
   * @param x A `boolean` to put into the buffer.
   */
  public fun addBoolean(x: Boolean) {
    prep(Byte.SIZE_BYTES, 0)
    putBoolean(x)
  }

  /**
   * Add a `byte` to the buffer, properly aligned, and grows the buffer (if necessary).
   *
   * @param x A `byte` to put into the buffer.
   */
  public fun addByte(x: Byte) {
    prep(Byte.SIZE_BYTES, 0)
    putByte(x)
  }

  /**
   * Add a `short` to the buffer, properly aligned, and grows the buffer (if necessary).
   *
   * @param x A `short` to put into the buffer.
   */
  public fun addShort(x: Short) {
    prep(Short.SIZE_BYTES, 0)
    putShort(x)
  }

  /**
   * Add an `int` to the buffer, properly aligned, and grows the buffer (if necessary).
   *
   * @param x An `int` to put into the buffer.
   */
  public fun addInt(x: Int) {
    prep(Int.SIZE_BYTES, 0)
    putInt(x)
  }

  /**
   * Add a `long` to the buffer, properly aligned, and grows the buffer (if necessary).
   *
   * @param x A `long` to put into the buffer.
   */
  public fun addLong(x: Long) {
    prep(Long.SIZE_BYTES, 0)
    putLong(x)
  }

  /**
   * Add a `float` to the buffer, properly aligned, and grows the buffer (if necessary).
   *
   * @param x A `float` to put into the buffer.
   */
  public fun addFloat(x: Float) {
    prep(Float.SIZE_BYTES, 0)
    putFloat(x)
  }

  /**
   * Add a `double` to the buffer, properly aligned, and grows the buffer (if necessary).
   *
   * @param x A `double` to put into the buffer.
   */
  public fun addDouble(x: Double) {
    prep(Double.SIZE_BYTES, 0)
    putDouble(x)
  }

  /**
   * Adds on offset, relative to where it will be written.
   *
   * @param off The offset to add.
   */
  public fun addOffset(off: Int) {
    var off1 = off
    prep(Int.SIZE_BYTES, 0) // Ensure alignment is already done.
    off1 = offset() - off1 + Int.SIZE_BYTES
    putInt(off1)
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
   * <pre>`// Need 10 strings
   * FlatBufferBuilder builder = new FlatBufferBuilder(existingBuffer);
   * int[] offsets = new int[10];
   *
   * for (int i = 0; i < 10; i++) {
   * offsets[i] = fbb.createString(" " + i);
   * }
   *
   * // Have the strings in the buffer, but don't have a vector.
   * // Add a vector that references the newly created strings:
   * builder.startVector(4, offsets.length, 4);
   *
   * // Add each string to the newly created vector
   * // The strings are added in reverse order since the buffer
   * // is filled in back to front
   * for (int i = offsets.length - 1; i >= 0; i--) {
   * builder.addOffset(offsets[i]);
   * }
   *
   * // Finish off the vector
   * int offsetOfTheVector = fbb.endVector();
   `</pre> *
   *
   * @param elem_size The size of each element in the array.
   * @param num_elems The number of elements in the array.
   * @param alignment The alignment of the array.
   */
  public fun startVector(elem_size: Int, num_elems: Int, alignment: Int) {
    notNested()
    vectorNumElems = num_elems
    prep(Int.SIZE_BYTES, elem_size * num_elems)
    prep(alignment, elem_size * num_elems) // Just in case alignment > int.
    nested = true
  }

  /**
   * Finish off the creation of an array and all its elements.  The array
   * must be created with [.startVector].
   *
   * @return The offset at which the newly created array starts.
   * @see .startVector
   */
  public fun endVector(): Int {
    if (!nested) throw AssertionError("FlatBuffers: endVector called without startVector")
    nested = false
    putInt(vectorNumElems)
    return offset()
  }

  /**
   * Create a new array/vector and return a ByteBuffer to be filled later.
   * Call [endVector] after this method to get an offset to the beginning
   * of vector.
   *
   * @param elemSize the size of each element in bytes.
   * @param numElems number of elements in the vector.
   * @param alignment byte alignment.
   * @return ByteBuffer with position and limit set to the space allocated for the array.
   */
  public fun createUnintializedVector(elemSize: Int, numElems: Int, alignment: Int): ReadWriteBuffer {
    val length = elemSize * numElems
    startVector(elemSize, numElems, alignment)
    buffer.writePosition = length.let { space -= it; space }
    return buffer.writeSlice(buffer.writePosition, length)
  }

  /**
   * Create a vector of tables.
   *
   * @param offsets Offsets of the tables.
   * @return Returns offset of the vector.
   */
  public fun createVectorOfTables(offsets: IntArray): Int {
    notNested()
    startVector(Int.SIZE_BYTES, offsets.size, Int.SIZE_BYTES)
    for (i in offsets.indices.reversed()) addOffset(offsets[i])
    return endVector()
  }

  /**
   * Create a vector of sorted by the key tables.
   *
   * @param obj Instance of the table subclass.
   * @param offsets Offsets of the tables.
   * @return Returns offset of the sorted vector.
   */
  public fun <T : Table> createSortedVectorOfTables(obj: T, offsets: IntArray): Int {
    obj.sortTables(offsets, buffer)
    return createVectorOfTables(offsets)
  }

  /**
   * Encode the String `s` in the buffer using UTF-8. If a String with
   * this exact contents has already been serialized using this method,
   * instead simply returns the offset of the existing String.
   *
   * Usage of the method will incur into additional allocations,
   * so it is advisable to use it only when it is known upfront that
   * your message will have several repeated strings.
   *
   * @param s The String to encode.
   * @return The offset in the buffer where the encoded String starts.
   */
  public fun createSharedString(s: CharSequence): Int {
    if (stringPool == null) {
      stringPool = HashMap()
      val offset: Int = createString(s)
      stringPool!![s] = offset
      return offset
    }
    var offset: Int? = stringPool!![s]
    if (offset == null) {
      offset = createString(s)
      stringPool?.put(s, offset)
    }
    return offset
  }

  /**
   * Encode the [CharSequence] `s` in the buffer using UTF-8.
   * @param s The [CharSequence] to encode.
   * @return The offset in the buffer where the encoded string starts.
   */
  public fun createString(s: CharSequence): Int {
    val length: Int = Utf8.encodedLength(s)
    addByte(0.toByte())
    startVector(1, length, 1)
    buffer.writePosition = length.let { space -= it; space }
    buffer.put(s, length)
    return endVector()
  }

  /**
   * Create a string in the buffer from an already encoded UTF-8 string in a ByteBuffer.
   *
   * @param s An already encoded UTF-8 string as a `ByteBuffer`.
   * @return The offset in the buffer where the encoded string starts.
   */
 public fun createString(s: ReadBuffer): Int {
    val length: Int = s.limit
    addByte(0.toByte())
    startVector(1, length, 1)
    buffer.writePosition = length.let { space -= it; space }
    buffer.put(s)
    return endVector()
  }

  /**
   * Create a byte array in the buffer.
   *
   * @param arr A source array with data
   * @return The offset in the buffer where the encoded array starts.
   */
  public fun createByteVector(arr: ByteArray): Int {
    val length = arr.size
    startVector(1, length, 1)
    buffer.writePosition = length.let { space -= it; space }
    buffer.put(arr)
    return endVector()
  }

  /**
   * Create a byte array in the buffer.
   *
   * @param arr a source array with data.
   * @param offset the offset in the source array to start copying from.
   * @param length the number of bytes to copy from the source array.
   * @return The offset in the buffer where the encoded array starts.
   */
  public fun createByteVector(arr: ByteArray, offset: Int, length: Int): Int {
    startVector(1, length, 1)
    buffer.writePosition = length.let { space -= it; space }
    buffer.put(arr, offset, length)
    return endVector()
  }

  /**
   * Create a byte array in the buffer.
   *
   * The source [ReadBuffer] position is advanced until [ReadBuffer.limit]
   * after this call.
   *
   * @param data A source [ReadBuffer] with data.
   * @return The offset in the buffer where the encoded array starts.
   */
 public fun createByteVector(data: ReadBuffer, from: Int = 0, until: Int = data.limit): Int {
    val length: Int = until - from
    startVector(1, length, 1)
    buffer.writePosition = length.let { space -= it; space }
    buffer.put(data, from, until)
    return endVector()
  }

  /**
   * Should not be accessing the final buffer before it is finished.
   */
  public fun finished() {
    if (!finished) throw AssertionError(
      "FlatBuffers: you can only access the serialized buffer after it has been" +
        " finished by FlatBufferBuilder.finish()."
    )
  }

  /**
   * Should not be creating any other object, string or vector
   * while an object is being constructed.
   */
  public fun notNested() {
    if (nested) throw AssertionError("FlatBuffers: object serialization must not be nested.")
  }

  /**
   * Structures are always stored inline, they need to be created right
   * where they're used.  You'll get this assertion failure if you
   * created it elsewhere.
   *
   * @param obj The offset of the created object.
   */
  public fun nested(obj: Int) {
    if (obj != offset()) throw AssertionError("FlatBuffers: struct must be serialized inline.")
  }

  /**
   * Start encoding a new object in the buffer.  Users will not usually need to
   * call this directly. The `FlatBuffers` compiler will generate helper methods
   * that call this method internally.
   *
   *
   * For example, using the "Monster" code found on the "landing page". An
   * object of type `Monster` can be created using the following code:
   *
   * <pre>`int testArrayOfString = Monster.createTestarrayofstringVector(fbb, new int[] {
   * fbb.createString("test1"),
   * fbb.createString("test2")
   * });
   *
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
   `</pre> *
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
   *
   * @param numfields The number of fields found in this object.
   */
  public fun startTable(numfields: Int) {
    notNested()
    if (vtable == null || vtable!!.size < numfields) {
      vtable = IntArray(numfields)
    }
    vtableInUse = numfields
    vtable!!.fill(0, 0, vtableInUse)
    nested = true
    objectStart = offset()
  }

  /**
   * Add a `boolean` to a table at `o` into its vtable, with value `x` and default `d`.
   *
   * @param o The index into the vtable.
   * @param x A `boolean` to put into the buffer, depending on how defaults are handled. If
   * `force_defaults` is `false`, compare `x` against the default value `d`. If `x` contains the
   * default value, it can be skipped.
   * @param d A `boolean` default value to compare against when `force_defaults` is `false`.
   */
  public fun addBoolean(o: Int, x: Boolean, d: Boolean) {
    if (forceDefaults || x != d) {
      addBoolean(x)
      slot(o)
    }
  }

  /**
   * Add a `byte` to a table at `o` into its vtable, with value `x` and default `d`.
   *
   * @param o The index into the vtable.
   * @param x A `byte` to put into the buffer, depending on how defaults are handled. If
   * `force_defaults` is `false`, compare `x` against the default value `d`. If `x` contains the
   * default value, it can be skipped.
   * @param d A `byte` default value to compare against when `force_defaults` is `false`.
   */
  public fun addByte(o: Int, x: Byte, d: Int) {
    if (forceDefaults || x.toInt() != d) {
      addByte(x)
      slot(o)
    }
  }

  /**
   * Add a `short` to a table at `o` into its vtable, with value `x` and default `d`.
   *
   * @param o The index into the vtable.
   * @param x A `short` to put into the buffer, depending on how defaults are handled. If
   * `force_defaults` is `false`, compare `x` against the default value `d`. If `x` contains the
   * default value, it can be skipped.
   * @param d A `short` default value to compare against when `force_defaults` is `false`.
   */
  public fun addShort(o: Int, x: Short, d: Int) {
    if (forceDefaults || x.toInt() != d) {
      addShort(x)
      slot(o)
    }
  }

  /**
   * Add an `int` to a table at `o` into its vtable, with value `x` and default `d`.
   *
   * @param o The index into the vtable.
   * @param x An `int` to put into the buffer, depending on how defaults are handled. If
   * `force_defaults` is `false`, compare `x` against the default value `d`. If `x` contains the
   * default value, it can be skipped.
   * @param d An `int` default value to compare against when `force_defaults` is `false`.
   */
  public fun addInt(o: Int, x: Int, d: Int) {
    if (forceDefaults || x != d) {
      addInt(x)
      slot(o)
    }
  }

  /**
   * Add a `long` to a table at `o` into its vtable, with value `x` and default `d`.
   *
   * @param o The index into the vtable.
   * @param x A `long` to put into the buffer, depending on how defaults are handled. If
   * `force_defaults` is `false`, compare `x` against the default value `d`. If `x` contains the
   * default value, it can be skipped.
   * @param d A `long` default value to compare against when `force_defaults` is `false`.
   */
  public fun addLong(o: Int, x: Long, d: Long) {
    if (forceDefaults || x != d) {
      addLong(x)
      slot(o)
    }
  }

  /**
   * Add a `float` to a table at `o` into its vtable, with value `x` and default `d`.
   *
   * @param o The index into the vtable.
   * @param x A `float` to put into the buffer, depending on how defaults are handled. If
   * `force_defaults` is `false`, compare `x` against the default value `d`. If `x` contains the
   * default value, it can be skipped.
   * @param d A `float` default value to compare against when `force_defaults` is `false`.
   */
  public fun addFloat(o: Int, x: Float, d: Double) {
    if (forceDefaults || x.toDouble() != d) {
      addFloat(x)
      slot(o)
    }
  }

  /**
   * Add a `double` to a table at `o` into its vtable, with value `x` and default `d`.
   *
   * @param o The index into the vtable.
   * @param x A `double` to put into the buffer, depending on how defaults are handled. If
   * `force_defaults` is `false`, compare `x` against the default value `d`. If `x` contains the
   * default value, it can be skipped.
   * @param d A `double` default value to compare against when `force_defaults` is `false`.
   */
  public fun addDouble(o: Int, x: Double, d: Double) {
    if (forceDefaults || x != d) {
      addDouble(x)
      slot(o)
    }
  }

  /**
   * Add an `offset` to a table at `o` into its vtable, with value `x` and default `d`.
   *
   * @param o The index into the vtable.
   * @param x An `offset` to put into the buffer, depending on how defaults are handled. If
   * `force_defaults` is `false`, compare `x` against the default value `d`. If `x` contains the
   * default value, it can be skipped.
   * @param d An `offset` default value to compare against when `force_defaults` is `false`.
   */
  public fun addOffset(o: Int, x: Int, d: Int) {
    if (forceDefaults || x != d) {
      addOffset(x)
      slot(o)
    }
  }

  /**
   * Add a struct to the table. Structs are stored inline, so nothing additional is being added.
   *
   * @param voffset The index into the vtable.
   * @param x The offset of the created struct.
   * @param d The default value is always `0`.
   */
  public fun addStruct(voffset: Int, x: Int, d: Int) {
    if (x != d) {
      nested(x)
      slot(voffset)
    }
  }

  /**
   * Set the current vtable at `voffset` to the current location in the buffer.
   *
   * @param voffset The index into the vtable to store the offset relative to the end of the
   * buffer.
   */
  public fun slot(voffset: Int) {
    vtable!![voffset] = offset()
  }

  /**
   * Finish off writing the object that is under construction.
   *
   * @return The offset to the object inside [.dataBuffer].
   * @see .startTable
   */
  public fun endTable(): Int {
    if (vtable == null || !nested) throw AssertionError("FlatBuffers: endTable called without startTable")
    addInt(0)
    val vtableloc = offset()
    // Write out the current vtable.
    var i: Int = vtableInUse - 1
    // Trim trailing zeroes.
    while (i >= 0 && vtable!![i] == 0) {
      i--
    }
    val trimmed_size = i + 1
    while (i >= 0) {

      // Offset relative to the start of the table.
      val off = (if (vtable!![i] != 0) vtableloc - vtable!![i] else 0).toShort()
      addShort(off)
      i--
    }
    val standard_fields = 2 // The fields below:
    addShort((vtableloc - objectStart).toShort())
    addShort(((trimmed_size + standard_fields) * Short.SIZE_BYTES).toShort())

    // Search for an existing vtable that matches the current one.
    var existing_vtable = 0
    i = 0
    outer_loop@ while (i < numVtables) {
      val vt1: Int = buffer.capacity - vtables[i]
      val vt2 = space
      val len: Short = buffer.getShort(vt1)
      if (len == buffer.getShort(vt2)) {
        var j: Int = Short.SIZE_BYTES
        while (j < len) {
          if (buffer.getShort(vt1 + j) != buffer.getShort(vt2 + j)) {
            i++
            continue@outer_loop
          }
          j += Short.SIZE_BYTES
        }
        existing_vtable = vtables[i]
        break@outer_loop
      }
      i++
    }
    if (existing_vtable != 0) {
      // Found a match:
      // Remove the current vtable.
      space = buffer.capacity - vtableloc
      // Point table to existing vtable.
      buffer.set(space, existing_vtable - vtableloc)
    } else {
      // No match:
      // Add the location of the current vtable to the list of vtables.
      if (numVtables == vtables.size) vtables = vtables.copyOf(numVtables * 2)
      vtables[numVtables++] = offset()
      // Point table to current vtable.
      buffer.set(buffer.capacity - vtableloc, offset() - vtableloc)
    }
    nested = false
    return vtableloc
  }
  public fun endTable2(): Int {
    if (vtable == null || !nested) throw AssertionError("FlatBuffers: endTable called without startTable")
    addInt(0)
    val vtableloc = offset()
    // Write out the current vtable.
    var i = vtableInUse - 1
    // Trim trailing zeroes.
    while (i >= 0 && vtable!![i] == 0) {
      i--
    }
    val trimmed_size = i + 1
    while (i >= 0) {

      // Offset relative to the start of the table.
      val off = (if (vtable!![i] != 0) vtableloc - vtable!![i] else 0).toShort()
      addShort(off)
      i--
    }
    val standard_fields = 2 // The fields below:
    addShort((vtableloc - objectStart).toShort())
    addShort(((trimmed_size + standard_fields) * Short.SIZE_BYTES).toShort())

    // Search for an existing vtable that matches the current one.
    var existing_vtable = 0
    i = 0
    outer_loop@ while (i < numVtables) {
      val vt1: Int = buffer.capacity - vtables[i]
      val vt2 = space
      val len: Short = buffer.getShort(vt1)
      if (len == buffer.getShort(vt2)) {
        var j: Int = Short.SIZE_BYTES
        while (j < len) {
          if (buffer.getShort(vt1 + j) != buffer.getShort(vt2 + j)) {
            i++
            continue@outer_loop
          }
          j += Short.SIZE_BYTES
        }
        existing_vtable = vtables[i]
        break@outer_loop
      }
      i++
    }
    if (existing_vtable != 0) {
      // Found a match:
      // Remove the current vtable.
      space = buffer.limit - vtableloc
      // Point table to existing vtable.
      buffer.set(space, existing_vtable - vtableloc)
    } else {
      // No match:
      // Add the location of the current vtable to the list of vtables.
      if (numVtables == vtables.size) vtables = vtables.copyOf(numVtables * 2)
      vtables[numVtables++] = offset()
      // Point table to current vtable.
      buffer.set(buffer.capacity - vtableloc, offset() - vtableloc)
    }
    nested = false
    return vtableloc
  }

  /**
   * Checks that a required field has been set in a given table that has
   * just been constructed.
   *
   * @param table The offset to the start of the table from the `ByteBuffer` capacity.
   * @param field The offset to the field in the vtable.
   */
  public fun required(table: Int, field: Int) {
    val tableStart: Int = buffer.capacity - table
    val vtableStart: Int = tableStart - buffer.getInt(tableStart)
    val ok = buffer.getShort(vtableStart + field).toInt() != 0
    // If this fails, the caller will show what field needs to be set.
    if (!ok) throw AssertionError("FlatBuffers: field $field must be set")
  }

  /**
   * Finalize a buffer, pointing to the given `root_table`.
   *
   * @param root_table An offset to be added to the buffer.
   * @param size_prefix Whether to prefix the size to the buffer.
   */
  protected fun finish(root_table: Int, size_prefix: Boolean) {
    prep(minalign, Int.SIZE_BYTES + if (size_prefix) Int.SIZE_BYTES else 0)
    addOffset(root_table)
    if (size_prefix) {
      addInt(buffer.capacity - space)
    }
    buffer.writePosition = space
    finished = true
  }

  /**
   * Finalize a buffer, pointing to the given `root_table`.
   *
   * @param root_table An offset to be added to the buffer.
   */
  public fun finish(root_table: Int) {
    finish(root_table, false)
  }

  /**
   * Finalize a buffer, pointing to the given `root_table`, with the size prefixed.
   *
   * @param root_table An offset to be added to the buffer.
   */
  public fun finishSizePrefixed(root_table: Int) {
    finish(root_table, true)
  }

  /**
   * Finalize a buffer, pointing to the given `root_table`.
   *
   * @param root_table An offset to be added to the buffer.
   * @param file_identifier A FlatBuffer file identifier to be added to the buffer before
   * `root_table`.
   * @param size_prefix Whether to prefix the size to the buffer.
   */
  protected fun finish(root_table: Int, file_identifier: String, size_prefix: Boolean) {
    val FILE_IDENTIFIER_LENGTH = 4
    prep(minalign, Int.SIZE_BYTES + FILE_IDENTIFIER_LENGTH + if (size_prefix) Int.SIZE_BYTES else 0)
    if (file_identifier.length != FILE_IDENTIFIER_LENGTH) throw AssertionError(
      "FlatBuffers: file identifier must be length " +
        FILE_IDENTIFIER_LENGTH
    )
    for (i in FILE_IDENTIFIER_LENGTH - 1 downTo 0) {
      addByte(file_identifier[i].code.toByte())
    }
    finish(root_table, size_prefix)
  }

  /**
   * Finalize a buffer, pointing to the given `root_table`.
   *
   * @param rootTable An offset to be added to the buffer.
   * @param fileIdentifier A FlatBuffer file identifier to be added to the buffer before
   * `root_table`.
   */
  public fun finish(rootTable: Int, fileIdentifier: String) {
    finish(rootTable, fileIdentifier, false)
  }

  /**
   * Finalize a buffer, pointing to the given `root_table`, with the size prefixed.
   *
   * @param root_table An offset to be added to the buffer.
   * @param file_identifier A FlatBuffer file identifier to be added to the buffer before
   * `root_table`.
   */
  public fun finishSizePrefixed(root_table: Int, file_identifier: String) {
    finish(root_table, file_identifier, true)
  }

  /**
   * In order to save space, fields that are set to their default value
   * don't get serialized into the buffer. Forcing defaults provides a
   * way to manually disable this optimization.
   *
   * @param forceDefaults When set to `true`, always serializes default values.
   * @return Returns `this`.
   */
  public fun forceDefaults(forceDefaults: Boolean): FlatBufferBuilder {
    this.forceDefaults = forceDefaults
    return this
  }

  /**
   * Get the ByteBuffer representing the FlatBuffer. Only call this after you've
   * called `finish()`. The actual data starts at the ByteBuffer's current position,
   * not necessarily at `0`.
   *
   * @return The [ReadBuffer] representing the FlatBuffer
   */
  public fun dataBuffer(): ReadWriteBuffer {
    finished()
    return buffer
  }

  /**
   * A utility function to copy and return the ByteBuffer data as a `byte[]`.
   *
   * @return A full copy of the [data buffer][.dataBuffer].
   */
 public fun sizedByteArray(start: Int = space, length: Int = buffer.capacity - space): ByteArray {
    finished()
    val array = ByteArray(length)
    buffer.getBytes(array, start)
    return array
  }

  /**
   * Helper function to test if a field is present in the table
   *
   * @param offset virtual table offset
   * @return true if the filed is present
  */
  public fun Table.isFieldPresent(offset: Int): Boolean = this.offset(offset) != 0
}
