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
    type: FlexBufferType)
  {
    self.byteBuffer = byteBuffer
    self.offset = offset
    self.byteWidth = byteWidth
    self.type = type
    count = getCount(buffer: byteBuffer, offset: offset, byteWidth: byteWidth)
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

    return TypedVector(
      byteBuffer: byteBuffer,
      offset: indirectOffset,
      byteWidth: numericCast(childByteWidth),
      type: .key)
  }
}

extension TypedVector {
  @inline(__always)
  func compare(offset off: Int, target: String) -> Int {

    let elementOffset = offset &+ (off &* numericCast(byteWidth))

    let indirectoffset = readIndirect(
      buffer: byteBuffer,
      offset: elementOffset,
      byteWidth)

    return byteBuffer.readWithUnsafeRawPointer(
      position: indirectoffset)
    { bufPointer in
      target.withCString { strPointer in
        Int(strcmp(bufPointer, strPointer))
      }
    }
  }
}

