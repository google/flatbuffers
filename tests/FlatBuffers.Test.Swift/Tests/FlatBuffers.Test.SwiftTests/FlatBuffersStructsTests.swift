import XCTest
@testable import FlatBuffers

final class FlatBuffersStructsTests: XCTestCase {
    
    func testCreatingStruct() {
        let v = createVecWrite(x: 1.0, y: 2.0, z: 3.0)
        let b = FlatBufferBuilder(initialSize: 20)
        let o = b.create(struct: v, type: Vec.self)
        let end = VPointerVec.createVPointer(b: b, o: o)
        b.finish(offset: end)
        XCTAssertEqual(b.sizedByteArray, [12, 0, 0, 0, 0, 0, 6, 0, 4, 0, 4, 0, 6, 0, 0, 0, 0, 0, 128, 63, 0, 0, 0, 64, 0, 0, 64, 64])
    }
    
    func testReadingStruct() {
        let v = createVecWrite(x: 1.0, y: 2.0, z: 3.0)
        let b = FlatBufferBuilder(initialSize: 20)
        let o = b.create(struct: v, type: Vec.self)
        let end = VPointerVec.createVPointer(b: b, o: o)
        b.finish(offset: end)
        let buffer = b.sizedByteArray
        XCTAssertEqual(buffer, [12, 0, 0, 0, 0, 0, 6, 0, 4, 0, 4, 0, 6, 0, 0, 0, 0, 0, 128, 63, 0, 0, 0, 64, 0, 0, 64, 64])
        let point = VPointerVec.getRootAsCountry(ByteBuffer(bytes: buffer))
        XCTAssertEqual(point.vec?.z, 3.0)
    }
    
    func testCreatingVectorStruct() {
        let b = FlatBufferBuilder(initialSize: 20)
        let path = b.createVector(structs: [createVecWrite(x: 1, y: 2, z: 3), createVecWrite(x: 4.0, y: 5.0, z: 6)], type: Vec.self)
        let end = VPointerVectorVec.createVPointer(b: b, v: path)
        b.finish(offset: end)
        XCTAssertEqual(b.sizedByteArray, [12, 0, 0, 0, 8, 0, 8, 0, 0, 0, 4, 0, 8, 0, 0, 0, 4, 0, 0, 0, 2, 0, 0, 0, 0, 0, 128, 63, 0, 0, 0, 64, 0, 0, 64, 64, 0, 0, 128, 64, 0, 0, 160, 64, 0, 0, 192, 64])
    }
    
    func testCreatingVectorStructWithForcedDefaults() {
        let b = FlatBufferBuilder(initialSize: 20, serializeDefaults: true)
        let path = b.createVector(structs: [createVecWrite(x: 1, y: 2, z: 3), createVecWrite(x: 4.0, y: 5.0, z: 6)], type: Vec.self)
        let end = VPointerVectorVec.createVPointer(b: b, v: path)
        b.finish(offset: end)
        XCTAssertEqual(b.sizedByteArray, [12, 0, 0, 0, 8, 0, 12, 0, 4, 0, 8, 0, 8, 0, 0, 0, 1, 0, 0, 0, 4, 0, 0, 0, 2, 0, 0, 0, 0, 0, 128, 63, 0, 0, 0, 64, 0, 0, 64, 64, 0, 0, 128, 64, 0, 0, 160, 64, 0, 0, 192, 64])
    }
    
    func testCreatingEnums() {
        let b = FlatBufferBuilder(initialSize: 20)
        let path = b.createVector(structs: [createVecWrite(x: 1, y: 2, z: 3), createVecWrite(x: 4, y: 5, z: 6)], type: Vec.self)
        let end = VPointerVectorVec.createVPointer(b: b, color: .blue, v: path)
        b.finish(offset: end)
        XCTAssertEqual(b.sizedByteArray, [12, 0, 0, 0, 8, 0, 12, 0, 4, 0, 8, 0, 8, 0, 0, 0, 2, 0, 0, 0, 4, 0, 0, 0, 2, 0, 0, 0, 0, 0, 128, 63, 0, 0, 0, 64, 0, 0, 64, 64, 0, 0, 128, 64, 0, 0, 160, 64, 0, 0, 192, 64])
    }
    
    func testReadingStructWithEnums() {
        let b = FlatBufferBuilder(initialSize: 20)
        let vec = createVec2(x: 1, y: 2, z: 3, color: .red)
        let o = b.create(struct: vec, type: Vec2.self)
        let end = VPointerVec2.createVPointer(b: b, o: o, type: .vec)
        b.finish(offset: end)
        let buffer = b.sizedByteArray
        XCTAssertEqual(buffer, [16, 0, 0, 0, 0, 0, 10, 0, 12, 0, 12, 0, 11, 0, 4, 0, 10, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 1, 0, 0, 128, 63, 0, 0, 0, 64, 0, 0, 64, 64, 0, 0, 0, 0])
        let point = VPointerVec2.getRootAsCountry(ByteBuffer(bytes: buffer))
        XCTAssertEqual(point.vec?.c, Color2.red)
        XCTAssertEqual(point.vec?.x, 1.0)
        XCTAssertEqual(point.vec?.y, 2.0)
        XCTAssertEqual(point.vec?.z, 3.0)
        XCTAssertEqual(point.UType, Test.vec)
    }
    
}

func createVecWrite(x: Float32, y: Float32, z: Float32) -> UnsafeMutableRawPointer{
    let memory = UnsafeMutableRawPointer.allocate(byteCount: Vec.size, alignment: Vec.alignment)
    memory.initializeMemory(as: UInt8.self, repeating: 0, count: Vec.size)
    memory.storeBytes(of: x, toByteOffset: 0, as: Float32.self)
    memory.storeBytes(of: y, toByteOffset: 4, as: Float32.self)
    memory.storeBytes(of: z, toByteOffset: 8, as: Float32.self)
    return memory
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

struct VPointerVec {
    
    private var __t: Table
    
    private init(_ t: Table) {
        __t = t
    }
    
    var vec: Vec? { let o = __t.offset(4); return o == 0 ? nil : Vec(__t.bb, o: o + __t.postion) }
    
    @inlinable static func getRootAsCountry(_ bb: ByteBuffer) -> VPointerVec {
        return VPointerVec(Table(bb: bb, position: Int32(bb.read(def: UOffset.self, position: 0))))
    }
    
    static func startVPointer(b: FlatBufferBuilder) -> UOffset { b.startTable(with: 1) }
    static func finish(b: FlatBufferBuilder, s: UOffset) -> Offset<UOffset> { return Offset(offset: b.endTable(at: s)) }
    
    static func createVPointer(b: FlatBufferBuilder, o: Offset<UOffset>) -> Offset<UOffset> {
        let s = VPointerVec.startVPointer(b: b)
        b.add(structOffset: 0)
        return VPointerVec.finish(b: b, s: s)
    }
}

enum Color: UInt32 { case red = 0, green = 1, blue = 2 }

private let VPointerVectorVecOffsets: (color: VOffset, vector: VOffset) = (0, 1)

struct VPointerVectorVec {
    
    static func startVPointer(b: FlatBufferBuilder) -> UOffset { b.startTable(with: 2) }
    
    static func addVector(b: FlatBufferBuilder, v: Offset<UOffset>) { b.add(offset: v, at: VPointerVectorVecOffsets.vector) }
    
    static func addColor(b: FlatBufferBuilder, color: Color) { b.add(element: color.rawValue, def: 1, at: VPointerVectorVecOffsets.color) }
    
    static func finish(b: FlatBufferBuilder, s: UOffset) -> Offset<UOffset> { return Offset(offset: b.endTable(at: s)) }
    
    static func createVPointer(b: FlatBufferBuilder, color: Color = .green, v: Offset<UOffset>) -> Offset<UOffset> {
        let s = VPointerVectorVec.startVPointer(b: b)
        VPointerVectorVec.addVector(b: b, v: v)
        VPointerVectorVec.addColor(b: b, color: color)
        return VPointerVectorVec.finish(b: b, s: s)
    }
}

enum Color2: Int32 { case red = 0, green = 1, blue = 2 }
enum Test: Byte { case none = 0, vec = 1 }

func createVec2(x: Float32 = 0, y: Float32 = 0, z: Float32 = 0, color: Color2) -> UnsafeMutableRawPointer {
    let memory = UnsafeMutableRawPointer.allocate(byteCount: Vec2.size, alignment: Vec2.alignment)
    memory.initializeMemory(as: UInt8.self, repeating: 0, count: Vec2.size)
    memory.storeBytes(of: x, toByteOffset: 0, as: Float32.self)
    memory.storeBytes(of: y, toByteOffset: 4, as: Float32.self)
    memory.storeBytes(of: z, toByteOffset: 8, as: Float32.self)
    return memory
}

struct Vec2: Readable {
    var __buffer: ByteBuffer! { __p.bb }
    
    static var size = 13
    static var alignment = 4
    private var __p: Struct
    
    init(_ fb: ByteBuffer, o: Int32) { __p = Struct(bb: fb, position: o) }
    var c: Color2 { return Color2(rawValue: __p.readBuffer(of: Int32.self, at: 12)) ?? .red }
    var x: Float32 { return __p.readBuffer(of: Float32.self, at: 0)}
    var y: Float32 { return __p.readBuffer(of: Float32.self, at: 4)}
    var z: Float32 { return __p.readBuffer(of: Float32.self, at: 8)}
}

struct VPointerVec2 {
    
    private var __t: Table
    
    private init(_ t: Table) {
        __t = t
    }
    
    var vec: Vec2? { let o = __t.offset(4); return o == 0 ? nil : Vec2( __t.bb, o: o + __t.postion) }
    var UType: Test? { let o = __t.offset(6); return o == 0 ? Test.none : Test(rawValue: __t.readBuffer(of: Byte.self, at: o)) }
    
    @inlinable static func getRootAsCountry(_ bb: ByteBuffer) -> VPointerVec2 {
        return VPointerVec2(Table(bb: bb, position: Int32(bb.read(def: UOffset.self, position: 0))))
    }
    
    static func startVPointer(b: FlatBufferBuilder) -> UOffset { b.startTable(with: 3) }
    static func finish(b: FlatBufferBuilder, s: UOffset) -> Offset<UOffset> { return Offset(offset: b.endTable(at: s)) }
    
    static func createVPointer(b: FlatBufferBuilder, o: Offset<UOffset>, type: Test) -> Offset<UOffset> {
        let s = VPointerVec2.startVPointer(b: b)
        b.add(structOffset: 0)
        b.add(element: type.rawValue, def: Test.none.rawValue, at: 1)
        b.add(offset: o, at: 2)
        return VPointerVec2.finish(b: b, s: s)
    }
}
