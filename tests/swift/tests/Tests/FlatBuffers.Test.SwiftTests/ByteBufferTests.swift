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

  func testReadingDoubleBuffer() {
    let count = 8
    let array: [Double] = Array(repeating: 8.8, count: count)
    var oldBuffer = ByteBuffer(initialSize: 16)
    oldBuffer.push(elements: array)
    let underlyingBytes = oldBuffer.underlyingBytes
    let byteBuffer = ByteBuffer(bytes: underlyingBytes)
    let bufferPointer: UnsafeBufferPointer<Double> = byteBuffer.bufferPointer(
      index: 0,
      count: count)
    XCTAssertEqual(bufferPointer.count, count)
    underlyingBytes.withUnsafeBufferPointer { ptr in
      XCTAssertEqual(
        UnsafeRawPointer(ptr.baseAddress),
        UnsafeRawPointer(bufferPointer.baseAddress))
    }
    for pointee in bufferPointer.startIndex..<bufferPointer.endIndex {
      XCTAssertEqual(bufferPointer[pointee], 8.8)
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
    var oldBuffer = ByteBuffer(initialSize: 16)
    oldBuffer.push(elements: array)
    let underlyingBytes = oldBuffer.underlyingBytes
    let byteBuffer = ByteBuffer(bytes: underlyingBytes)
    let bufferPointer: UnsafeBufferPointer<MyGame_Example_Vec3> = byteBuffer
      .bufferPointer(
        index: 0,
        count: count)
    XCTAssertEqual(bufferPointer.count, count)
    underlyingBytes.withUnsafeBufferPointer { ptr in
      XCTAssertEqual(
        UnsafeRawPointer(ptr.baseAddress),
        UnsafeRawPointer(bufferPointer.baseAddress))
    }
    var start: Int = 8
    for pointee in bufferPointer.startIndex..<bufferPointer.endIndex {
      let obj = bufferPointer[pointee]
      XCTAssertEqual(Int(obj.test3.a), start)
      XCTAssertEqual(Int(obj.test3.b), start)
      start &*= 2
      XCTAssertTrue(
        MyGame_Example_Color.allCases
          .contains(where: { $0 == obj.test2 }))
    }

    let memory = byteBuffer.memory
    let pointer = UnsafeRawPointer(bufferPointer.baseAddress)
    for i in 0..<array.count {
      let off = i * MemoryLayout<MyGame_Example_Vec3>.size
      XCTAssertEqual(memory.advanced(by: off), pointer?.advanced(by: off))
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
