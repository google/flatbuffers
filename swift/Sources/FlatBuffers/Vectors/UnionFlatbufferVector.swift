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

public struct UnionFlatbufferVector {

  private let bb: ByteBuffer
  private let byteSize: Int
  private let startPosition: Int32
  public let count: Int

  init(
    bb: ByteBuffer,
    startPosition: Int32,
    count: Int,
    byteSize: Int)
  {
    self.bb = bb
    self.byteSize = byteSize
    self.count = count
    self.startPosition = startPosition
  }

  public var startIndex: Int {
    0
  }

  public var endIndex: Int {
    count
  }

  public var isEmpty: Bool {
    count == 0
  }

  public subscript(
    position: Int,
    Type: FlatbuffersVectorInitializable
      .Type) -> FlatbuffersVectorInitializable
  {
    guard position < count else {
      fatalError(
        "Trying to read element at index \(position) in a vector of size \(count)")
    }
    let index = startPosition &+ Int32(position &* byteSize)
    return Type.readFrom(byteBuffer: bb, index: Int(index))
  }
}
