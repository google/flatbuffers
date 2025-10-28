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
    byteBuffer.withUnsafeBytes { memory in
      XCTAssertNotEqual(memory.baseAddress, ptr)
    }
  }

  func testSamePointer() {
    let count = 100
    let ptr = UnsafeMutableRawPointer.allocate(byteCount: count, alignment: 1)
    let byteBuffer = ByteBuffer(assumingMemoryBound: ptr, capacity: count)
    byteBuffer.withUnsafeBytes { memory in
      XCTAssertEqual(memory.baseAddress!, ptr)
    }
  }

  func testSameDataPtr() {
    let count = 100
    let ptr = Data(repeating: 0, count: count)
    let byteBuffer = ByteBuffer(data: ptr)
    byteBuffer.withUnsafeBytes { memory in
      ptr.withUnsafeBytes { ptr in
        XCTAssertEqual(memory.baseAddress!, ptr.baseAddress!)
      }
    }
  }

  func testSameArrayPtr() {
    let count = 100
    let ptr: [UInt8] = Array(repeating: 0, count: count)
    let byteBuffer = ByteBuffer(bytes: ptr)
    ptr.withUnsafeBytes { ptr in
      byteBuffer.withUnsafeBytes { memory in
        XCTAssertEqual(memory.baseAddress, ptr.baseAddress)
      }
    }
  }

  func testReadingDoubleBuffer() {
    let count = 8
    let array: [Double] = Array(repeating: 8.8, count: count)
    var oldBuffer = _InternalByteBuffer(initialSize: 16)
    oldBuffer.push(elements: array)
    let bytes: [Byte] = oldBuffer.withUnsafeBytes { bytes in
      Array(bytes)
    }
    let byteBuffer = ByteBuffer(bytes: bytes)
    byteBuffer.withUnsafePointerToSlice(index: 0, count: count) { ptr in
      XCTAssertEqual(ptr.count, count)
      bytes.withUnsafeBufferPointer {
        XCTAssertEqual(
          UnsafeRawPointer($0.baseAddress),
          UnsafeRawPointer(ptr.baseAddress))
      }
    }
  }

  func testReadingNativeStructs() {
    let array = [
      MyGame_Example_Vec3(
        x: 3.2,
        y: 3.2,
        z: 3.2,
        test1: 8,
        test2: .red,
        test3: MyGame_Example_Test(a: 8, b: 8)),
      MyGame_Example_Vec3(
        x: 3.2,
        y: 3.2,
        z: 3.2,
        test1: 8,
        test2: .green,
        test3: MyGame_Example_Test(a: 16, b: 16)),
      MyGame_Example_Vec3(
        x: 3.2,
        y: 3.2,
        z: 3.2,
        test1: 8,
        test2: .blue,
        test3: MyGame_Example_Test(a: 32, b: 32)),
    ]
    let count = array.count
    var oldBuffer = _InternalByteBuffer(initialSize: 16)
    oldBuffer.push(elements: array)
    let bytes: [Byte] = oldBuffer.withUnsafeBytes { bytes in
      Array(bytes)
    }
    let byteBuffer = ByteBuffer(bytes: bytes)
    byteBuffer
      .withUnsafePointerToSlice(index: 0, count: count) { bufferPointer in
        XCTAssertEqual(bufferPointer.count, count)
        bytes.withUnsafeBufferPointer { ptr in
          XCTAssertEqual(
            UnsafeRawPointer(ptr.baseAddress),
            UnsafeRawPointer(bufferPointer.baseAddress))
        }
      }
  }
}

private struct TestNativeStructs: NativeStruct {
  let x: Double
  let y: Double
  let z: Int
}

extension MyGame_Example_Color: CaseIterable {
  public static var allCases: [MyGame_Example_Color] = [.red, .blue, .green]
}
