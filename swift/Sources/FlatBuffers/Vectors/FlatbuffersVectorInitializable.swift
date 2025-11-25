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

#if canImport(Common)
import Common
#endif

public protocol FlatbuffersVectorInitializable {
  static func readFrom(byteBuffer: ByteBuffer, index: Int) -> Self
}

extension Scalar where Self: FlatbuffersVectorInitializable {
  public static func readFrom(byteBuffer: ByteBuffer, index: Int) -> Self {
    byteBuffer.read(def: Self.self, position: index)
  }
}

extension NativeStruct where Self: FlatbuffersVectorInitializable {
  public static func readFrom(byteBuffer: ByteBuffer, index: Int) -> Self {
    byteBuffer.read(def: Self.self, position: index)
  }
}

extension FlatBufferStruct {
  public static func readFrom(byteBuffer: ByteBuffer, index: Int) -> Self {
    Self.init(byteBuffer, o: Int32(index))
  }
}

extension FlatBufferTable {
  public static func readFrom(byteBuffer: ByteBuffer, index: Int) -> Self {
    return Self.init(
      byteBuffer,
      o: Int32(index) &+ byteBuffer.read(def: Int32.self, position: index))
  }
}

extension Optional: FlatbuffersVectorInitializable where Wrapped == String {
  public static func readFrom(byteBuffer: ByteBuffer, index: Int) -> Self {
    var index = Int32(index)
    index &+= byteBuffer.read(def: Int32.self, position: Int(index))
    let count = byteBuffer.read(def: Int32.self, position: Int(index))
    let position = Int(index) &+ MemoryLayout<Int32>.size
    return byteBuffer.readString(at: position, count: Int(count))
  }
}

extension Enum where Self: FlatbuffersVectorInitializable {
  public static func readFrom(byteBuffer: ByteBuffer, index: Int) -> Self {
    Self
      .init(rawValue: byteBuffer.read(def: Self.T.self, position: index)) ??
      Self.min
  }
}
