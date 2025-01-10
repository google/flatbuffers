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

import XCTest
@testable import FlatBuffers

final class ByteBufferTests: XCTestCase {
  func testCopyingMemory() {
    let count = 100
    let ptr = UnsafeMutableRawPointer.allocate(byteCount: count, alignment: 1)
    let byteBuffer = ByteBuffer(copyingMemoryBound: ptr, capacity: count)
    XCTAssertNotEqual(byteBuffer.memory, ptr)
  }

  func testSamePointer() {
    let count = 100
    let ptr = UnsafeMutableRawPointer.allocate(byteCount: count, alignment: 1)
    let byteBuffer = ByteBuffer(assumingMemoryBound: ptr, capacity: count)
    XCTAssertEqual(byteBuffer.memory, ptr)
  }

  func testSameDataPtr() {
    let count = 100
    let ptr = Data(repeating: 0, count: count)
    let byteBuffer = ByteBuffer(data: ptr)
    ptr.withUnsafeBytes { ptr in
      XCTAssertEqual(byteBuffer.memory, ptr.baseAddress)
    }
  }

  func testSameArrayPtr() {
    let count = 100
    let ptr: [UInt8] = Array(repeating: 0, count: count)
    let byteBuffer = ByteBuffer(bytes: ptr)
    ptr.withUnsafeBytes { ptr in
      XCTAssertEqual(byteBuffer.memory, ptr.baseAddress)
    }
  }
}
