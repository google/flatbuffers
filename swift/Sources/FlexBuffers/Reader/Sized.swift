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

protocol Sized {
  var byteBuffer: ByteBuffer { get }
  var offset: Int { get }
  var byteWidth: UInt8 { get }
  var count: Int { get }
}

extension Sized {

  @inline(__always)
  func getReference(at index: Int) -> Reference? {
    if index >= count { return nil }
    let bWidth = Int(byteWidth)

    let packedType = byteBuffer.read(
      def: UInt8.self,
      position: (offset &+ (count &* bWidth)) &+ index)

    let offset = offset &+ (index &* bWidth)

    return Reference(
      byteBuffer: byteBuffer,
      offset: offset,
      parentWidth: byteWidth,
      packedType: packedType)
  }
}
