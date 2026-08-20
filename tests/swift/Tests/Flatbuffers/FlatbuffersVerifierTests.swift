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

import Testing

@testable import FlatBuffers

final class FlatbuffersVerifierTests {

  private var buffer: ByteBuffer
  private var validFlatbuffersObject: ByteBuffer
  private var invalidFlatbuffersObject: ByteBuffer
  private var invalidFlatbuffersObject2: ByteBuffer
  private var invalidFlatbuffersObject3: ByteBuffer

  init() {
    // swiftformat:disable all
    validFlatbuffersObject = ByteBuffer(bytes: [
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

    invalidFlatbuffersObject = ByteBuffer(bytes: [
      0, 48, 0, 0, 0, 77, 79, 78, 83, 0, 0, 0, 0, 36, 0, 72, 0, 40, 0, 0, 0, 38, 0, 32, 0, 0, 0, 28,
      0, 0, 0, 27, 0, 20, 0, 16, 0, 12, 0, 4, 0, 0, 0, 0, 0, 0, 0, 11, 0, 36, 0, 0, 0, 164, 0, 0, 0,
      0, 0, 0, 1, 60, 0, 0, 0, 68, 0, 0, 0, 76, 0, 0, 0, 0, 0, 0, 1, 88, 0, 0, 0, 120, 0, 0, 0, 0,
      0, 80, 0, 0, 0, 128, 63, 0, 0, 0, 64, 0, 0, 64, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 64, 2, 0,
      5, 0, 6, 0, 0, 0, 2, 0, 0, 0, 64, 0, 0, 0, 48, 0, 0, 0, 2, 0, 0, 0, 30, 0, 40, 0, 10, 0, 20,
      0, 152, 255, 255, 255, 4, 0, 0, 0, 4, 0, 0, 0, 70, 114, 101, 100, 0, 0, 0, 0, 5, 0, 0, 0, 0,
      1, 2, 3, 4, 0, 0, 0, 5, 0, 0, 0, 116, 101, 115, 116, 50, 0, 0, 0, 5, 0, 0, 0, 116, 101, 115,
      116, 49, 0, 0, 0, 9, 0, 0, 0, 77, 121, 77, 111, 110, 115, 116, 101, 114, 0, 0, 0, 3, 0, 0, 0,
      20, 0, 0, 0, 36, 0, 0, 0, 4, 0, 0, 0, 240, 255, 255, 255, 32, 0, 0, 0, 248, 255, 255, 255, 36,
      0, 0, 0, 12, 0, 8, 0, 0, 0, 0, 0, 0, 0, 4, 0, 12, 0, 0, 0, 28, 0, 0, 0, 5, 0, 0, 0, 87, 105,
      108, 109, 97, 0, 0, 0, 6, 0, 0, 0, 66, 97, 114, 110, 101, 121, 0, 0, 5, 0, 0, 0, 70, 114, 111,
      100, 111, 0, 0, 0,
    ])

    // Array failure within a the inventory array
    invalidFlatbuffersObject2 = ByteBuffer(bytes: [
      48, 0, 0, 0, 77, 79, 78, 83, 0, 0, 0, 0, 36, 0, 72, 0, 40, 0, 0, 0, 38, 0, 32, 0, 0, 0, 28, 0,
      0, 0, 27, 0, 20, 0, 16, 0, 12, 0, 4, 0, 0, 0, 0, 0, 0, 0, 11, 0, 36, 0, 0, 0, 164, 0, 0, 0, 0,
      0, 0, 1, 60, 0, 0, 0, 68, 0, 0, 0, 76, 0, 0, 0, 0, 0, 0, 1, 88, 0, 0, 0, 120, 0, 0, 0, 0, 0,
      80, 0, 0, 0, 128, 63, 0, 0, 0, 64, 0, 0, 64, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 64, 2, 0, 5,
      0, 6, 0, 0, 0, 2, 0, 0, 0, 64, 0, 0, 0, 48, 0, 0, 0, 2, 0, 0, 0, 30, 0, 40, 0, 10, 0, 20, 0,
      152, 255, 255, 255, 4, 0, 0, 0, 4, 0, 0, 0, 70, 114, 101, 100, 0, 0, 0, 0, 5, 0, 0, 0, 0, 1,
      2, 0x00, 3, 4, 0, 0, 0, 5, 0, 0, 0, 116, 101, 115, 116, 50, 0, 0, 0, 5, 0, 0, 0, 116, 101,
      115, 116, 49, 0, 0, 0, 9, 0, 0, 0, 77, 121, 77, 111, 110, 115, 116, 101, 114, 0, 0, 0, 3, 0,
      0, 0, 20, 0, 0, 0, 36, 0, 0, 0, 4, 0, 0, 0, 240, 255, 255, 255, 32, 0, 0, 0, 248, 255, 255,
      255, 36, 0, 0, 0, 12, 0, 8, 0, 0, 0, 0, 0, 0, 0, 4, 0, 12, 0, 0, 0, 28, 0, 0, 0, 5, 0, 0, 0,
      87, 105, 108, 109, 97, 0, 0, 0, 6, 0, 0, 0, 66, 97, 114, 110, 101, 121, 0, 0, 5, 0, 0, 0, 70,
      114, 111, 100, 111, 0, 0, 0,
    ])

    // Array failure within a the strings array
    invalidFlatbuffersObject3 = ByteBuffer(bytes: [
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
      0x00, 111, 0, 0, 0,
    ])

    let memory = UnsafeMutableRawPointer.allocate(byteCount: 32, alignment: 1)
    buffer = ByteBuffer(assumingMemoryBound: memory, capacity: 32)

    add(buffer: &buffer, v: 4, p: 16)
    add(buffer: &buffer, v: 4, p: 20)
    // swiftformat:enable all
  }

  @Test
  func testVeriferInitPassing() throws {
    let memory = UnsafeMutableRawPointer.allocate(byteCount: 8, alignment: 1)
    var buffer = ByteBuffer(
      assumingMemoryBound: memory,
      capacity: Int(FlatBufferMaxSize) - 1)
    _ = try Verifier(buffer: &buffer)
  }

  @Test
  func testVeriferInitFailing() {
    let memory = UnsafeMutableRawPointer.allocate(byteCount: 8, alignment: 1)
    var buffer = ByteBuffer(
      assumingMemoryBound: memory,
      capacity: Int(FlatBufferMaxSize) + 1)
    #expect(throws: FlatbuffersErrors.exceedsMaxSizeAllowed) {
      try Verifier(buffer: &buffer)
    }
  }

  @Test
  func testFailingID() {
    let dutData: [UInt8] = [1, 2, 3, 4, 5, 6, 7]
    var buff = ByteBuffer(bytes: dutData)
    #expect(throws: FlatbuffersErrors.bufferDoesntContainID) {
      let _: Monster = try getCheckedRoot(byteBuffer: &buff, fileId: "ABCD")
    }
  }

  @Test
  func testVerifierCheckAlignment() throws {
    let verifier = try! Verifier(buffer: &buffer)
    #expect(throws: FlatbuffersErrors.missAlignedPointer(
      position: 20,
      type: "Int"))
    {
      try verifier.isAligned(position: 20, type: Int.self)
    }

    try verifier.isAligned(position: 16, type: Int.self)

    let newVerifer = try! Verifier(buffer: &buffer, checkAlignment: false)
    try newVerifer.isAligned(position: 16, type: Int.self)
  }

  @Test
  func testRangeInBuffer() throws {
    var verifier = try! Verifier(buffer: &buffer)
    let size = MemoryLayout<Int64>.size
    try verifier.rangeInBuffer(position: 24, size: size)
    #expect(throws: FlatbuffersErrors.outOfBounds(position: 34, end: 32)) {
      try verifier.rangeInBuffer(position: 26, size: size)
    }
    #expect(throws: FlatbuffersErrors.outOfBounds(position: 34, end: 32)) {
      try verifier.rangeInBuffer(
        position: 26,
        size: size)
    }
    #expect(throws: FlatbuffersErrors.outOfBounds(position: 38, end: 32)) {
      try verifier.rangeInBuffer(
        position: 30,
        size: size)
    }
    #expect(throws: FlatbuffersErrors.outOfBounds(position: 40, end: 32)) {
      try verifier.rangeInBuffer(
        position: 32,
        size: size)
    }
    #expect(throws: FlatbuffersErrors.outOfBounds(position: 42, end: 32)) {
      try verifier.rangeInBuffer(
        position: 34,
        size: size)
    }

    verifier = try! Verifier(
      buffer: &buffer,
      options: .init(maxDepth: 0, maxTableCount: 0, maxApparentSize: 4))

    #expect(throws: FlatbuffersErrors.apparentSizeTooLarge) {
      try verifier.rangeInBuffer(position: 24, size: size)
    }
  }

  @Test
  func testPositionInBuffer() throws {
    let verifier = try! Verifier(buffer: &buffer)
    try verifier.inBuffer(position: 0, of: Int64.self)
    try verifier.inBuffer(position: 24, of: Int64.self)

    #expect(
      throws: FlatbuffersErrors.missAlignedPointer(
        position: -9,
        type: "Int64"))
    {
      try verifier.inBuffer(position: -9, of: Int64.self)
    }
    #expect(
      throws: FlatbuffersErrors.missAlignedPointer(
        position: 25,
        type: "Int64"))
    {
      try verifier.inBuffer(position: 25, of: Int64.self)
    }
    #expect(
      throws: FlatbuffersErrors.missAlignedPointer(
        position: 26,
        type: "Int32"))
    {
      try verifier.inBuffer(position: 26, of: Int32.self)
    }
    #expect(
      throws: FlatbuffersErrors.missAlignedPointer(
        position: 26,
        type: "Int64"))
    {
      try verifier.inBuffer(position: 26, of: Int64.self)
    }
    #expect(
      throws: FlatbuffersErrors.missAlignedPointer(
        position: 30,
        type: "Int64"))
    {
      try verifier.inBuffer(position: 30, of: Int64.self)
    }
    #expect(throws: FlatbuffersErrors.outOfBounds(position: 40, end: 32)) {
      try verifier.inBuffer(
        position: 32,
        of: Int64.self)
    }
    #expect(
      throws: FlatbuffersErrors.missAlignedPointer(
        position: 34,
        type: "Int64"))
    {
      try verifier.inBuffer(position: 34, of: Int64.self)
    }
  }

  @Test
  func testVisitTable() throws {
    var verifier = try! Verifier(buffer: &validFlatbuffersObject)
    _ = try verifier.visitTable(at: 48)
    verifier.reset()
  }

  @Test
  func testTableVerifier() throws {
    var verifier = try! Verifier(buffer: &validFlatbuffersObject)

    var tableVerifer = try! verifier.visitTable(at: 48)
    #expect(verifier.depth == 1)
    #expect(verifier.tableCount == 1)

    try tableVerifer.visit(
      field: 4,
      fieldName: "Vec",
      required: false,
      type: Vec3.self)

    try tableVerifer.visit(
      field: 8,
      fieldName: "hp",
      required: false,
      type: Int16.self)

    try tableVerifer.visit(
      field: 10,
      fieldName: "name",
      required: true,
      type: ForwardOffset<String>.self)

    try tableVerifer.visit(
      field: 14,
      fieldName: "inventory",
      required: false,
      type: ForwardOffset<Vector<UInt8, UInt8>>.self)

    try tableVerifer.visit(
      field: 22,
      fieldName: "test4",
      required: false,
      type: ForwardOffset<Vector<MyGame_Example_Test, MyGame_Example_Test>>
        .self)

    try tableVerifer.visit(
      field: 24,
      fieldName: "Vector of strings",
      required: false,
      type: ForwardOffset<Vector<ForwardOffset<String>, String>>.self)

    #expect(throws: FlatbuffersErrors.missAlignedPointer(
      position: 25,
      type: "UInt16"))
    {
      try tableVerifer.visit(
        field: 13,
        fieldName: "notvalid",
        required: false,
        type: Int16.self)
    }

    try tableVerifer.visit(
      unionKey: 18,
      unionField: 20,
      unionKeyName: "testType",
      fieldName: "test",
      required: false,
      completion: { (verifier, key: MyGame_Example_Any_, pos) in
        switch key {
        case .none_:
          break // NOTE - SWIFT doesnt support none
        case .monster:
          try ForwardOffset<MyGame_Example_Monster>.verify(
            &verifier,
            at: pos,
            of: MyGame_Example_Monster.self)
        case .testsimpletablewithenum:
          try ForwardOffset<MyGame_Example_TestSimpleTableWithEnum>.verify(
            &verifier,
            at: pos,
            of: MyGame_Example_TestSimpleTableWithEnum.self)
        case .mygameExample2Monster:
          try ForwardOffset<MyGame_Example2_Monster>.verify(
            &verifier,
            at: pos,
            of: MyGame_Example2_Monster.self)
        }
      })

    tableVerifer.finish()
    #expect(verifier.depth == 0)
  }

  @Test
  func testVerifyUnionVectors() throws {
    // swiftformat:disable all
    var byteBuffer = ByteBuffer(bytes: [
      20, 0, 0, 0, 77, 79, 86, 73, 12, 0, 12, 0, 0, 0, 0, 0, 8, 0, 4, 0, 12, 0, 0, 0, 8, 0, 0, 0,
      20, 0, 0, 0, 3, 0, 0, 0, 24, 0, 0, 0, 32, 0, 0, 0, 12, 0, 0, 0, 3, 0, 0, 0, 3, 1, 4, 0, 2, 0,
      0, 0, 7, 0, 0, 0, 0, 0, 6, 0, 8, 0, 4, 0, 6, 0, 0, 0, 8, 0, 0, 0,
    ])
    // swiftformat:enable all
    _ = try getCheckedRoot(byteBuffer: &byteBuffer) as Movie
  }

  @Test
  func testErrorWrongFileId() throws{
    // swiftformat:disable all
    var byteBuffer = ByteBuffer(bytes: [
      20, 0, 0, 0, 77, 79, 86, 73, 12, 0, 12, 0, 0, 0, 0, 0, 8, 0, 4, 0, 12, 0, 0, 0, 8, 0, 0, 0,
      20, 0, 0, 0, 3, 0, 0, 0, 24, 0, 0, 0, 32, 0, 0, 0, 12, 0, 0, 0, 3, 0, 0, 0, 3, 1, 4, 0, 2, 0,
      0, 0, 7, 0, 0, 0, 0, 0, 6, 0, 8, 0, 4, 0, 6, 0, 0, 0, 8, 0, 0, 0,
    ])
    // swiftformat:enable all
    #expect(throws: FlatbuffersErrors.bufferIdDidntMatchPassedId) {
      try getCheckedRoot(
        byteBuffer: &byteBuffer,
        fileId: "FLEX") as Movie
    }
  }

  @Test
  func testVerifyPrefixedBuffer() {
    // swiftformat:disable all
    var byteBuffer = ByteBuffer(bytes: [
      0, 0, 0, 1, 20, 0, 0, 0, 77, 79, 86, 73, 12, 0, 12, 0, 0, 0, 0, 0, 8, 0, 4, 0, 12, 0, 0, 0, 8,
      0, 0, 0, 20, 0, 0, 0, 3, 0, 0, 0, 24, 0, 0, 0, 32, 0, 0, 0, 12, 0, 0, 0, 3, 0, 0, 0, 3, 1, 4,
      0, 2, 0, 0, 0, 7, 0, 0, 0, 0, 0, 6, 0, 8, 0, 4, 0, 6, 0, 0, 0, 8, 0, 0, 0,
    ])
    // swiftformat:enable all
    #expect(throws: FlatbuffersErrors.prefixedSizeNotEqualToBufferSize) {
      try getCheckedPrefixedSizeRoot(byteBuffer: &byteBuffer) as Movie
    }
  }

  @Test
  func testSizePrefixedVerifierUsesPostPrefixRoot() throws {
    // The size prefix points to a valid empty decoy table at byte 44. The real
    // root at byte 4 points to a truncated UInt64 vector that must be rejected.
    let bytes: [UInt8] = [
      44, 0, 0, 0,
      16, 0, 0, 0,
      6, 0, 8, 0, 4, 0,
      0, 0, 0, 0, 0, 0,
      12, 0, 0, 0,
      8, 0, 0, 0, 0, 0, 0, 0,
      2, 0, 0, 0, 65, 66,
      0, 0,
      4, 0, 4, 0,
      4, 0, 0, 0,
    ]

    var strictBuffer = ByteBuffer(bytes: bytes)
    #expect(throws: FlatbuffersErrors.self) {
      try getCheckedPrefixedSizeRoot(
        byteBuffer: &strictBuffer) as Swift_Tests_Vectors
    }

    var prefixedBuffer = ByteBuffer(bytes: bytes)
    #expect(throws: FlatbuffersErrors.self) {
      try getPrefixedSizeCheckedRoot(
        byteBuffer: &prefixedBuffer) as Swift_Tests_Vectors
    }

    var builder = FlatBufferBuilder()
    let movie = Movie.createMovie(&builder)
    Movie.finish(&builder, end: movie, prefix: true)
    var validBuffer = builder.sizedBuffer
    let _: Movie = try getCheckedPrefixedSizeRoot(
      byteBuffer: &validBuffer,
      fileId: Movie.id)

    var shortBuffer = ByteBuffer(bytes: [0, 0, 0, 0])
    #expect(throws: FlatbuffersErrors.bufferDoesntContainID) {
      try getCheckedRoot(
        byteBuffer: &shortBuffer,
        fileId: Movie.id) as Movie
    }

    var oversizedPrefix = ByteBuffer(bytes: [5, 0, 0, 0, 0, 0, 0, 0])
    #expect(throws: FlatbuffersErrors.outOfBounds(position: 9, end: 8)) {
      try getPrefixedSizeCheckedRoot(
        byteBuffer: &oversizedPrefix) as Movie
    }
    #expect(oversizedPrefix.reader == 0)
  }

  @Test
  func testFullVerifier() throws {
    _ =
      try getCheckedRoot(
        byteBuffer: &validFlatbuffersObject) as MyGame_Example_Monster
  }

  @Test
  func testFullVerifierWithFileId() throws {
    _ = try getCheckedRoot(
      byteBuffer: &validFlatbuffersObject,
      fileId: MyGame_Example_Monster.id) as MyGame_Example_Monster
  }

  @Test
  func testInvalidBuffer() {
    #expect(throws: FlatbuffersErrors.self) {
      try getCheckedRoot(
        byteBuffer: &self.invalidFlatbuffersObject) as MyGame_Example_Monster
    }
  }

  @Test
  func testInvalidBuffer2() {
    #expect(throws: FlatbuffersErrors.self) {
      try getCheckedRoot(
        byteBuffer: &self.invalidFlatbuffersObject2) as MyGame_Example_Monster
    }
  }

  @Test
  func testInvalidBuffer3() {
    #expect(throws: FlatbuffersErrors.self) {
      try getCheckedRoot(
        byteBuffer: &self.invalidFlatbuffersObject3) as MyGame_Example_Monster
    }
  }

  @Test(.bug("https://github.com/google/flatbuffers/issues/9082"))
  func testRejectsTruncatedScalarVector() {
    // swiftformat:disable all
    var byteBuffer = ByteBuffer(bytes: [
      16, 0, 0, 0,
      6, 0, 8, 0,
      4, 0, 0, 0,
      0, 0, 0, 0,
      12, 0, 0, 0,
      8, 0, 0, 0,
      0, 0, 0, 0,
      2, 0, 0, 0,
      65, 66,
    ])
    // swiftformat:enable all

    #expect(throws: FlatbuffersErrors.self) {
      try getCheckedRoot(byteBuffer: &byteBuffer) as Swift_Tests_Vectors
    }
  }

  @Test
  func testValidUnionBuffer() {
    let string = "Awesome \\\\t\t\nstring!"
    var fb = FlatBufferBuilder()
    let stringOffset = fb.create(string: string)
    let characterType: [Character] = [.bookfan, .other]

    let characters = [
      fb.create(struct: BookReader(booksRead: 7)),
      stringOffset,
    ]
    let types = fb.createVector(characterType)
    let characterVector = fb.createVector(ofOffsets: characters)

    let end = Movie.createMovie(
      &fb,
      mainCharacterType: .other,
      mainCharacterOffset: Offset(offset: stringOffset.o),
      charactersTypeVectorOffset: types,
      charactersVectorOffset: characterVector)
    Movie.finish(&fb, end: end)
    var buf = fb.sizedBuffer
    do {
      _ = try getCheckedRoot(byteBuffer: &buf) as Movie
    } catch {
      Issue.record(error)
    }
  }

  @Test
  func testNestedTables() throws {
    var builder = FlatBufferBuilder()
    let name = builder.create(string: "Monster")

    let enemy = MyGame_Example_Monster.createMonster(
      &builder,
      nameOffset: name)
    let currentName = builder.create(string: "Main name")
    let monster = MyGame_Example_Monster.createMonster(
      &builder,
      nameOffset: currentName,
      enemyOffset: enemy)
    builder.finish(offset: monster)

    var sizedBuffer = builder.sizedBuffer
    var verifier = try! Verifier(buffer: &sizedBuffer)
    var tableVerifer = try! verifier.visitTable(
      at: try getOffset(at: 0, within: verifier))
    #expect(verifier.depth == 1)
    #expect(verifier.tableCount == 1)

    let position = try tableVerifer.dereference(28)!

    var nestedTable = try verifier.visitTable(
      at: try getOffset(at: position, within: verifier))

    #expect(verifier.depth == 2)
    #expect(verifier.tableCount == 2)
    nestedTable.finish()
    #expect(verifier.depth == 1)
    #expect(verifier.tableCount == 2)
    tableVerifer.finish()
    #expect(verifier.depth == 0)
    #expect(verifier.tableCount == 2)
  }

  func add(buffer: inout ByteBuffer, v: Int32, p: Int) {
    buffer.write(value: v, index: p)
  }

  private func getOffset(
    at value: Int,
    within verifier: Verifier) throws -> Int
  {
    let offset: UOffset = try verifier.getValue(at: value)
    return Int(clamping: (Int(offset) &+ 0).magnitude)
  }

  @Test
  func testHighVTableSlotIsReadUnsignedLikeTheVerifier() throws {
    // Crafted buffer whose `array` vtable slot is 0x8000. The verifier reads
    // the slot as an unsigned offset and accepts the buffer (the vector at
    // table + 32768 is in bounds). Table.offset must observe the same
    // displacement; re-reading the slot as a signed value would send the
    // accessor before the start of the buffer.
    // `array` is the 3rd field in tests/vector_has_test.fbs (VT.array = 8).
    // The object size of 4 is deliberate: field data may live outside the
    // table's inline region; the verifier only checks target bounds.
    var bytes = [UInt8](repeating: 0, count: 32792)
    bytes[0..<4] = [4, 0, 0, 0]  // root offset -> table at 4
    bytes[4..<8] = [0xFC, 0xFF, 0xFF, 0xFF]  // soffset -4 -> vtable at 8
    bytes[8..<10] = [10, 0]  // vtable size
    bytes[10..<12] = [4, 0]  // object size
    bytes[16..<18] = [0, 0x80]  // `array` slot (vtable + 8) = 0x8000
    // Field lives at table (4) + 0x8000 and holds a uoffset to the vector.
    bytes[32772..<32776] = [8, 0, 0, 0]  // uoffset -> vector header at 32780
    bytes[32780..<32784] = [1, 0, 0, 0]  // vector length = 1
    bytes[32784..<32792] = [0x2A, 0, 0, 0, 0, 0, 0, 0]  // element 42

    var byteBuffer = ByteBuffer(bytes: bytes)
    let vectors: Swift_Tests_Vectors = try getCheckedRoot(byteBuffer: &byteBuffer)
    #expect(vectors.array.count == 1)
    #expect(vectors.array[0] == 42)
  }

  @Test
  func testHighVTableSlotMutateUsesVerifiedPosition() throws {
    // Same crafted-slot shape through the write path: `mutate` reuses
    // Table.offset, so the patched unsigned read must also steer writes to
    // the verified field position. `count` is the 3rd field of
    // MyGame.Example.Stat (VT.count = 8).
    var bytes = [UInt8](repeating: 0, count: 32776)
    bytes[0..<4] = [4, 0, 0, 0]  // root offset -> table at 4
    bytes[4..<8] = [0xFC, 0xFF, 0xFF, 0xFF]  // soffset -4 -> vtable at 8
    bytes[8..<10] = [10, 0]  // vtable size
    bytes[10..<12] = [4, 0]  // object size
    bytes[16..<18] = [0, 0x80]  // `count` slot (vtable + 8) = 0x8000
    bytes[32772..<32774] = [0x2A, 0]  // count = 42 at table (4) + 0x8000

    var byteBuffer = ByteBuffer(bytes: bytes)
    var stat: MyGame_Example_Stat = try getCheckedRoot(byteBuffer: &byteBuffer)
    #expect(stat.count == 42)
    #expect(stat.mutate(count: 0x7777))
    #expect(stat.count == 0x7777)
  }

  @Test
  func testHighVTableSlotLookupByKeyUsesVerifiedPosition() throws {
    // Generated `lookupByKey` resolves keyed-table slots through the static
    // Table.offset(_:vOffset:fbb:) overload; the slot read there must match
    // the verifier too. Monster carries a sorted Stat vector
    // (VT.scalarKeySortedTables = 104) whose Stat `count` slot
    // (VT.count = 8) is 0x8000.
    var bytes = [UInt8](repeating: 0, count: 32928)
    bytes[0..<4] = [4, 0, 0, 0]  // root offset -> Monster table at 4
    bytes[4..<8] = [0xFC, 0xFF, 0xFF, 0xFF]  // soffset -4 -> vtable at 8
    bytes[8..<10] = [106, 0]  // Monster vtable size (covers slot 104)
    bytes[10..<12] = [4, 0]  // object size
    bytes[18..<20] = [112, 0]  // `name` slot (vtable + 10) -> field at 116
    bytes[112..<114] = [128, 0]  // `scalarKeySortedTables` slot -> field at 132
    bytes[116..<120] = [8, 0, 0, 0]  // name uoffset -> string at 124
    bytes[124..<128] = [2, 0, 0, 0]  // string length
    bytes[128..<130] = [0x41, 0x42]  // "AB"
    bytes[132..<136] = [8, 0, 0, 0]  // vector-field uoffset -> header at 140
    bytes[140..<144] = [1, 0, 0, 0]  // vector length = 1
    bytes[144..<148] = [8, 0, 0, 0]  // element uoffset -> Stat table at 152
    bytes[152..<156] = [0xF8, 0xFF, 0xFF, 0xFF]  // soffset -8 -> vtable at 160
    bytes[160..<162] = [10, 0]  // Stat vtable size
    bytes[162..<164] = [4, 0]  // object size
    bytes[168..<170] = [0, 0x80]  // `count` slot (vtable + 8) = 0x8000
    bytes[32920..<32922] = [0, 0]  // count = 0 at Stat (152) + 0x8000

    var byteBuffer = ByteBuffer(bytes: bytes)
    let monster: MyGame_Example_Monster = try getCheckedRoot(byteBuffer: &byteBuffer)
    let stat = monster.scalarKeySortedTablesBy(key: 0)
    #expect(stat != nil)
    #expect(stat?.count == 0)
  }
}
