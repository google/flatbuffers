import XCTest
@testable import FlatBuffers

final class FlatBuffersStructsTests: XCTestCase {

    func testWritingAndMutatingBools() {
        var fbb = FlatBufferBuilder()
        let start = TestMutatingBool.startTestMutatingBool(&fbb)
        TestMutatingBool.add(b: createProperty(builder: &fbb), &fbb)
        let root = TestMutatingBool.endTestMutatingBool(&fbb, start: start)
        fbb.finish(offset: root)
        
        let testMutatingBool = TestMutatingBool.getRootAsTestMutatingBool(bb: fbb.sizedBuffer)
        let property = testMutatingBool.b
        XCTAssertEqual(property?.property, false)
        property?.mutate(property: false)
        XCTAssertEqual(property?.property, false)
        property?.mutate(property: true)
        XCTAssertEqual(property?.property, true)
    }
    
}

struct Vec: Readable {
    var __buffer: ByteBuffer! { __p.bb }

    static var size = 12
    static var alignment = 4
    private var __p: Struct
    init(_ fb: ByteBuffer, o: Int32) { __p = Struct(bb: fb, position: o) }
    var x: Float32 { return __p.readBuffer(of: Float32.self, at: 0)}
    var y: Float32 { return __p.readBuffer(of: Float32.self, at: 4)}
    var z: Float32 { return __p.readBuffer(of: Float32.self, at: 8)}
}

@discardableResult
func createVecWrite(builder: inout FlatBufferBuilder, x: Float32, y: Float32, z: Float32) -> Offset<UOffset> {
    builder.createStructOf(size: Vec.size, alignment: Vec.alignment)
    builder.reverseAdd(v: x, postion: 0)
    builder.reverseAdd(v: y, postion: 4)
    builder.reverseAdd(v: z, postion: 8)
    return builder.endStruct()
}
