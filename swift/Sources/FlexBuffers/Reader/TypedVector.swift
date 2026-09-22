/*
 * Copyright 2024 Google Inc. All rights reserved.
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

import Foundation

public struct TypedVector: FlexBufferVector {
  public let byteBuffer: ByteBuffer
  public let offset: Int
  public let type: FlexBufferType
  public let count: Int
  public var isEmpty: Bool { count == 0 }

  let byteWidth: UInt8

  @inline(__always)
  init(
    byteBuffer: ByteBuffer,
    offset: Int,
    byteWidth: UInt8,
    type: FlexBufferType,
    count providedCount: Int? = nil)
  {
    self.byteBuffer = byteBuffer
    self.offset = offset
    self.byteWidth = byteWidth
    self.type = type
    count = providedCount
      ?? getCount(buffer: byteBuffer, offset: offset, byteWidth: byteWidth)
  }

  @inline(__always)
  public subscript(index: Int) -> Reference? {
    let elementOffset = offset &+ (numericCast(index) &* numericCast(byteWidth))
    return Reference(
      byteBuffer: byteBuffer,
      offset: elementOffset,
      parentWidth: byteWidth,
      byteWidth: 1,
      type: type)
  }

  @inline(__always)
  static func mapKeys(
    byteBuffer: ByteBuffer,
    offset: Int,
    byteWidth: UInt8) -> TypedVector
  {
    let prefixedFields = 3
    let keysOffset = offset &- (numericCast(byteWidth) &* prefixedFields)

    let indirectOffset = readIndirect(
      buffer: byteBuffer,
      offset: keysOffset,
      byteWidth)

    let childByteWidth = byteBuffer.readUInt64(
      offset: keysOffset &+ numericCast(byteWidth),
      byteWidth: byteWidth)

    // A corrupted buffer may report a child byte width that is not one of
    // the valid flexbuffers widths (1, 2, 4, 8), or a keys location that
    // lies outside of the buffer. Treat such a map as if it had no
    // readable keys, instead of reading out of bounds.
    guard
      let width = UInt8(exactly: childByteWidth),
      isValidByteWidth(width),
      indirectOffset >= numericCast(width),
      indirectOffset <= byteBuffer.count
    else {
      return TypedVector(
        byteBuffer: byteBuffer,
        offset: offset,
        byteWidth: 1,
        type: .key,
        count: 0)
    }

    return TypedVector(
      byteBuffer: byteBuffer,
      offset: indirectOffset,
      byteWidth: width,
      type: .key)
  }
}

extension TypedVector {
  /// Compares a key within the vector to a target string.
  ///
  /// The comparison is bounded by the end of the hosting buffer so that a
  /// corrupted buffer without a string terminator cannot cause an
  /// out-of-bounds read past the end of the buffer.
  @inline(__always)
  func compare(offset off: Int, target: String) -> Int {

    let elementOffset = offset &+ (off &* numericCast(byteWidth))

    let indirectoffset = readIndirect(
      buffer: byteBuffer,
      offset: elementOffset,
      byteWidth)

    guard indirectoffset >= 0, indirectoffset < byteBuffer.capacity else {
      return -1
    }

    let maxLength = byteBuffer.capacity &- indirectoffset
    return byteBuffer.readWithUnsafeRawPointer(
      position: indirectoffset)
    { bufPointer in
      target.withCString { strPointer in
        boundedStrcmp(bufPointer, strPointer, maxLength: maxLength)
      }
    }
  }
}

@inline(__always)
private func boundedStrcmp(
  _ lhs: UnsafeRawPointer,
  _ rhs: UnsafePointer<CChar>,
  maxLength: Int) -> Int
{
  var i = 0
  while i < maxLength {
    let l = UInt8(bitPattern: lhs.load(fromByteOffset: i, as: Int8.self))
    let r = UInt8(bitPattern: rhs[i])
    if l != r {
      return Int(l) &- Int(r)
    }
    if l == 0 {
      return 0
    }
    i &+= 1
  }
  // The readable bytes ran out before a terminator or a difference was
  // found: the target is either equal (it ended as well) or longer than
  // the readable bytes.
  return rhs[i] == 0 ? 0 : -1
}
