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

import Foundation

extension String: FlatbuffersInitializable {

  /// Initailizes a string from a Flatbuffers ByteBuffer
  /// - Parameters:
  ///   - bb: ByteBuffer containing the readable string
  ///   - o: Current position
  public init(_ bb: ByteBuffer, o: Int32) {
    let count = bb.read(def: Int32.self, position: Int(o))
    self = bb.readString(
      at: Int32(MemoryLayout<Int32>.size) + o,
      count: count) ?? ""
  }
}

extension String: ObjectAPIPacker {

  public static func pack(_ builder: inout FlatBufferBuilder, obj: inout String?) -> Offset {
    guard var obj = obj else { return Offset() }
    return pack(&builder, obj: &obj)
  }

  public static func pack(_ builder: inout FlatBufferBuilder, obj: inout String) -> Offset {
    builder.create(string: obj)
  }

  public mutating func unpack() -> String {
    self
  }

}

extension String: NativeObject {

  public func serialize<T: ObjectAPIPacker>(type: T.Type) -> ByteBuffer where T.T == Self {
    fatalError("serialize should never be called from string directly")
  }

  public func serialize<T: ObjectAPIPacker>(builder: inout FlatBufferBuilder, type: T.Type) -> ByteBuffer where T.T == Self {
    fatalError("serialize should never be called from string directly")
  }
}
