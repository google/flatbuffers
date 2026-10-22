/*
 * Copyright 2026 Google Inc. All rights reserved.
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

private typealias Holder = Swift_Tests_ForceAlign_Holder
private typealias HolderT = Swift_Tests_ForceAlign_HolderT
private typealias Aligned16 = Swift_Tests_ForceAlign_Aligned16
private typealias Aligned8 = Swift_Tests_ForceAlign_Aligned8
private typealias Aligned2 = Swift_Tests_ForceAlign_Aligned2
private typealias Outer = Swift_Tests_ForceAlign_Outer

/// Checks that structs are written at their FlatBuffers alignment (the
/// struct's minalign, which includes `force_align`), both as table fields and
/// as vector elements, even though Swift can't give the generated structs
/// that alignment.
struct FlatBuffersForceAlignTests {

  @Test
  func testGeneratedStructsAreLessAlignedThanRequired() {
    // The premise of the tests below: the natural alignment of the generated
    // Swift structs is lower than what the schema requires, so a builder that
    // relies on `MemoryLayout<T>.alignment` misaligns them.
    #expect(MemoryLayout<Aligned16>.size == 16)
    #expect(MemoryLayout<Aligned16>.alignment < 16)
    #expect(MemoryLayout<Aligned8>.size == 8)
    #expect(MemoryLayout<Aligned8>.alignment < 8)
    #expect(MemoryLayout<Outer>.size == 32)
    #expect(MemoryLayout<Outer>.alignment < 16)
    #expect(MemoryLayout<Aligned2>.size == 2)
  }

  /// `prefix` shifts everything written after the leading byte vector, so
  /// that every struct and vector starts at a different offset.
  @Test(arguments: 0..<17)
  func testBuilderAPIAlignsStructs(prefix: Int) throws {
    var builder = FlatBufferBuilder(initialSize: 8)
    let bytes = builder.createVector(
      [UInt8](repeating: 0xAB, count: prefix))

    // The buffer is built back to front, so the elements are added last first.
    Holder.startVectorOfVec16(3, in: &builder)
    for value in [Int32(3), 2, 1] {
      builder.create(struct: Aligned16(a: value))
    }
    let vec16 = builder.endVector(len: 3)

    let vec8 = builder.createVector(ofStructs: [
      Aligned8(a: 1, b: 2),
      Aligned8(a: 3, b: 4),
      Aligned8(a: 5, b: 6),
    ])

    Holder.startVectorOfOuters(2, in: &builder)
    builder.create(struct: Outer(b: 2, inner: Aligned16(a: 20)))
    builder.create(struct: Outer(b: 1, inner: Aligned16(a: 10)))
    let outers = builder.endVector(len: 2)

    let vec2 = builder.createVector(ofStructs: [
      Aligned2(a: 1),
      Aligned2(a: 2),
      Aligned2(a: 3),
    ])

    let start = Holder.startHolder(&builder)
    Holder.addVectorOf(bytes: bytes, &builder)
    Holder.add(single: Aligned16(a: 42), &builder)
    Holder.addVectorOf(vec16: vec16, &builder)
    Holder.add(outer: Outer(b: 7, inner: Aligned16(a: 43)), &builder)
    Holder.addVectorOf(vec8: vec8, &builder)
    Holder.addVectorOf(outers: outers, &builder)
    Holder.addVectorOf(vec2: vec2, &builder)
    let root = Holder.endHolder(&builder, start: start)
    builder.finish(offset: root)

    try checkHolder(builder, prefix: prefix)
  }

  /// `pack` must also keep the order of the elements of struct vectors.
  @Test(arguments: 0..<17)
  func testObjectAPIAlignsStructs(prefix: Int) throws {
    var object = HolderT()
    object.bytes = [UInt8](repeating: 0xAB, count: prefix)
    object.single = Aligned16(a: 42)
    object.outer = Outer(b: 7, inner: Aligned16(a: 43))
    object.vec16 = [Aligned16(a: 1), Aligned16(a: 2), Aligned16(a: 3)]
    object.vec8 = [
      Aligned8(a: 1, b: 2),
      Aligned8(a: 3, b: 4),
      Aligned8(a: 5, b: 6),
    ]
    object.outers = [
      Outer(b: 1, inner: Aligned16(a: 10)),
      Outer(b: 2, inner: Aligned16(a: 20)),
    ]
    object.vec2 = [Aligned2(a: 1), Aligned2(a: 2), Aligned2(a: 3)]

    var builder = FlatBufferBuilder(initialSize: 8)
    let root = Holder.pack(&builder, obj: &object)
    builder.finish(offset: root)

    try checkHolder(builder, prefix: prefix)
  }

  private func checkHolder(_ builder: FlatBufferBuilder, prefix: Int) throws {
    let bytes = builder.sizedByteArray
    // The buffer size must be a multiple of the largest alignment used, so
    // that offsets relative to the start of the buffer stay aligned.
    #expect(bytes.count % 16 == 0)

    let layout = BufferLayout(bytes: bytes)
    #expect(layout.fieldPosition(Field.single)! % 16 == 0)
    #expect(layout.fieldPosition(Field.outer)! % 16 == 0)

    let vectors: [(field: Int, alignment: Int, count: Int)] = [
      (Field.bytes, 1, prefix),
      (Field.vec16, 16, 3),
      (Field.vec8, 8, 3),
      (Field.outers, 16, 2),
      (Field.vec2, 2, 3),
    ]
    for vector in vectors {
      let (count, data) = layout.vector(vector.field)
      // The length is immediately followed by the first element, and the
      // first element is aligned to the struct's alignment.
      #expect(count == vector.count)
      #expect(data % 4 == 0)
      #expect(data % vector.alignment == 0)
    }

    var buffer = builder.sizedBuffer
    let holder: Holder = try getCheckedRoot(byteBuffer: &buffer)
    #expect(holder.bytes.count == prefix)
    #expect(holder.single?.a == 42)
    #expect(holder.outer?.b == 7)
    #expect(holder.outer?.inner.a == 43)
    #expect(holder.vec16.map(\.a) == [1, 2, 3])
    #expect(holder.vec8.map(\.a) == [1, 3, 5])
    #expect(holder.vec8.map(\.b) == [2, 4, 6])
    #expect(holder.outers.map(\.b) == [1, 2])
    #expect(holder.outers.map(\.inner.a) == [10, 20])
    #expect(holder.vec2.map(\.a) == [1, 2, 3])
  }
}

/// Field indices of `Holder` in the vtable.
private enum Field {
  static let bytes = 0
  static let single = 1
  static let outer = 2
  static let vec16 = 3
  static let vec8 = 4
  static let outers = 5
  static let vec2 = 6
}

/// Reads the layout of a finished buffer whose root is a table, independently
/// of the generated readers.
private struct BufferLayout {
  let bytes: [UInt8]

  func uint16(_ position: Int) -> Int {
    Int(bytes[position]) | Int(bytes[position + 1]) << 8
  }

  func uint32(_ position: Int) -> Int {
    uint16(position) | uint16(position + 2) << 16
  }

  /// Position of the field with the given index in the root table.
  func fieldPosition(_ index: Int) -> Int? {
    let table = uint32(0)
    let vtable = table - Int(Int32(bitPattern: UInt32(uint32(table))))
    let entry = 4 + 2 * index
    guard entry < uint16(vtable) else { return nil }
    let offset = uint16(vtable + entry)
    return offset == 0 ? nil : table + offset
  }

  /// Length and position of the first element of a vector field.
  func vector(_ index: Int) -> (count: Int, data: Int) {
    let field = fieldPosition(index)!
    let vector = field + uint32(field)
    return (uint32(vector), vector + 4)
  }
}
