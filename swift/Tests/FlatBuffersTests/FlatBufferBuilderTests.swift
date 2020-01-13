import FlatBuffers
import XCTest

final class FlatBufferBuilderTests: XCTestCase {
    func testCreateString() throws {
        let fbb = FlatBufferBuilder()
        XCTAssertEqual(fbb.create(string: String(repeating: "a", count: 257)).isEmpty, false)
    }
}
