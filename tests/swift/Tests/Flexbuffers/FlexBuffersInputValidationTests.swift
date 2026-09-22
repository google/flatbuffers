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

@testable import FlexBuffers

struct FlexBuffersInputValidationTests {

  // MARK: - Writer: UTF-8 byte lengths

  @Test
  func testUnicodeStringsRoundTrip() throws {
    var flx = FlexBuffersWriter()
    flx.vector {
      $0.add(string: "héllo")
      $0.add(string: "日本語")
      $0.add(string: "a🎉b")
    }
    flx.finish()

    let reference = try getRoot(buffer: flx.byteBuffer)!
    let vector = reference.vector!
    #expect(vector.count == 3)
    #expect(vector[0]?.string() == "héllo")
    #expect(vector[1]?.string() == "日本語")
    #expect(vector[2]?.string() == "a🎉b")
  }

  @Test
  func testUnicodeMapKeysRoundTrip() throws {
    var flx = FlexBuffersWriter()
    flx.map {
      $0.add(string: "café au lait", key: "café")
      $0.add(int32: 42, key: "日本語")
    }
    flx.finish()

    let reference = try getRoot(buffer: flx.byteBuffer)!
    let map = reference.map!
    #expect(map.count == 2)
    #expect(map["café"]?.string() == "café au lait")
    #expect(map["日本語"]?.int == 42)
    #expect(map["zzzz"] == nil)
    #expect(map[""] == nil)
  }

  // MARK: - Reader: map lookups stay within the keys vector

  @Test
  func testMapLookupWithTargetOutsideOfKeyRange() throws {
    var flx = FlexBuffersWriter()
    flx.map {
      $0.add(int32: 1, key: "a")
      $0.add(int32: 2, key: "b")
      $0.add(int32: 3, key: "c")
    }
    flx.finish()

    let reference = try getRoot(buffer: flx.byteBuffer)!
    let map = reference.map!
    #expect(map["a"]?.int == 1)
    #expect(map["b"]?.int == 2)
    #expect(map["c"]?.int == 3)
    // Looking up a key greater than every stored key used to probe one
    // element past the end of the keys vector.
    #expect(map["zzz"] == nil)
    #expect(map["0"] == nil)
  }

  // MARK: - Reader: structural validation

  @Test
  func testGetRootRejectsInvalidByteWidth() {
    // The last byte of a flexbuffer encodes the root byte width; widths
    // other than 1, 2, 4, 8 are structurally invalid.
    #expect(throws: FlexBuffersErrors.self) {
      _ = try getRoot(buffer: ByteBuffer(bytes: [0, 0, 0, 0, 0, 3]))
    }
    #expect(throws: FlexBuffersErrors.self) {
      _ = try getRoot(buffer: ByteBuffer(bytes: [0, 0, 0]))
    }
  }

  @Test
  func testGetRootRejectsByteWidthLargerThanBuffer() {
    // A root byte width of 8 cannot be addressed by a 3 byte buffer.
    #expect(throws: FlexBuffersErrors.self) {
      _ = try getRoot(buffer: ByteBuffer(bytes: [0, 0, 8]))
    }
  }

  @Test
  func testGetRootStillReadsValidBuffers() throws {
    var flx = FlexBuffersWriter()
    flx.map {
      $0.add(bool: true, key: "ok")
    }
    flx.finish()

    let reference = try getRoot(buffer: flx.byteBuffer)!
    #expect(reference.map?["ok"]?.bool == true)
  }

  @Test
  func testReadStringOutOfBoundsReturnsNil() {
    let bb = ByteBuffer(bytes: Array("abc".utf8))
    #expect(bb.readString(at: 0, count: 3, type: .utf8) == "abc")
    #expect(bb.readString(at: 0, count: 4, type: .utf8) == nil)
    #expect(bb.readString(at: 0, count: -1, type: .utf8) == nil)
    #expect(bb.readString(at: 0, count: 4) == nil)
  }

  // MARK: - Value.elementWidth

  @Test
  func testElementWidthUsesPowerOfTwoByteWidths() {
    // A value stored 1000 bytes away from the current write location fits
    // into a 16-bit offset, so elementWidth has to resolve to .w16. The
    // width search previously iterated over 1, 3, 5, 7 instead of
    // 1, 2, 4, 8, which made every offset wider than a byte fall back
    // to .w64.
    let wide = Value(v: UInt64(1000), type: .indirectUInt, bitWidth: .w8)
    #expect(wide.elementWidth(size: 2000, index: 0) == .w16)

    let small = Value(v: UInt64(5), type: .indirectUInt, bitWidth: .w8)
    #expect(small.elementWidth(size: 10, index: 1) == .w8)
  }
}
