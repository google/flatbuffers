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

public struct Map: Sized {
  let byteBuffer: ByteBuffer
  let offset: Int
  let byteWidth: UInt8
  public let keys: TypedVector
  public let count: Int

  public var values: Vector {
    return Vector(byteBuffer: byteBuffer, offset: offset, byteWidth: byteWidth)
  }

  @inline(__always)
  init(byteBuffer: ByteBuffer, offset: Int, byteWidth: UInt8) {
    self.byteBuffer = byteBuffer
    self.offset = offset
    self.byteWidth = byteWidth

    count = getCount(buffer: byteBuffer, offset: offset, byteWidth: byteWidth)
    keys = TypedVector.mapKeys(
      byteBuffer: byteBuffer,
      offset: offset,
      byteWidth: byteWidth)
  }

  @inline(__always)
  public subscript(key: String) -> Reference? {
    guard let position = binarySearch(vector: keys, target: key)
    else { return nil }

    return getReference(at: position)
  }
}

extension Map {
  public func jsonBuilder(json: inout String) {
    json += "{"
    for i in 0..<count {
      if let key = keys[i]?.cString {
        let comma = i == count - 1 ? "" : ", "
        let value = values[i]?.jsonString() ?? StaticJSON.null
        json += "\"\(key)\": \(value)\(comma)"
      }
    }
    json += "}"
  }
}
