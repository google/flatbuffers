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
@file:JvmName("JVMByteArray")
@file:Suppress("NOTHING_TO_INLINE")

package com.google.flatbuffers.kotlin
import kotlin.experimental.and

/**
 * This implementation uses Little Endian order.
 */
public actual inline fun ByteArray.getUByte(index: Int): UByte = get(index).toUByte()
public actual inline fun ByteArray.getShort(index: Int): Short {
  return (this[index + 1].toInt() shl 8 or (this[index].toInt() and 0xff)).toShort()
}
public actual inline fun ByteArray.getUShort(index: Int): UShort = getShort(index).toUShort()

public actual inline fun ByteArray.getInt(index: Int): Int {
  return (
    (this[index + 3].toInt() shl 24) or
      ((this[index + 2].toInt() and 0xff) shl 16) or
      ((this[index + 1].toInt() and 0xff) shl 8) or
      ((this[index].toInt() and 0xff))
    )
}
public actual inline fun ByteArray.getUInt(index: Int): UInt = getInt(index).toUInt()

public actual inline fun ByteArray.getLong(index: Int): Long {
  var idx = index
  return this[idx++].toLong() and 0xff or
    (this[idx++].toLong() and 0xff shl 8) or
    (this[idx++].toLong() and 0xff shl 16) or
    (this[idx++].toLong() and 0xff shl 24) or
    (this[idx++].toLong() and 0xff shl 32) or
    (this[idx++].toLong() and 0xff shl 40) or
    (this[idx++].toLong() and 0xff shl 48) or
    (this[idx].toLong() shl 56)
}
public actual inline fun ByteArray.getULong(index: Int): ULong = getLong(index).toULong()

public actual inline fun ByteArray.setUByte(index: Int, value: UByte): Unit = set(index, value.toByte())
public actual inline fun ByteArray.setShort(index: Int, value: Short) {
  var idx = index
  this[idx++] = (value and 0xff).toByte()
  this[idx] = (value.toInt() shr 8 and 0xff).toByte()
}

public actual inline fun ByteArray.setUShort(index: Int, value: UShort): Unit = setShort(index, value.toShort())

public actual inline fun ByteArray.setInt(index: Int, value: Int) {
  var idx = index
  this[idx++] = (value and 0xff).toByte()
  this[idx++] = (value shr 8 and 0xff).toByte()
  this[idx++] = (value shr 16 and 0xff).toByte()
  this[idx] = (value shr 24 and 0xff).toByte()
}

public actual inline fun ByteArray.setUInt(index: Int, value: UInt): Unit = setInt(index, value.toInt())

public actual inline fun ByteArray.setLong(index: Int, value: Long) {
  var idx = index
  var i = value.toInt()
  this[idx++] = (i and 0xff).toByte()
  this[idx++] = (i shr 8 and 0xff).toByte()
  this[idx++] = (i shr 16 and 0xff).toByte()
  this[idx++] = (i shr 24 and 0xff).toByte()
  i = (value shr 32).toInt()
  this[idx++] = (i and 0xff).toByte()
  this[idx++] = (i shr 8 and 0xff).toByte()
  this[idx++] = (i shr 16 and 0xff).toByte()
  this[idx] = (i shr 24 and 0xff).toByte()
}

public actual inline fun ByteArray.setULong(index: Int, value: ULong): Unit = setLong(index, value.toLong())

public actual inline fun ByteArray.setFloat(index: Int, value: Float) {
  var idx = index
  val iValue: Int = value.toRawBits()
  this[idx++] = (iValue and 0xff).toByte()
  this[idx++] = (iValue shr 8 and 0xff).toByte()
  this[idx++] = (iValue shr 16 and 0xff).toByte()
  this[idx] = (iValue shr 24 and 0xff).toByte()
}

public actual inline fun ByteArray.setDouble(index: Int, value: Double) {
  var idx = index
  val lValue: Long = value.toRawBits()
  var i = lValue.toInt()
  this[idx++] = (i and 0xff).toByte()
  this[idx++] = (i shr 8 and 0xff).toByte()
  this[idx++] = (i shr 16 and 0xff).toByte()
  this[idx++] = (i shr 24 and 0xff).toByte()
  i = (lValue shr 32).toInt()
  this[idx++] = (i and 0xff).toByte()
  this[idx++] = (i shr 8 and 0xff).toByte()
  this[idx++] = (i shr 16 and 0xff).toByte()
  this[idx] = (i shr 24 and 0xff).toByte()
}

public actual inline fun ByteArray.getFloat(index: Int): Float = Float.fromBits(this.getInt(index))
public actual inline fun ByteArray.getDouble(index: Int): Double = Double.fromBits(this.getLong(index))
