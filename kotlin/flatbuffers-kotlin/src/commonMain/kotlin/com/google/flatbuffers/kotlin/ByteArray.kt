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

internal fun ByteArray.getString(index: Int, size: Int): String = Utf8.decodeUtf8Array(this, index, size)

internal fun ByteArray.setString(index: Int, value: String): Int =
  Utf8.encodeUtf8Array(value, this, index, this.size - index)

// List of functions that needs to be implemented on all platforms.
internal expect inline fun ByteArray.getUByte(index: Int): UByte
internal expect inline fun ByteArray.getShort(index: Int): Short
internal expect inline fun ByteArray.getUShort(index: Int): UShort
internal expect inline fun ByteArray.getInt(index: Int): Int
internal expect inline fun ByteArray.getUInt(index: Int): UInt
internal expect inline fun ByteArray.getLong(index: Int): Long
internal expect inline fun ByteArray.getULong(index: Int): ULong
internal expect inline fun ByteArray.getFloat(index: Int): Float
internal expect inline fun ByteArray.getDouble(index: Int): Double

internal expect inline fun ByteArray.setUByte(index: Int, value: UByte)
internal expect inline fun ByteArray.setShort(index: Int, value: Short)
internal expect inline fun ByteArray.setUShort(index: Int, value: UShort)
internal expect inline fun ByteArray.setInt(index: Int, value: Int)
internal expect inline fun ByteArray.setUInt(index: Int, value: UInt)
internal expect inline fun ByteArray.setLong(index: Int, value: Long)
internal expect inline fun ByteArray.setULong(index: Int, value: ULong)
internal expect inline fun ByteArray.setFloat(index: Int, value: Float)
internal expect inline fun ByteArray.setDouble(index: Int, value: Double)
