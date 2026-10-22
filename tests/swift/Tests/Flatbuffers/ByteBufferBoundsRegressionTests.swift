import XCTest

@testable import FlatBuffers

/// Regression coverage for the release-enforced read guards.
///
/// `read`, `readSlice`, `readString`, `readDirectString`, and `skipPrefix`
/// guard against out-of-bounds reads with `precondition` (enforced in release
/// builds). These tests pin the in-bounds behavior so the guards cannot
/// accidentally reject well-formed access.
final class ByteBufferBoundsRegressionTests: XCTestCase {

  func testReadSliceInBounds() {
    let buffer = ByteBuffer(bytes: [1, 2, 3, 4, 5, 6, 7, 8])
    let slice: [UInt8] = buffer.readSlice(index: 2, count: 4)
    XCTAssertEqual(slice, [3, 4, 5, 6])
  }

  func testReadStringInBounds() {
    let buffer = ByteBuffer(bytes: Array("hello flatbuffers".utf8))
    let str = buffer.readString(at: 0, count: 5)
    XCTAssertEqual(str, "hello")
  }

  func testReadStringInteriorInBounds() {
    let buffer = ByteBuffer(bytes: Array("swift".utf8))
    let str = buffer.readString(at: 1, count: 4)
    XCTAssertEqual(str, "wift")
  }

  func testSkipPrefixOnPrefixedBuffer() {
    var buffer = ByteBuffer(bytes: [0x03, 0x00, 0x00, 0x00, 0xAA, 0xBB, 0xCC])
    let prefix = buffer.skipPrefix()
    XCTAssertEqual(prefix, 3)
    XCTAssertEqual(buffer.read(def: UInt8.self, position: 4), 0xAA) // position is _readerIndex-relative
  }
}
