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
import Foundation
import Testing

@testable import FlexBuffers

struct FlexBuffersReaderTests {

  @Test
  func testReadingProperBuffer() throws {
    let buf: ByteBuffer = createProperBuffer().byteBuffer
    try validate(buffer: buf)
  }

  @Test
  func testReadingSizedBuffer() throws {
    let buf: ByteBuffer = createSizedBuffer()
    try validate(buffer: buf)
  }

  @Test(.bug("https://github.com/google/flatbuffers/issues/8642"))
  func testReset() throws {
    var fbx = FlexBuffersWriter(
      initialSize: 8,
      flags: .shareKeysAndStrings)
    write(fbx: &fbx)

    try validate(buffer: ByteBuffer(data: fbx.data))
    #expect(fbx.capacity == 512)
    fbx.reset()
    #expect(fbx.writerIndex == 0)
    #expect(fbx.capacity == 8)

    write(fbx: &fbx)
    try validate(buffer: ByteBuffer(data: fbx.data))
    fbx.reset(keepingCapacity: true)
    #expect(fbx.writerIndex == 0)
    #expect(fbx.capacity == 512)

    write(fbx: &fbx)
    try validate(buffer: ByteBuffer(data: fbx.data))
    #expect(fbx.capacity == 512)
  }

  private func validate(buffer buf: ByteBuffer) throws {
    let reference = try getRoot(buffer: buf)!
    #expect(reference.type == .map)
    let map = reference.map!
    #expect(map.count == 7)
    let vecRef = map["vec"]!
    #expect(vecRef.type == .vector)
    let vec = vecRef.vector!
    #expect(vec.count == 6)
    #expect(vec[0]?.type == .int)
    #expect(vec[0]?.int == -100)
    #expect(vec[1]?.type == .string)
    #expect(vec[1]?.cString == "Fred")
    #expect(vec[1]?.int == nil)
    #expect(vec[2]?.double == 4.0)
    #expect(vec[3]?.type == .blob)

    let blob = vec[3]!.blob { pointer in
      Array(pointer)
    }

    #expect(blob?.count == 1)
    #expect(blob?[0] == 77)
    #expect(vec[4]?.type == .bool)
    #expect(vec[4]?.bool == false)
    #expect(vec[5]?.double == 4.0)  // Shared with vec[2]

    let barVec = map["bar"]!.typedVector!
    #expect(barVec.count == 3)
    #expect(barVec[2]?.int == 3)
    #expect(barVec[2]?.asInt() == UInt8(3))

    let fixedVec = map["bar3"]!.fixedTypedVector!
    #expect(fixedVec.count == 3)
    #expect(fixedVec[2]?.int == 3)
    #expect(fixedVec[2]?.asInt() == UInt8(3))
    #expect(map["bool"]?.bool == true)

    let boolsVector = map["bools"]!.typedVector!
    #expect(boolsVector.type == .bool)
    #expect(boolsVector[0]?.bool == true)
    #expect(boolsVector[1]?.bool == false)

    let bools = [true, false, true, false]
    boolsVector.withUnsafeRawBufferPointer { buff in
      for i in 0..<boolsVector.count {
        #expect(buff.load(fromByteOffset: i, as: Bool.self) == bools[i])
      }
    }
    #expect(map["foo"]?.double == 100)
    #expect(map["unknown"] == nil)
    let mymap = map["mymap"]?.map

    // Check if both addresses used are the same for keys and strings
    #expect(mymap?.keys[0]?.cString == map.keys[4]?.cString)
    map.keys[4]?.withUnsafeRawPointer { pointer in
      mymap?.keys[0]?.withUnsafeRawPointer { mymapPointer in
        #expect(pointer == mymapPointer)
      }
    }

    #expect(mymap?.values[0]?.cString == vec[1]?.cString)
    vec[1]?.withUnsafeRawPointer { pointer in
      mymap?.values[0]?.withUnsafeRawPointer { mymapPointer in
        #expect(pointer == mymapPointer)
      }
    }
  }

  private var path: String {
    #if os(macOS)
    // Gets the current path of this test file then
    // strips out the nested directories.
    let filePath = URL(filePath: #filePath)
      .deletingLastPathComponent()
      .deletingLastPathComponent()
      .deletingLastPathComponent()
    return filePath.absoluteString
    #else
    return FileManager.default.currentDirectoryPath
    #endif
  }
}
