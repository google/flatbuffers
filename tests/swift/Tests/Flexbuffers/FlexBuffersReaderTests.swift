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

import Common
import XCTest

@testable import FlexBuffers

final class FlexBuffersReaderTests: XCTestCase {

  func testReadingProperBuffer() throws {
    let buf: ByteBuffer = createProperBuffer().byteBuffer
    try validate(buffer: buf)
  }

  func testReadingSizedBuffer() throws {
    let buf: ByteBuffer = createSizedBuffer()
    try validate(buffer: buf)
  }

  func testReset() throws {
    var fbx = FlexBuffersWriter(
      initialSize: 8,
      flags: .shareKeysAndStrings)
    write(fbx: &fbx)

    try validate(buffer: ByteBuffer(data: fbx.data))
    XCTAssertEqual(fbx.capacity, 512)
    fbx.reset()
    XCTAssertEqual(fbx.writerIndex, 0)
    XCTAssertEqual(fbx.capacity, 8)

    write(fbx: &fbx)
    try validate(buffer: ByteBuffer(data: fbx.data))
    fbx.reset(keepingCapacity: true)
    XCTAssertEqual(fbx.writerIndex, 0)
    XCTAssertEqual(fbx.capacity, 512)

    write(fbx: &fbx)
    try validate(buffer: ByteBuffer(data: fbx.data))
    XCTAssertEqual(fbx.capacity, 512)
  }

  private func validate(buffer buf: ByteBuffer) throws {
    let reference = try getRoot(buffer: buf)!
    XCTAssertEqual(reference.type, .map)
    let map = reference.map!
    XCTAssertEqual(map.count, 7)
    let vecRef = map["vec"]!
    XCTAssertEqual(vecRef.type, .vector)
    let vec = vecRef.vector!
    XCTAssertEqual(vec.count, 6)
    XCTAssertEqual(vec[0]?.type, .int)
    XCTAssertEqual(vec[0]?.int, -100)
    XCTAssertEqual(vec[1]?.type, .string)
    XCTAssertEqual(vec[1]?.cString, "Fred")
    XCTAssertNil(vec[1]?.int)
    XCTAssertEqual(vec[2]?.double, 4.0)
    XCTAssertTrue(vec[3]?.type == .blob)

    let blob = vec[3]!.blob { pointer in
      Array(pointer)
    }

    XCTAssertEqual(blob?.count, 1)
    XCTAssertEqual(blob?[0], 77)
    XCTAssertEqual(vec[4]?.type, .bool)
    XCTAssertEqual(vec[4]?.bool, false)
    XCTAssertEqual(vec[5]?.double, 4.0)  // Shared with vec[2]

    let barVec = map["bar"]!.typedVector!
    XCTAssertEqual(barVec.count, 3)
    XCTAssertEqual(barVec[2]?.int, 3)
    XCTAssertEqual(barVec[2]?.asInt(), UInt8(3))

    let fixedVec = map["bar3"]!.fixedTypedVector!
    XCTAssertEqual(fixedVec.count, 3)
    XCTAssertEqual(fixedVec[2]?.int, 3)
    XCTAssertEqual(fixedVec[2]?.asInt(), UInt8(3))
    XCTAssertEqual(map["bool"]?.bool, true)

    let boolsVector = map["bools"]!.typedVector!
    XCTAssertEqual(boolsVector.type, .bool)
    XCTAssertEqual(boolsVector[0]?.bool, true)
    XCTAssertEqual(boolsVector[1]?.bool, false)

    let bools = [true, false, true, false]
    boolsVector.withUnsafeRawBufferPointer { buff in
      for i in 0..<boolsVector.count {
        XCTAssertEqual(buff.load(fromByteOffset: i, as: Bool.self), bools[i])
      }
    }
    XCTAssertEqual(map["foo"]?.double, 100)
    XCTAssertNil(map["unknown"])
    let mymap = map["mymap"]?.map

    // Check if both addresses used are the same for keys and strings
    XCTAssertEqual(mymap?.keys[0]?.cString, map.keys[4]?.cString)
    map.keys[4]?.withUnsafeRawPointer { pointer in
      mymap?.keys[0]?.withUnsafeRawPointer { mymapPointer in
        XCTAssertEqual(pointer, mymapPointer)
      }
    }

    XCTAssertEqual(mymap?.values[0]?.cString, vec[1]?.cString)
    vec[1]?.withUnsafeRawPointer { pointer in
      mymap?.values[0]?.withUnsafeRawPointer { mymapPointer in
        XCTAssertEqual(pointer, mymapPointer)
      }
    }
  }

  private var path: String {
    #if os(macOS)
      // Gets the current path of this test file then
      // strips out the nested directories.
      let filePath = URL(filePath: #file)
        .deletingLastPathComponent()
        .deletingLastPathComponent()
        .deletingLastPathComponent()
      return filePath.absoluteString
    #else
      return FileManager.default.currentDirectoryPath
    #endif
  }

}
