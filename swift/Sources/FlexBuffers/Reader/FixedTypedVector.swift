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

public struct FixedTypedVector: FlexBufferVector {
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
    count: Int)
  {
    self.byteBuffer = byteBuffer
    self.offset = offset
    self.byteWidth = byteWidth
    self.type = type
    self.count = count
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
}
