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

protocol FlexBufferVector: Sized & FlexBufferContiguousBytes {
  subscript(index: Int) -> Reference? { get }
}

extension FlexBufferVector {
  public func jsonBuilder(json: inout String) {
    json += "["
    for i in 0..<count {
      if let val = self[i]?.jsonString() {
        let comma = i == count - 1 ? "" : ", "
        json += "\(val)\(comma)"
      }
    }
    json += "]"
  }
}

public protocol FlexBufferContiguousBytes {
  var byteBuffer: ByteBuffer { get }
  var offset: Int { get }
  var count: Int { get }

  func withUnsafeRawBufferPointer<Result>(
    _ body: (UnsafeRawBufferPointer) throws -> Result) rethrows -> Result
}

extension FlexBufferContiguousBytes {
  public func withUnsafeRawBufferPointer<Result>(
    _ body: (UnsafeRawBufferPointer) throws -> Result) rethrows -> Result
  {
    try byteBuffer.withUnsafePointerToSlice(
      index: offset,
      count: count,
      body: body)
  }
}
