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

import Foundation
import Testing

import FlatBuffers

struct FlatbuffersInputValidationTests {

  // MARK: - Table.compare

  /// Builds a buffer hosting two length-prefixed, null-terminated strings
  /// addressed by two uoffset slots (the layout generated code uses for
  /// keyed vectors of tables).
  private func compareBuffer(_ a: String, _ b: String) -> ByteBuffer {
    let aBytes = Array(a.utf8)
    let bBytes = Array(b.utf8)
    var bytes: [UInt8] = []
    func appendUInt32(_ value: UInt32) {
      withUnsafeBytes(of: value.littleEndian) { bytes.append(contentsOf: $0) }
    }

    let stringAPosition = 8
    let stringBPosition = stringAPosition + 4 + aBytes.count + 1

    // uoffset slots store the forward distance to their target.
    appendUInt32(UInt32(stringAPosition))
    appendUInt32(UInt32(stringBPosition - 4))
    appendUInt32(UInt32(aBytes.count))
    bytes.append(contentsOf: aBytes)
    bytes.append(0)
    appendUInt32(UInt32(bBytes.count))
    bytes.append(contentsOf: bBytes)
    bytes.append(0)
    return ByteBuffer(bytes: bytes)
  }

  @Test(
    "Table.compare orders strings by their UTF-8 bytes",
    arguments: [
      ("abc", "abc"),
      ("abc", "abd"),
      ("abc", "ab"),
      ("ab", "abc"),
      ("", ""),
      ("", "a"),
      ("a", ""),
      ("a", "b"),
      ("b", "a"),
      ("a\0b", "a\0c"),
      ("a\0b", "a\0b"),
    ])
  func testTableCompareOrdersByUTF8Bytes(a: String, b: String) {
    let bb = compareBuffer(a, b)
    let aPrecedes = Array(a.utf8).lexicographicallyPrecedes(Array(b.utf8))
    let equal = a == b

    // The two-offset variant returns (second - first).
    let twoOffsets = Table.compare(0, 4, fbb: bb)
    #expect((twoOffsets > 0) == aPrecedes)
    #expect((twoOffsets == 0) == equal)
    #expect((twoOffsets < 0) == (!aPrecedes && !equal))

    // The key variant returns (buffer - key).
    let keyVariant = Table.compare(0, Array(b.utf8), fbb: bb)
    #expect((keyVariant < 0) == aPrecedes)
    #expect((keyVariant == 0) == equal)
    #expect((keyVariant > 0) == (!aPrecedes && !equal))
  }

  // MARK: - ByteBuffer bounds

  @Test
  func testReadStringOutOfBoundsReturnsNil() {
    let bb = ByteBuffer(bytes: Array("abc".utf8))
    #expect(bb.readString(at: 0, count: 3) == "abc")
    #expect(bb.readString(at: 0, count: 4) == nil)
    #expect(bb.readString(at: 2, count: 2) == nil)
    #expect(bb.readString(at: 0, count: -1) == nil)
    #expect(bb.readString(at: -1, count: 1) == nil)
  }

  @Test
  func testReadSliceStillReadsInBounds() {
    let bb = ByteBuffer(bytes: [1, 0, 0, 0, 7, 0, 0, 0])
    let values: [Int32] = bb.readSlice(index: 0, count: 2)
    #expect(values == [1, 7])
  }

  // MARK: - Corrupted string offsets

  @Test
  func testStringInitWithCorruptedOffsetsIsEmpty() {
    let bb = ByteBuffer(bytes: Array("abc".utf8))
    // A string pointing outside of the buffer resolves to "" instead of
    // reading out of bounds.
    #expect(String(bb, o: 100) == "")
    #expect(String(bb, o: -5) == "")
    #expect(String(bb, o: 1) == "")
    // In-bounds offset with a length that exceeds the buffer also
    // resolves to "".
    #expect(String(bb, o: 0) == "")
  }
}
