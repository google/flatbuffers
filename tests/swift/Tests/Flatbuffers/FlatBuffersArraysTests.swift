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

#if compiler(>=6.2)

import XCTest

@testable import FlatBuffers

@available(macOS 26.0, iOS 26.0, watchOS 26.0, tvOS 26.0, *)
final class FlatBuffersArraysTests: XCTestCase {

  func testStructSizes() {
    XCTAssertEqual(MemoryLayout<MyGame_Example_NestedStruct>.size, 32)
    XCTAssertEqual(MemoryLayout<MyGame_Example_ArrayStruct>.size, 160)
  }

  func testGoldenData() {
    // swiftformat:disable all
    let data: [UInt8] = [
      20, 0, 0, 0, 65, 82, 82, 84, 0, 0, 0, 0, 0, 0, 6, 0, 164, 0, 4, 0, 6,
      0, 0, 0, 164, 112, 69, 65, 1, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 4, 0,
      0, 0, 5, 0, 0, 0, 6, 0, 0, 0, 7, 0, 0, 0, 8, 0, 0, 0, 9, 0, 0, 0, 10,
      0, 0, 0, 11, 0, 0, 0, 12, 0, 0, 0, 13, 0, 0, 0, 14, 0, 0, 0, 15, 0, 0,
      0, 129, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 2, 0, 0, 0, 0, 2, 1,
      0, 0, 0, 0, 0, 136, 119, 102, 85, 68, 51, 34, 17, 120, 136, 153, 170,
      187, 204, 221, 238, 3, 0, 0, 0, 252, 255, 255, 255, 1, 1, 0, 0, 0, 0,
      0, 0, 120, 136, 153, 170, 187, 204, 221, 238, 136, 119, 102, 85, 68,
      51, 34, 17, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 255,
      255, 255, 255, 255, 255, 255, 127
    ]
    // swiftformat:enable all

    XCTAssertEqual(data, createArrayTable())
  }

  func testData() throws {
    var buf = ByteBuffer(bytes: createArrayTable())
    let table: MyGame_Example_ArrayTable = try getCheckedRoot(
      byteBuffer: &buf,
      fileId: "ARRT")
    verifyNativeStruct(a: table.a)
    verifyMutations(in: table)
    verifyNativeStruct(a: table.a)
  }

  func testObjectAPI() throws {
    var buf = ByteBuffer(bytes: createArrayTable())
    let table: MyGame_Example_ArrayTable = try getCheckedRoot(
      byteBuffer: &buf,
      fileId: "ARRT")
    verifyNativeStruct(a: table.unpack().a)
  }

  func testDefaults() {
    XCTAssertEqual(
      MyGame_Example_NestedStruct(),
      MyGame_Example_NestedStruct(
        a: [0, 0],
        b: .a,
        c: [0, 0],
        d: [0, 0]))
  }

  func verifyNativeStruct(a: MyGame_Example_ArrayStruct?) {
    let a = a!
    XCTAssertEqual(a.a, 12.34)
    XCTAssertEqual(a.b.count, 15)
    var sum: Int32 = 0
    for i in a.b.startIndex..<a.b.endIndex {
      sum += a.b[i]
    }
    XCTAssertEqual(sum, 120)
    XCTAssertEqual(a.c, -127)
    XCTAssertEqual(a.d.count, 2)
    let nestedStruct1 = a.d[0]
    XCTAssertEqual(nestedStruct1.a.toArray(), [-1, 2])
    XCTAssertEqual(nestedStruct1.b, .a)
    XCTAssertEqual(nestedStruct1.c.toArray(), [.c, .b])
    XCTAssertEqual(
      nestedStruct1.d.toArray(),
      [0x1122334455667788, -0x1122334455667788])

    let nestedStruct2 = a.d[1]
    XCTAssertEqual(nestedStruct2.a.toArray(), [3, -4])
    XCTAssertEqual(nestedStruct2.b, .b)
    XCTAssertEqual(nestedStruct2.c.toArray(), [.b, .a])
    XCTAssertEqual(
      nestedStruct2.d.toArray(),
      [-0x1122334455667788, 0x1122334455667788])

    XCTAssertEqual(a.e, 1)
    XCTAssertEqual(a.f.count, 2)
  }

  func verifyMutations(in table: MyGame_Example_ArrayTable) {
    let a = table.mutableA!
    XCTAssertEqual(a.a, 12.34)
    XCTAssertEqual(a.b.count, 15)
    var sum: Int32 = 0
    for i in a.b.startIndex..<a.b.endIndex {
      sum += a.b[i]
    }
    XCTAssertEqual(sum, 120)
    XCTAssertEqual(a.c, -127)
    XCTAssertEqual(a.d.count, 2)
    let nestedStruct1 = a.d[0]

    XCTAssertEqual(nestedStruct1.a.reduce(0) { $0 + $1 }, 1)
    XCTAssertEqual(nestedStruct1.b, .a)
    XCTAssertEqual(nestedStruct1.c[0], .c)
    XCTAssertEqual(nestedStruct1.c[1], .b)

    let nestedStruct2 = a.d[1]
    XCTAssertEqual(nestedStruct2.a.reduce(0) { $0 + $1 }, -1)
    XCTAssertEqual(nestedStruct2.b, .b)
    XCTAssertEqual(nestedStruct2.c[0], .b)
    XCTAssertEqual(nestedStruct2.c[1], .a)
    XCTAssertEqual(nestedStruct2.d[0], -0x1122334455667788)
    XCTAssertEqual(nestedStruct2.d[1], 0x1122334455667788)

    XCTAssertTrue(a.mutate(b: 1000, at: 0))
    XCTAssertTrue(a.mutate(b: 2000, at: 1))

    XCTAssertTrue(nestedStruct2.mutate(c: .a, at: 0))
    XCTAssertTrue(nestedStruct2.mutate(c: .b, at: 1))

    XCTAssertEqual(nestedStruct2.c[0], .a)
    XCTAssertEqual(nestedStruct2.c[1], .b)

    XCTAssertTrue(nestedStruct2.mutate(d: 0, at: 0))
    XCTAssertTrue(nestedStruct2.mutate(d: 0, at: 1))

    XCTAssertEqual(nestedStruct2.d.reduce(0) { $0 + $1 }, 0)

    let nativeStruct = table.a?.d[1]

    XCTAssertEqual(nativeStruct?.c[0], .a)
    XCTAssertEqual(nativeStruct?.c[1], .b)

    XCTAssertEqual(nativeStruct?.d[0], 0)
    XCTAssertEqual(nativeStruct?.d[1], 0)

    XCTAssertTrue(a.mutate(b: 1, at: 0))
    XCTAssertTrue(a.mutate(b: 2, at: 1))

    XCTAssertTrue(nestedStruct2.mutate(c: .b, at: 0))
    XCTAssertTrue(nestedStruct2.mutate(c: .a, at: 1))

    XCTAssertTrue(nestedStruct2.mutate(d: -0x1122334455667788, at: 0))
    XCTAssertTrue(nestedStruct2.mutate(d: 0x1122334455667788, at: 1))
  }

  private func createArrayTable() -> [UInt8] {
    var builder = FlatBufferBuilder(initialSize: 1024)

    let nestedStruct1 = MyGame_Example_NestedStruct(
      a: [-1, 2],
      b: .a,
      c: [
        MyGame_Example_TestEnum.c.rawValue,
        MyGame_Example_TestEnum.b.rawValue,
      ],
      d: [0x1122334455667788, -0x1122334455667788])

    let nestedStruct2 = MyGame_Example_NestedStruct(
      a: [3, -4],
      b: .b,
      c: [
        MyGame_Example_TestEnum.b.rawValue,
        MyGame_Example_TestEnum.a.rawValue,
      ],
      d: [-0x1122334455667788, 0x1122334455667788])

    let arrayStruct = MyGame_Example_ArrayStruct(
      a: 12.34,
      b: [1, 2, 3, 4, 5, 6, 7, 8, 9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF],
      c: -127,
      d: [nestedStruct1, nestedStruct2],
      e: 1,
      f: [-0x8000000000000000, 0x7FFFFFFFFFFFFFFF])

    let arrayTable = MyGame_Example_ArrayTable.createArrayTable(
      &builder,
      a: arrayStruct)
    builder.finish(offset: arrayTable, fileId: "ARRT")

    return builder.sizedByteArray
  }
}

@available(macOS 26.0, iOS 26.0, watchOS 26.0, tvOS 26.0, *)
extension MyGame_Example_NestedStruct: Equatable {
  public static func == (
    lhs: MyGame_Example_NestedStruct,
    rhs: MyGame_Example_NestedStruct) -> Bool
  {
    lhs.a == rhs.a && lhs.c == rhs.c && lhs.d == rhs.d && lhs.b == rhs.b
  }
}

@available(macOS 26.0, iOS 26.0, watchOS 26.0, tvOS 26.0, *)
extension InlineArray: @retroactive Equatable where Element: Equatable {
  public static func == (lhs: Self, rhs: Self) -> Bool {
    guard lhs.count == rhs.count else { return false }

    for i in 0..<lhs.count {
      if lhs[i] != rhs[i] { return false }
    }
    return true
  }

  func toArray() -> [Element] {
    var result: [Element] = []
    for i in startIndex..<endIndex {
      result.append(self[i])
    }
    return result
  }
}

#endif

