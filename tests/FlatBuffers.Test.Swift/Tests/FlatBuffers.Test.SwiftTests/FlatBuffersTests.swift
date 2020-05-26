import XCTest
@testable import FlatBuffers

final class FlatBuffersTests: XCTestCase {

    let country = "Norway"
    
    func testEndian() { XCTAssertEqual(isLitteEndian, true) }

    func testOffset() {
        let o = Offset<Int>()
        let b = Offset<Int>(offset: 1)
        XCTAssertEqual(o.isEmpty, true)
        XCTAssertEqual(b.isEmpty, false)
    }
    
    func testCreateString() {
        let helloWorld = "Hello, world!"
        let b = FlatBufferBuilder(initialSize: 16)
        XCTAssertEqual(b.create(string: country).o, 12)
        XCTAssertEqual(b.create(string: helloWorld).o, 32)
        b.clear()
        XCTAssertEqual(b.create(string: helloWorld).o, 20)
        XCTAssertEqual(b.create(string: country).o, 32)
        b.clear()
        XCTAssertEqual(b.create(string: String(repeating: "a", count: 257)).o, 264)
    }
    
    func testStartTable() {
        let b = FlatBufferBuilder(initialSize: 16)
        XCTAssertNoThrow(b.startTable(with: 0))
        b.clear()
        XCTAssertEqual(b.create(string: country).o, 12)
        XCTAssertEqual(b.startTable(with: 0), 12)
    }
    
    func testCreate() {
        var b = FlatBufferBuilder(initialSize: 16)
        _ = Country.createCountry(builder: &b, name: country, log: 200, lan: 100)
        let v: [UInt8] = [10, 0, 16, 0, 4, 0, 8, 0, 12, 0, 10, 0, 0, 0, 12, 0, 0, 0, 100, 0, 0, 0, 200, 0, 0, 0, 6, 0, 0, 0, 78, 111, 114, 119, 97, 121, 0, 0]
        XCTAssertEqual(b.sizedByteArray, v)
    }
    
    func testCreateFinish() {
        var b = FlatBufferBuilder(initialSize: 16)
        let countryOff = Country.createCountry(builder: &b, name: country, log: 200, lan: 100)
        b.finish(offset: countryOff)
        let v: [UInt8] = [16, 0, 0, 0, 0, 0, 10, 0, 16, 0, 4, 0, 8, 0, 12, 0, 10, 0, 0, 0, 12, 0, 0, 0, 100, 0, 0, 0, 200, 0, 0, 0, 6, 0, 0, 0, 78, 111, 114, 119, 97, 121, 0, 0]
        XCTAssertEqual(b.sizedByteArray, v)
    }
    
    func testCreateFinishWithPrefix() {
        var b = FlatBufferBuilder(initialSize: 16)
        let countryOff = Country.createCountry(builder: &b, name: country, log: 200, lan: 100)
        b.finish(offset: countryOff, addPrefix: true)
        let v: [UInt8] = [44, 0, 0, 0, 16, 0, 0, 0, 0, 0, 10, 0, 16, 0, 4, 0, 8, 0, 12, 0, 10, 0, 0, 0, 12, 0, 0, 0, 100, 0, 0, 0, 200, 0, 0, 0, 6, 0, 0, 0, 78, 111, 114, 119, 97, 121, 0, 0]
        XCTAssertEqual(b.sizedByteArray, v)
    }
    
    func testReadCountry() {
        let v: [UInt8] = [16, 0, 0, 0, 0, 0, 10, 0, 16, 0, 4, 0, 8, 0, 12, 0, 10, 0, 0, 0, 12, 0, 0, 0, 100, 0, 0, 0, 200, 0, 0, 0, 6, 0, 0, 0, 78, 111, 114, 119, 97, 121, 0, 0]
        let buffer = ByteBuffer(bytes: v)
        let c = Country.getRootAsCountry(buffer)
        XCTAssertEqual(c.lan, 100)
        XCTAssertEqual(c.log, 200)
        XCTAssertEqual(c.nameVector, [78, 111, 114, 119, 97, 121])
        XCTAssertEqual(c.name, country)
    }
}

class Country {
    
    static let offsets: (name: VOffset, lan: VOffset, lng: VOffset) = (0, 1, 2)
    private var __t: Table
    
    private init(_ t: Table) {
        __t = t
    }
    
    var lan: Int32 { let o = __t.offset(6); return o == 0 ? 0 : __t.readBuffer(of: Int32.self, at: o) }
    var log: Int32 { let o = __t.offset(8); return o == 0 ? 0 : __t.readBuffer(of: Int32.self, at: o) }
    var nameVector: [UInt8]? { return __t.getVector(at: 4) }
    var name: String? { let o = __t.offset(4); return o == 0 ? nil : __t.string(at: o) }
    
    @inlinable static func getRootAsCountry(_ bb: ByteBuffer) -> Country {
        return Country(Table(bb: bb, position: Int32(bb.read(def: UOffset.self, position: 0))))
    }
    
    @inlinable static func createCountry(builder: inout FlatBufferBuilder, name: String, log: Int32, lan: Int32) -> Offset<Country> {
        return createCountry(builder: &builder, offset: builder.create(string: name), log: log, lan: lan)
    }
    
    @inlinable static func createCountry(builder: inout FlatBufferBuilder, offset: Offset<String>, log: Int32, lan: Int32) -> Offset<Country> {
        let _start = builder.startTable(with: 3)
        Country.add(builder: &builder, lng: log)
        Country.add(builder: &builder, lan: lan)
        Country.add(builder: &builder, name: offset)
        return Country.end(builder: &builder, startOffset: _start)
    }
    
    @inlinable static func end(builder: inout FlatBufferBuilder, startOffset: UOffset) -> Offset<Country> {
        return Offset(offset: builder.endTable(at: startOffset))
    }
    
    @inlinable static func add(builder: inout FlatBufferBuilder, name: String) {
        add(builder: &builder, name: builder.create(string: name))
    }
    
    @inlinable static func add(builder: inout FlatBufferBuilder, name: Offset<String>) {
        builder.add(offset: name, at: Country.offsets.name)
    }
    
    @inlinable static func add(builder: inout FlatBufferBuilder, lan: Int32) {
        builder.add(element: lan, def: 0, at: Country.offsets.lan)
    }
    
    @inlinable static func add(builder: inout FlatBufferBuilder, lng: Int32) {
        builder.add(element: lng, def: 0, at: Country.offsets.lng)
    }
}
