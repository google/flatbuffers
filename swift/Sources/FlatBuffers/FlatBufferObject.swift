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

/// NativeStruct is a protocol that indicates if the struct is a native `swift` struct
/// since now we will be serializing native structs into the buffer.
public protocol NativeStruct {}

/// FlatbuffersInitializable is a protocol that allows any object to be
/// Initialized from a ByteBuffer
public protocol FlatbuffersInitializable {
  init(_ bb: ByteBuffer, o: Int32)
}

/// FlatbufferObject structures all the Flatbuffers objects
public protocol FlatBufferObject: FlatbuffersInitializable {
  var __buffer: ByteBuffer! { get }
}

public protocol ObjectAPIPacker {
  associatedtype T
  static func pack(_ builder: inout FlatBufferBuilder, obj: inout T?) -> Offset
  static func pack(_ builder: inout FlatBufferBuilder, obj: inout T) -> Offset
  mutating func unpack() -> T
}

public protocol Enum {
  associatedtype T: Scalar
  static var byteSize: Int { get }
  var value: T { get }
}
