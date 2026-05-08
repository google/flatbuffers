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
import Testing

@testable import FlatBuffers

typealias Test = MyGame_Example_Test
typealias Monster = MyGame_Example_Monster
typealias Vec3 = MyGame_Example_Vec3
typealias Stat = MyGame_Example_Stat

struct FlatBuffersMonsterWriterTests {

  @Test
  func testData() {
    // swiftformat:disable all
    let data = Data([
      48, 0, 0, 0, 77, 79, 78, 83, 0, 0, 0, 0, 36, 0, 72, 0, 40, 0, 0, 0, 38, 0, 32, 0, 0, 0, 28, 0,
      0, 0, 27, 0, 20, 0, 16, 0, 12, 0, 4, 0, 0, 0, 0, 0, 0, 0, 11, 0, 36, 0, 0, 0, 164, 0, 0, 0, 0,
      0, 0, 1, 60, 0, 0, 0, 68, 0, 0, 0, 76, 0, 0, 0, 0, 0, 0, 1, 88, 0, 0, 0, 120, 0, 0, 0, 0, 0,
      80, 0, 0, 0, 128, 63, 0, 0, 0, 64, 0, 0, 64, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 64, 2, 0, 5,
      0, 6, 0, 0, 0, 2, 0, 0, 0, 64, 0, 0, 0, 48, 0, 0, 0, 2, 0, 0, 0, 30, 0, 40, 0, 10, 0, 20, 0,
      152, 255, 255, 255, 4, 0, 0, 0, 4, 0, 0, 0, 70, 114, 101, 100, 0, 0, 0, 0, 5, 0, 0, 0, 0, 1,
      2, 3, 4, 0, 0, 0, 5, 0, 0, 0, 116, 101, 115, 116, 50, 0, 0, 0, 5, 0, 0, 0, 116, 101, 115, 116,
      49, 0, 0, 0, 9, 0, 0, 0, 77, 121, 77, 111, 110, 115, 116, 101, 114, 0, 0, 0, 3, 0, 0, 0, 20,
      0, 0, 0, 36, 0, 0, 0, 4, 0, 0, 0, 240, 255, 255, 255, 32, 0, 0, 0, 248, 255, 255, 255, 36, 0,
      0, 0, 12, 0, 8, 0, 0, 0, 0, 0, 0, 0, 4, 0, 12, 0, 0, 0, 28, 0, 0, 0, 5, 0, 0, 0, 87, 105, 108,
      109, 97, 0, 0, 0, 6, 0, 0, 0, 66, 97, 114, 110, 101, 121, 0, 0, 5, 0, 0, 0, 70, 114, 111, 100,
      111, 0, 0, 0,
    ])
    // swiftformat:enable all
    let _data = ByteBuffer(data: data)
    readVerifiedMonster(fb: _data)
  }

  @Test
  func testReadFromOtherLanguages() {
    let path = {
      #if os(macOS)
      // Gets the current path of this test file then
      // strips out the nested directories.
      let filePath = URL(filePath: #filePath)
        .deletingLastPathComponent()
      return filePath.absoluteString
      #else
      return FileManager.default.currentDirectoryPath
        .appending("/tests/swift/Tests/Flatbuffers")
      #endif
    }()

    let url = URL(string: path)!
      .appendingPathComponent("monsterdata_test")
      .appendingPathExtension("mon")

    let data = FileManager.default.contents(atPath: url.path)!
    let _data = ByteBuffer(data: data)

    readVerifiedMonster(fb: _data)
  }

  @Test
  func testCreateMonsterData() {
    let bytes = createMonster(withPrefix: false)
    var buffer = ByteBuffer(data: bytes.data)
    let monster: MyGame_Example_Monster = getRoot(byteBuffer: &buffer)
    readMonster(monster: monster)
    mutateMonster(fb: bytes.buffer)
    readMonster(monster: monster)
  }

  @Test(.bug("https://github.com/google/flatbuffers/issues/8642"))
  func testCreateMonsterResetTests() {
    var builder = createMonster(withPrefix: false)
    var buffer = ByteBuffer(data: builder.data)
    let monster: MyGame_Example_Monster = getRoot(byteBuffer: &buffer)
    readMonster(monster: monster)
    builder.clear()
    #expect(builder.capacity == 1)
    #expect(builder.size == 0)

    write(fbb: &builder, prefix: false)
    var _buffer = ByteBuffer(data: builder.data)
    #expect(_buffer.capacity == 304)
    let _monster: MyGame_Example_Monster = getRoot(byteBuffer: &_buffer)
    readMonster(monster: _monster)
    builder.clear(keepingCapacity: true)
    #expect(builder.capacity == 512)
    #expect(builder.size == 0)
  }

  @Test
  func testCreateMonster() {
    let bytes = createMonster(withPrefix: false)
    // swiftformat:disable all
    #expect(
      bytes.sizedByteArray ==
      [
        48, 0, 0, 0, 77, 79, 78, 83, 0, 0, 0, 0, 36, 0, 72, 0, 40, 0, 0, 0, 38, 0, 32, 0, 0, 0, 28,
        0, 0, 0, 27, 0, 20, 0, 16, 0, 12, 0, 4, 0, 0, 0, 0, 0, 0, 0, 11, 0, 36, 0, 0, 0, 164, 0, 0,
        0, 0, 0, 0, 1, 60, 0, 0, 0, 68, 0, 0, 0, 76, 0, 0, 0, 0, 0, 0, 1, 88, 0, 0, 0, 120, 0, 0, 0,
        0, 0, 80, 0, 0, 0, 128, 63, 0, 0, 0, 64, 0, 0, 64, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 64,
        2, 0, 5, 0, 6, 0, 0, 0, 2, 0, 0, 0, 64, 0, 0, 0, 48, 0, 0, 0, 2, 0, 0, 0, 30, 0, 40, 0, 10,
        0, 20, 0, 152, 255, 255, 255, 4, 0, 0, 0, 4, 0, 0, 0, 70, 114, 101, 100, 0, 0, 0, 0, 5, 0,
        0, 0, 0, 1, 2, 3, 4, 0, 0, 0, 5, 0, 0, 0, 116, 101, 115, 116, 50, 0, 0, 0, 5, 0, 0, 0, 116,
        101, 115, 116, 49, 0, 0, 0, 9, 0, 0, 0, 77, 121, 77, 111, 110, 115, 116, 101, 114, 0, 0, 0,
        3, 0, 0, 0, 20, 0, 0, 0, 36, 0, 0, 0, 4, 0, 0, 0, 240, 255, 255, 255, 32, 0, 0, 0, 248, 255,
        255, 255, 36, 0, 0, 0, 12, 0, 8, 0, 0, 0, 0, 0, 0, 0, 4, 0, 12, 0, 0, 0, 28, 0, 0, 0, 5, 0,
        0, 0, 87, 105, 108, 109, 97, 0, 0, 0, 6, 0, 0, 0, 66, 97, 114, 110, 101, 121, 0, 0, 5, 0, 0,
        0, 70, 114, 111, 100, 111, 0, 0, 0,
      ])
    // swiftformat:enable all
    var buffer = bytes.buffer

    let monster: MyGame_Example_Monster = getRoot(byteBuffer: &buffer)
    readMonster(monster: monster)
    mutateMonster(fb: bytes.buffer)
    readMonster(monster: monster)
  }

  @Test
  func testCreateMonsterResizedBuffer() {
    let bytes = createMonster(withPrefix: false)
    // swiftformat:disable all
    #expect(
      bytes.sizedByteArray ==
      [
        48, 0, 0, 0, 77, 79, 78, 83, 0, 0, 0, 0, 36, 0, 72, 0, 40, 0, 0, 0, 38, 0, 32, 0, 0, 0, 28,
        0, 0, 0, 27, 0, 20, 0, 16, 0, 12, 0, 4, 0, 0, 0, 0, 0, 0, 0, 11, 0, 36, 0, 0, 0, 164, 0, 0,
        0, 0, 0, 0, 1, 60, 0, 0, 0, 68, 0, 0, 0, 76, 0, 0, 0, 0, 0, 0, 1, 88, 0, 0, 0, 120, 0, 0, 0,
        0, 0, 80, 0, 0, 0, 128, 63, 0, 0, 0, 64, 0, 0, 64, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 64,
        2, 0, 5, 0, 6, 0, 0, 0, 2, 0, 0, 0, 64, 0, 0, 0, 48, 0, 0, 0, 2, 0, 0, 0, 30, 0, 40, 0, 10,
        0, 20, 0, 152, 255, 255, 255, 4, 0, 0, 0, 4, 0, 0, 0, 70, 114, 101, 100, 0, 0, 0, 0, 5, 0,
        0, 0, 0, 1, 2, 3, 4, 0, 0, 0, 5, 0, 0, 0, 116, 101, 115, 116, 50, 0, 0, 0, 5, 0, 0, 0, 116,
        101, 115, 116, 49, 0, 0, 0, 9, 0, 0, 0, 77, 121, 77, 111, 110, 115, 116, 101, 114, 0, 0, 0,
        3, 0, 0, 0, 20, 0, 0, 0, 36, 0, 0, 0, 4, 0, 0, 0, 240, 255, 255, 255, 32, 0, 0, 0, 248, 255,
        255, 255, 36, 0, 0, 0, 12, 0, 8, 0, 0, 0, 0, 0, 0, 0, 4, 0, 12, 0, 0, 0, 28, 0, 0, 0, 5, 0,
        0, 0, 87, 105, 108, 109, 97, 0, 0, 0, 6, 0, 0, 0, 66, 97, 114, 110, 101, 121, 0, 0, 5, 0, 0,
        0, 70, 114, 111, 100, 111, 0, 0, 0,
      ])
    // swiftformat:enable all
    readVerifiedMonster(fb: bytes.sizedBuffer)
  }

  @Test
  func testCreateMonsterPrefixed() {
    let bytes = createMonster(withPrefix: true)
    // swiftformat:disable all
    #expect(
      bytes.sizedByteArray ==
      [
        44, 1, 0, 0, 44, 0, 0, 0, 77, 79, 78, 83, 36, 0, 72, 0, 40, 0, 0, 0, 38, 0, 32, 0, 0, 0, 28,
        0, 0, 0, 27, 0, 20, 0, 16, 0, 12, 0, 4, 0, 0, 0, 0, 0, 0, 0, 11, 0, 36, 0, 0, 0, 164, 0, 0,
        0, 0, 0, 0, 1, 60, 0, 0, 0, 68, 0, 0, 0, 76, 0, 0, 0, 0, 0, 0, 1, 88, 0, 0, 0, 120, 0, 0, 0,
        0, 0, 80, 0, 0, 0, 128, 63, 0, 0, 0, 64, 0, 0, 64, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 64,
        2, 0, 5, 0, 6, 0, 0, 0, 2, 0, 0, 0, 64, 0, 0, 0, 48, 0, 0, 0, 2, 0, 0, 0, 30, 0, 40, 0, 10,
        0, 20, 0, 152, 255, 255, 255, 4, 0, 0, 0, 4, 0, 0, 0, 70, 114, 101, 100, 0, 0, 0, 0, 5, 0,
        0, 0, 0, 1, 2, 3, 4, 0, 0, 0, 5, 0, 0, 0, 116, 101, 115, 116, 50, 0, 0, 0, 5, 0, 0, 0, 116,
        101, 115, 116, 49, 0, 0, 0, 9, 0, 0, 0, 77, 121, 77, 111, 110, 115, 116, 101, 114, 0, 0, 0,
        3, 0, 0, 0, 20, 0, 0, 0, 36, 0, 0, 0, 4, 0, 0, 0, 240, 255, 255, 255, 32, 0, 0, 0, 248, 255,
        255, 255, 36, 0, 0, 0, 12, 0, 8, 0, 0, 0, 0, 0, 0, 0, 4, 0, 12, 0, 0, 0, 28, 0, 0, 0, 5, 0,
        0, 0, 87, 105, 108, 109, 97, 0, 0, 0, 6, 0, 0, 0, 66, 97, 114, 110, 101, 121, 0, 0, 5, 0, 0,
        0, 70, 114, 111, 100, 111, 0, 0, 0,
      ])
    // swiftformat:enable all

    var buffer = bytes.buffer
    readMonster(monster: getPrefixedSizeRoot(byteBuffer: &buffer))
  }

  @Test
  func testCreateMonsterUsingCreateMonsterMethodWithNilPos() {
    var fbb = FlatBufferBuilder(initialSize: 1)
    let name = fbb.create(string: "Frodo")
    let mStart = Monster.startMonster(&fbb)
    Monster.add(name: name, &fbb)
    let root = Monster.endMonster(&fbb, start: mStart)
    fbb.finish(offset: root)
    var buffer = fbb.sizedBuffer
    let newMonster: Monster = getRoot(byteBuffer: &buffer)
    #expect(newMonster.pos == nil)
    #expect(newMonster.name == "Frodo")
  }

  @Test
  func testCreateMonsterUsingCreateMonsterMethodWithPosX() {
    var fbb = FlatBufferBuilder(initialSize: 1)
    let name = fbb.create(string: "Barney")
    let mStart = Monster.startMonster(&fbb)
    Monster.add(
      pos: MyGame_Example_Vec3(
        x: 10,
        y: 0,
        z: 0,
        test1: 0,
        test2: .blue,
        test3: .init()),
      &fbb)
    Monster.add(name: name, &fbb)
    let root = Monster.endMonster(&fbb, start: mStart)
    fbb.finish(offset: root)

    var buffer = fbb.sizedBuffer
    let newMonster: Monster = getRoot(byteBuffer: &buffer)
    #expect(newMonster.pos!.x == 10)
    #expect(newMonster.name == "Barney")
  }

  @Test
  func testReadMonsterFromUnsafePointerWithoutCopying() {
    // swiftformat:disable all
    var array: [UInt8] = [
      48, 0, 0, 0, 77, 79, 78, 83, 0, 0, 0, 0, 36, 0, 72, 0, 40, 0, 0, 0, 38, 0, 32, 0, 0, 0, 28, 0,
      0, 0, 27, 0, 20, 0, 16, 0, 12, 0, 4, 0, 0, 0, 0, 0, 0, 0, 11, 0, 36, 0, 0, 0, 164, 0, 0, 0, 0,
      0, 0, 1, 60, 0, 0, 0, 68, 0, 0, 0, 76, 0, 0, 0, 0, 0, 0, 1, 88, 0, 0, 0, 120, 0, 0, 0, 0, 0,
      80, 0, 0, 0, 128, 63, 0, 0, 0, 64, 0, 0, 64, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 64, 2, 0, 5,
      0, 6, 0, 0, 0, 2, 0, 0, 0, 64, 0, 0, 0, 48, 0, 0, 0, 2, 0, 0, 0, 30, 0, 40, 0, 10, 0, 20, 0,
      152, 255, 255, 255, 4, 0, 0, 0, 4, 0, 0, 0, 70, 114, 101, 100, 0, 0, 0, 0, 5, 0, 0, 0, 0, 1,
      2, 3, 4, 0, 0, 0, 5, 0, 0, 0, 116, 101, 115, 116, 50, 0, 0, 0, 5, 0, 0, 0, 116, 101, 115, 116,
      49, 0, 0, 0, 9, 0, 0, 0, 77, 121, 77, 111, 110, 115, 116, 101, 114, 0, 0, 0, 3, 0, 0, 0, 20,
      0, 0, 0, 36, 0, 0, 0, 4, 0, 0, 0, 240, 255, 255, 255, 32, 0, 0, 0, 248, 255, 255, 255, 36, 0,
      0, 0, 12, 0, 8, 0, 0, 0, 0, 0, 0, 0, 4, 0, 12, 0, 0, 0, 28, 0, 0, 0, 5, 0, 0, 0, 87, 105, 108,
      109, 97, 0, 0, 0, 6, 0, 0, 0, 66, 97, 114, 110, 101, 121, 0, 0, 5, 0, 0, 0, 70, 114, 111, 100,
      111, 0, 0, 0,
    ]
    // swiftformat:enable all
    let unpacked =
      array
        .withUnsafeMutableBytes { memory -> MyGame_Example_MonsterT in
          var bytes = ByteBuffer(
            assumingMemoryBound: memory.baseAddress!,
            capacity: memory.count)
          var monster: Monster = getRoot(byteBuffer: &bytes)
          readFlatbufferMonster(monster: &monster)
          let unpacked = monster.unpack()
          return unpacked
        }
    readObjectApi(monster: unpacked)
  }

  @Test
  func testArrayOfBools() {
    let boolArray = [false, true, false, true, false, true, false]
    var fbb = FlatBufferBuilder(initialSize: 1)
    let name = fbb.create(string: "Frodo")
    let bools = fbb.createVector(boolArray)

    let root = Monster.createMonster(
      &fbb,
      nameOffset: name,
      testarrayofboolsVectorOffset: bools)
    fbb.finish(offset: root)
    var buffer = fbb.sizedBuffer
    let monster: Monster = getRoot(byteBuffer: &buffer)
    let values = monster.testarrayofbools

    #expect(boolArray.count == values.count)

    for (index, bool) in monster.testarrayofbools.enumerated() {
      #expect(bool == boolArray[index])
    }
  }

  func readVerifiedMonster(fb: ByteBuffer) {
    var byteBuffer = fb
    do {
      try readMonster(
        monster: getCheckedRoot(
          byteBuffer: &byteBuffer) as MyGame_Example_Monster)
    } catch {
      Issue.record(error)
    }
  }

  @Test(.bug("https://github.com/google/flatbuffers/issues/8133"))
  func testUnalignedRead() {
    // Aligned read
    let fbb = createMonster(withPrefix: false)
    let testAligned: () -> Bool = {
      var buffer = fbb.sizedBuffer
      var monster: Monster = getRoot(byteBuffer: &buffer)
      readFlatbufferMonster(monster: &monster)
      return true
    }
    #expect(testAligned() == true)
    let testUnaligned: () -> Bool = {
      var bytes: [UInt8] = [0x00]
      bytes.append(contentsOf: fbb.sizedByteArray)
      return bytes.withUnsafeMutableBytes { ptr in
        guard var baseAddress = ptr.baseAddress else {
          Issue.record("Base pointer is not defined")
          return false
        }
        baseAddress = baseAddress.advanced(by: 1)
        let unlignedPtr = UnsafeMutableRawPointer(baseAddress)
        var bytes = ByteBuffer(
          assumingMemoryBound: unlignedPtr,
          capacity: ptr.count - 1)
        var monster: Monster = getRoot(byteBuffer: &bytes)
        readFlatbufferMonster(monster: &monster)
        return true
      }
    }
    #expect(testUnaligned() == true)
  }

  @Test
  func testReadingRemovedSizeUnalignedBuffer() {
    // Aligned read
    let fbb = createMonster(withPrefix: true)
    let testUnaligned: () -> Bool = {
      var bytes: [UInt8] = [0x00]
      bytes.append(contentsOf: fbb.sizedByteArray)
      return bytes.withUnsafeMutableBytes { ptr in
        guard var baseAddress = ptr.baseAddress else {
          Issue.record("Base pointer is not defined")
          return false
        }
        baseAddress = baseAddress.advanced(by: 1)
        let unlignedPtr = UnsafeMutableRawPointer(baseAddress)
        let bytes = ByteBuffer(
          assumingMemoryBound: unlignedPtr,
          capacity: ptr.count - 1)
        var newBuf = FlatBuffersUtils.removeSizePrefix(bb: bytes)
        var monster: Monster = getRoot(byteBuffer: &newBuf)
        readFlatbufferMonster(monster: &monster)
        return true
      }
    }
    #expect(testUnaligned() == true)
  }

  @Test
  func testCreateMessage() {
    let fbb = createMonster(withPrefix: false)
    let byteBuffer = fbb.buffer
    let firstMessage = GRPCMessage<Monster>(byteBuffer: byteBuffer)
    firstMessage.withUnsafeReadableBytes { ptr in
      var bytes = ByteBuffer(contiguousBytes: ptr, count: ptr.count)
      var monster: Monster = getRoot(byteBuffer: &bytes)
      readFlatbufferMonster(monster: &monster)
    }

    let secondByteBuffer = fbb.sizedBuffer
    let secondMessage = GRPCMessage<Monster>(byteBuffer: secondByteBuffer)
    secondMessage.withUnsafeReadableBytes { ptr in
      var bytes = ByteBuffer(contiguousBytes: ptr, count: ptr.count)
      var monster: Monster = getRoot(byteBuffer: &bytes)
      readFlatbufferMonster(monster: &monster)
    }
  }

  @Test
  func testForceRetainedObject() {
    let byteBuffer = {
      // swiftformat:disable all
      var data: Data? = Data([
        48, 0, 0, 0, 77, 79, 78, 83, 0, 0, 0, 0, 36, 0, 72, 0, 40, 0, 0, 0, 38, 0, 32, 0, 0, 0, 28,
        0, 0, 0, 27, 0, 20, 0, 16, 0, 12, 0, 4, 0, 0, 0, 0, 0, 0, 0, 11, 0, 36, 0, 0, 0, 164, 0, 0,
        0, 0, 0, 0, 1, 60, 0, 0, 0, 68, 0, 0, 0, 76, 0, 0, 0, 0, 0, 0, 1, 88, 0, 0, 0, 120, 0, 0, 0,
        0, 0, 80, 0, 0, 0, 128, 63, 0, 0, 0, 64, 0, 0, 64, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 64,
        2, 0, 5, 0, 6, 0, 0, 0, 2, 0, 0, 0, 64, 0, 0, 0, 48, 0, 0, 0, 2, 0, 0, 0, 30, 0, 40, 0, 10,
        0, 20, 0, 152, 255, 255, 255, 4, 0, 0, 0, 4, 0, 0, 0, 70, 114, 101, 100, 0, 0, 0, 0, 5, 0,
        0, 0, 0, 1, 2, 3, 4, 0, 0, 0, 5, 0, 0, 0, 116, 101, 115, 116, 50, 0, 0, 0, 5, 0, 0, 0, 116,
        101, 115, 116, 49, 0, 0, 0, 9, 0, 0, 0, 77, 121, 77, 111, 110, 115, 116, 101, 114, 0, 0, 0,
        3, 0, 0, 0, 20, 0, 0, 0, 36, 0, 0, 0, 4, 0, 0, 0, 240, 255, 255, 255, 32, 0, 0, 0, 248, 255,
        255, 255, 36, 0, 0, 0, 12, 0, 8, 0, 0, 0, 0, 0, 0, 0, 4, 0, 12, 0, 0, 0, 28, 0, 0, 0, 5, 0,
        0, 0, 87, 105, 108, 109, 97, 0, 0, 0, 6, 0, 0, 0, 66, 97, 114, 110, 101, 121, 0, 0, 5, 0, 0,
        0, 70, 114, 111, 100, 111, 0, 0, 0,
      ])
      // swiftformat:enable all
      let buffer = ByteBuffer(data: data!)
      data = nil
      return buffer
    }()
    readVerifiedMonster(fb: byteBuffer)
  }

  func readMonster(monster: Monster) {
    var monster = monster
    readFlatbufferMonster(monster: &monster)
    let unpacked: MyGame_Example_MonsterT? = monster.unpack()
    readObjectApi(monster: unpacked!)
    guard var buffer = unpacked?.serialize()
    else { fatalError("Couldnt generate bytebuffer") }
    var newMonster: Monster = getRoot(byteBuffer: &buffer)
    readFlatbufferMonster(monster: &newMonster)
  }

  func createMonster(withPrefix prefix: Bool) -> FlatBufferBuilder {
    var fbb = FlatBufferBuilder(initialSize: 1)
    write(fbb: &fbb, prefix: prefix)
    return fbb
  }

  func write(fbb: inout FlatBufferBuilder, prefix: Bool = false) {
    let names = [
      fbb.create(string: "Frodo"),
      fbb.create(string: "Barney"),
      fbb.create(string: "Wilma"),
    ]
    var offsets: [Offset] = []
    let start1 = Monster.startMonster(&fbb)
    Monster.add(name: names[0], &fbb)
    offsets.append(Monster.endMonster(&fbb, start: start1))
    let start2 = Monster.startMonster(&fbb)
    Monster.add(name: names[1], &fbb)
    offsets.append(Monster.endMonster(&fbb, start: start2))
    let start3 = Monster.startMonster(&fbb)
    Monster.add(name: names[2], &fbb)
    offsets.append(Monster.endMonster(&fbb, start: start3))

    let sortedArray = Monster.sortVectorOfMonster(offsets: offsets, &fbb)

    let str = fbb.create(string: "MyMonster")
    let test1 = fbb.create(string: "test1")
    let test2 = fbb.create(string: "test2")
    let _inv: [Byte] = [0, 1, 2, 3, 4]
    let inv = fbb.createVector(_inv)

    let fred = fbb.create(string: "Fred")
    let mon1Start = Monster.startMonster(&fbb)
    Monster.add(name: fred, &fbb)
    let mon2 = Monster.endMonster(&fbb, start: mon1Start)

    let test4 = fbb.createVector(ofStructs: [
      MyGame_Example_Test(a: 30, b: 40),
      MyGame_Example_Test(a: 10, b: 20),
    ])

    let stringTestVector = fbb.createVector(ofOffsets: [test1, test2])
    let mStart = Monster.startMonster(&fbb)
    Monster.add(
      pos: MyGame_Example_Vec3(
        x: 1,
        y: 2,
        z: 3,
        test1: 3,
        test2: .green,
        test3: .init(a: 5, b: 6)),
      &fbb)
    Monster.add(hp: 80, &fbb)
    Monster.add(name: str, &fbb)
    Monster.addVectorOf(inventory: inv, &fbb)
    Monster.add(testType: .monster, &fbb)
    Monster.add(test: mon2, &fbb)
    Monster.addVectorOf(test4: test4, &fbb)
    Monster.addVectorOf(testarrayofstring: stringTestVector, &fbb)
    Monster.add(testbool: true, &fbb)
    Monster.addVectorOf(testarrayoftables: sortedArray, &fbb)
    let end = Monster.endMonster(&fbb, start: mStart)
    Monster.finish(&fbb, end: end, prefix: prefix)
  }

  func mutateMonster(fb: ByteBuffer) {
    var fb = fb

    let monster: Monster = getRoot(byteBuffer: &fb)
    #expect(monster.mutate(mana: 10) == false)
    #expect(monster.testarrayoftables[0].name == "Barney")
    #expect(monster.testarrayoftables[1].name == "Frodo")
    #expect(monster.testarrayoftables[2].name == "Wilma")

    // Example of searching for a table by the key
    #expect(monster.testarrayoftablesBy(key: "Frodo") != nil)
    #expect(monster.testarrayoftablesBy(key: "Barney") != nil)
    #expect(monster.testarrayoftablesBy(key: "Wilma") != nil)

    #expect(monster.testType == .monster)

    #expect(monster.mutate(inventory: 1, at: 0) == true)
    #expect(monster.mutate(inventory: 2, at: 1) == true)
    #expect(monster.mutate(inventory: 3, at: 2) == true)
    #expect(monster.mutate(inventory: 4, at: 3) == true)
    #expect(monster.mutate(inventory: 5, at: 4) == true)

    for i in 0..<monster.inventory.count {
      #expect(monster.inventory[i] == Byte(i + 1))
    }

    #expect(monster.mutate(inventory: 0, at: 0) == true)
    #expect(monster.mutate(inventory: 1, at: 1) == true)
    #expect(monster.mutate(inventory: 2, at: 2) == true)
    #expect(monster.mutate(inventory: 3, at: 3) == true)
    #expect(monster.mutate(inventory: 4, at: 4) == true)

    let vec = monster.mutablePos
    #expect(vec?.x == 1)
    #expect(vec?.mutate(x: 55.0) == true)
    #expect(vec?.mutate(test1: 55) == true)
    #expect(vec?.x == 55.0)
    #expect(vec?.test1 == 55.0)
    #expect(vec?.mutate(x: 1) == true)
    #expect(vec?.x == 1)
    #expect(vec?.mutate(test1: 3) == true)

    let mutableTest4 = monster.mutableTest4
    let orignalValues = mutableTest4[0].a
    #expect(mutableTest4[0].mutate(a: 100) == true)
    #expect(monster.test4[0].a != orignalValues)
    #expect(monster.test4[0].a == 100)
    #expect(mutableTest4[0].mutate(a: orignalValues) == true)
  }

  func readFlatbufferMonster(monster: inout MyGame_Example_Monster) {
    #expect(monster.hp == 80)
    #expect(monster.mana == 150)
    #expect(monster.name == "MyMonster")
    let pos = monster.pos
    #expect(pos?.x == 1)
    #expect(pos?.y == 2)
    #expect(pos?.z == 3)
    #expect(pos?.test1 == 3)
    #expect(pos?.test2 == .green)
    let test = pos?.test3
    #expect(test?.a == 5)
    #expect(test?.b == 6)
    #expect(monster.testType == .monster)
    let monster2 = monster.test(type: Monster.self)
    #expect(monster2?.name == "Fred")

    #expect(monster.mutate(mana: 10) == false)

    #expect(monster.mana == 150)
    #expect(monster.inventory.count == 5)
    var sum: Byte = 0
    for inventory in monster.inventory {
      sum += inventory
    }
    #expect(sum == 10)

    monster.withUnsafePointerToInventory { ptr, count in
      var sum: UInt8 = 0
      for pointee in ptr.startIndex..<ptr.endIndex {
        sum += ptr[pointee]
      }
      #expect(sum == 10)
    }

    #expect(monster.test4.count == 2)

    let test4 = monster.test4
    var sum0 = 0
    for test0 in test4 {
      sum0 += Int(test0.a) + Int(test0.b)
    }
    #expect(sum0 == 100)

    monster.withUnsafePointerToTest4 { ptr, count in
      guard let ptr = ptr.baseAddress else { return }

      let bindedMemory: UnsafeBufferPointer<MyGame_Example_Test> =
        UnsafeBufferPointer(
          start: ptr.bindMemory(
            to: MyGame_Example_Test.self,
            capacity: count),
          count: count)
      var pointerSum = 0
      for pointee in bindedMemory.startIndex..<bindedMemory.endIndex {
        pointerSum += Int(bindedMemory[pointee].a) +
          Int(bindedMemory[pointee].b)
      }
      #expect(pointerSum == 100)
    }

    let mutableTest4 = monster.mutableTest4
    var sum2 = 0
    for test0 in mutableTest4 {
      sum2 += Int(test0.a) + Int(test0.b)
    }
    #expect(sum2 == 100)

    let stringArray = monster.testarrayofstring
    #expect(stringArray.count == 2)
    #expect(stringArray[0] == "test1")
    #expect(stringArray[1] == "test2")
    #expect(monster.testbool == true)

    let array = monster.nameSegmentArray
    #expect(String(bytes: array ?? [], encoding: .utf8) == "MyMonster")

    if 0 == monster.testarrayofbools.count {
      #expect(monster.testarrayofbools.isEmpty == true)
    } else {
      #expect(monster.testarrayofbools.isEmpty == false)
    }
  }

  func readObjectApi(monster: MyGame_Example_MonsterT) {
    #expect(monster.hp == 80)
    #expect(monster.mana == 150)
    #expect(monster.name == "MyMonster")
    let pos = monster.pos
    #expect(pos?.x == 1)
    #expect(pos?.y == 2)
    #expect(pos?.z == 3)
    #expect(pos?.test1 == 3)
    #expect(pos?.test2 == .green)
    let test = pos?.test3
    #expect(test?.a == 5)
    #expect(test?.b == 6)
    let monster2 = monster.test?.value as? MyGame_Example_MonsterT
    #expect(monster2?.name == "Fred")
    #expect(monster.mana == 150)
    monster.mana = 10
    #expect(monster.mana == 10)
    monster.mana = 150
    #expect(monster.mana == 150)

    #expect(monster.inventory.count == 5)
    var sum: Byte = 0
    for i in monster.inventory {
      sum += i
    }
    #expect(sum == 10)
    #expect(monster.test4.count == 2)
    var sum0 = 0
    for test in monster.test4 {
      sum0 += Int(test.a) + Int(test.b)
    }
    #expect(sum0 == 100)
    #expect(monster.testbool == true)
  }

  @Test
  func testEncoding() throws {
    let fbb = createMonster(withPrefix: false)
    var sizedBuffer = fbb.sizedBuffer
    struct Test: Decodable {
      struct Pos: Decodable {
        let x, y, z: Int
      }
      let hp: Int
      let inventory: [UInt8]
      let name: String
      let pos: Pos
    }
    let reader: Monster = try getCheckedRoot(byteBuffer: &sizedBuffer)
    let encoder = JSONEncoder()
    encoder.keyEncodingStrategy = .convertToSnakeCase
    let data = try encoder.encode(reader)
    let decoder = JSONDecoder()
    decoder.keyDecodingStrategy = .convertFromSnakeCase
    let value = try decoder.decode(Test.self, from: data)
    #expect(value.name == "MyMonster")
    #expect(value.pos.x == 1)
    #expect(value.pos.y == 2)
    #expect(value.pos.z == 3)
  }

  var jsonData: String {
    """
    {\"hp\":80,\"inventory\":[0,1,2,3,4],\"test\":{\"name\":\"Fred\"},\"testarrayofstring\":[\"test1\",\"test2\"],\"testarrayoftables\":[{\"name\":\"Barney\"},{\"name\":\"Frodo\"},{\"name\":\"Wilma\"}],\"test4\":[{\"a\":30,\"b\":40},{\"a\":10,\"b\":20}],\"testbool\":true,\"test_type\":\"Monster\",\"pos\":{\"y\":2,\"test3\":{\"a\":5,\"b\":6},\"z\":3,\"x\":1,\"test1\":3,\"test2\":\"Green\"},\"name\":\"MyMonster\"}
    """
  }

  @Test
  func testContiguousBytes() {
    let byteArray: [UInt8] = [3, 1, 4, 1, 5, 9]
    var fbb = FlatBufferBuilder(initialSize: 1)
    let name = fbb.create(string: "Frodo")
    let bytes = fbb.createVector(bytes: byteArray)
    let root = Monster.createMonster(
      &fbb,
      nameOffset: name,
      inventoryVectorOffset: bytes)
    fbb.finish(offset: root)
    var buffer = fbb.sizedBuffer
    let monster: Monster = getRoot(byteBuffer: &buffer)
    let values = monster.inventory

    monster.withUnsafePointerToInventory { ptr, count in
      let array = Array(ptr)
      for (index, value) in values.enumerated() {
        #expect(array[index] == value)
      }
    }
  }
}
