import XCTest
import Foundation
@testable import FlatBuffers

typealias Test1 = MyGame.Example.Test
typealias Monster1 = MyGame.Example.Monster
typealias Vec3 = MyGame.Example.Vec3

class FlatBuffersMonsterWriterTests: XCTestCase {
    
    func testData() {
        let data = Data([48, 0, 0, 0, 77, 79, 78, 83, 0, 0, 0, 0, 36, 0, 72, 0, 40, 0, 0, 0, 38, 0, 32, 0, 0, 0, 28, 0, 0, 0, 27, 0, 20, 0, 16, 0, 12, 0, 4, 0, 0, 0, 0, 0, 0, 0, 11, 0, 36, 0, 0, 0, 164, 0, 0, 0, 0, 0, 0, 1, 60, 0, 0, 0, 68, 0, 0, 0, 76, 0, 0, 0, 0, 0, 0, 1, 88, 0, 0, 0, 120, 0, 0, 0, 0, 0, 80, 0, 0, 0, 128, 63, 0, 0, 0, 64, 0, 0, 64, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 64, 2, 0, 5, 0, 6, 0, 0, 0, 2, 0, 0, 0, 64, 0, 0, 0, 48, 0, 0, 0, 2, 0, 0, 0, 30, 0, 40, 0, 10, 0, 20, 0, 152, 255, 255, 255, 4, 0, 0, 0, 4, 0, 0, 0, 70, 114, 101, 100, 0, 0, 0, 0, 5, 0, 0, 0, 0, 1, 2, 3, 4, 0, 0, 0, 5, 0, 0, 0, 116, 101, 115, 116, 50, 0, 0, 0, 5, 0, 0, 0, 116, 101, 115, 116, 49, 0, 0, 0, 9, 0, 0, 0, 77, 121, 77, 111, 110, 115, 116, 101, 114, 0, 0, 0, 3, 0, 0, 0, 20, 0, 0, 0, 36, 0, 0, 0, 4, 0, 0, 0, 240, 255, 255, 255, 32, 0, 0, 0, 248, 255, 255, 255, 36, 0, 0, 0, 12, 0, 8, 0, 0, 0, 0, 0, 0, 0, 4, 0, 12, 0, 0, 0, 28, 0, 0, 0, 5, 0, 0, 0, 87, 105, 108, 109, 97, 0, 0, 0, 6, 0, 0, 0, 66, 97, 114, 110, 101, 121, 0, 0, 5, 0, 0, 0, 70, 114, 111, 100, 111, 0, 0, 0])
        let _data = ByteBuffer(data: data)
        readMonster(fb: _data)
    }

    func testReadFromOtherLangagues() {
        let path = FileManager.default.currentDirectoryPath
        let url = URL(fileURLWithPath: path, isDirectory: true).appendingPathComponent("monsterdata_test").appendingPathExtension("mon")
        guard let data = try? Data(contentsOf: url) else { return }
        let _data = ByteBuffer(data: data)
        readMonster(fb: _data)
    }
    
    func testCreateMonster() {
        let bytes = createMonster(withPrefix: false)
        XCTAssertEqual(bytes.sizedByteArray, [48, 0, 0, 0, 77, 79, 78, 83, 0, 0, 0, 0, 36, 0, 72, 0, 40, 0, 0, 0, 38, 0, 32, 0, 0, 0, 28, 0, 0, 0, 27, 0, 20, 0, 16, 0, 12, 0, 4, 0, 0, 0, 0, 0, 0, 0, 11, 0, 36, 0, 0, 0, 164, 0, 0, 0, 0, 0, 0, 1, 60, 0, 0, 0, 68, 0, 0, 0, 76, 0, 0, 0, 0, 0, 0, 1, 88, 0, 0, 0, 120, 0, 0, 0, 0, 0, 80, 0, 0, 0, 128, 63, 0, 0, 0, 64, 0, 0, 64, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 64, 2, 0, 5, 0, 6, 0, 0, 0, 2, 0, 0, 0, 64, 0, 0, 0, 48, 0, 0, 0, 2, 0, 0, 0, 30, 0, 40, 0, 10, 0, 20, 0, 152, 255, 255, 255, 4, 0, 0, 0, 4, 0, 0, 0, 70, 114, 101, 100, 0, 0, 0, 0, 5, 0, 0, 0, 0, 1, 2, 3, 4, 0, 0, 0, 5, 0, 0, 0, 116, 101, 115, 116, 50, 0, 0, 0, 5, 0, 0, 0, 116, 101, 115, 116, 49, 0, 0, 0, 9, 0, 0, 0, 77, 121, 77, 111, 110, 115, 116, 101, 114, 0, 0, 0, 3, 0, 0, 0, 20, 0, 0, 0, 36, 0, 0, 0, 4, 0, 0, 0, 240, 255, 255, 255, 32, 0, 0, 0, 248, 255, 255, 255, 36, 0, 0, 0, 12, 0, 8, 0, 0, 0, 0, 0, 0, 0, 4, 0, 12, 0, 0, 0, 28, 0, 0, 0, 5, 0, 0, 0, 87, 105, 108, 109, 97, 0, 0, 0, 6, 0, 0, 0, 66, 97, 114, 110, 101, 121, 0, 0, 5, 0, 0, 0, 70, 114, 111, 100, 111, 0, 0, 0])
        readMonster(fb: bytes.buffer)
        mutateMonster(fb: bytes.buffer)
        readMonster(fb: bytes.buffer)
    }

    func testCreateMonsterResizedBuffer() {
        let bytes = createMonster(withPrefix: false)
        XCTAssertEqual(bytes.sizedByteArray, [48, 0, 0, 0, 77, 79, 78, 83, 0, 0, 0, 0, 36, 0, 72, 0, 40, 0, 0, 0, 38, 0, 32, 0, 0, 0, 28, 0, 0, 0, 27, 0, 20, 0, 16, 0, 12, 0, 4, 0, 0, 0, 0, 0, 0, 0, 11, 0, 36, 0, 0, 0, 164, 0, 0, 0, 0, 0, 0, 1, 60, 0, 0, 0, 68, 0, 0, 0, 76, 0, 0, 0, 0, 0, 0, 1, 88, 0, 0, 0, 120, 0, 0, 0, 0, 0, 80, 0, 0, 0, 128, 63, 0, 0, 0, 64, 0, 0, 64, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 64, 2, 0, 5, 0, 6, 0, 0, 0, 2, 0, 0, 0, 64, 0, 0, 0, 48, 0, 0, 0, 2, 0, 0, 0, 30, 0, 40, 0, 10, 0, 20, 0, 152, 255, 255, 255, 4, 0, 0, 0, 4, 0, 0, 0, 70, 114, 101, 100, 0, 0, 0, 0, 5, 0, 0, 0, 0, 1, 2, 3, 4, 0, 0, 0, 5, 0, 0, 0, 116, 101, 115, 116, 50, 0, 0, 0, 5, 0, 0, 0, 116, 101, 115, 116, 49, 0, 0, 0, 9, 0, 0, 0, 77, 121, 77, 111, 110, 115, 116, 101, 114, 0, 0, 0, 3, 0, 0, 0, 20, 0, 0, 0, 36, 0, 0, 0, 4, 0, 0, 0, 240, 255, 255, 255, 32, 0, 0, 0, 248, 255, 255, 255, 36, 0, 0, 0, 12, 0, 8, 0, 0, 0, 0, 0, 0, 0, 4, 0, 12, 0, 0, 0, 28, 0, 0, 0, 5, 0, 0, 0, 87, 105, 108, 109, 97, 0, 0, 0, 6, 0, 0, 0, 66, 97, 114, 110, 101, 121, 0, 0, 5, 0, 0, 0, 70, 114, 111, 100, 111, 0, 0, 0])
        readMonster(fb: ByteBuffer(data: bytes.data))
    }

    func testCreateMonsterPrefixed() {
        let bytes = createMonster(withPrefix: true)
        XCTAssertEqual(bytes.sizedByteArray, [44, 1, 0, 0, 44, 0, 0, 0, 77, 79, 78, 83, 36, 0, 72, 0, 40, 0, 0, 0, 38, 0, 32, 0, 0, 0, 28, 0, 0, 0, 27, 0, 20, 0, 16, 0, 12, 0, 4, 0, 0, 0, 0, 0, 0, 0, 11, 0, 36, 0, 0, 0, 164, 0, 0, 0, 0, 0, 0, 1, 60, 0, 0, 0, 68, 0, 0, 0, 76, 0, 0, 0, 0, 0, 0, 1, 88, 0, 0, 0, 120, 0, 0, 0, 0, 0, 80, 0, 0, 0, 128, 63, 0, 0, 0, 64, 0, 0, 64, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 64, 2, 0, 5, 0, 6, 0, 0, 0, 2, 0, 0, 0, 64, 0, 0, 0, 48, 0, 0, 0, 2, 0, 0, 0, 30, 0, 40, 0, 10, 0, 20, 0, 152, 255, 255, 255, 4, 0, 0, 0, 4, 0, 0, 0, 70, 114, 101, 100, 0, 0, 0, 0, 5, 0, 0, 0, 0, 1, 2, 3, 4, 0, 0, 0, 5, 0, 0, 0, 116, 101, 115, 116, 50, 0, 0, 0, 5, 0, 0, 0, 116, 101, 115, 116, 49, 0, 0, 0, 9, 0, 0, 0, 77, 121, 77, 111, 110, 115, 116, 101, 114, 0, 0, 0, 3, 0, 0, 0, 20, 0, 0, 0, 36, 0, 0, 0, 4, 0, 0, 0, 240, 255, 255, 255, 32, 0, 0, 0, 248, 255, 255, 255, 36, 0, 0, 0, 12, 0, 8, 0, 0, 0, 0, 0, 0, 0, 4, 0, 12, 0, 0, 0, 28, 0, 0, 0, 5, 0, 0, 0, 87, 105, 108, 109, 97, 0, 0, 0, 6, 0, 0, 0, 66, 97, 114, 110, 101, 121, 0, 0, 5, 0, 0, 0, 70, 114, 111, 100, 111, 0, 0, 0])

        let newBuf = FlatBuffersUtils.removeSizePrefix(bb: bytes.buffer)
        readMonster(fb: newBuf)
    }

    func createMonster(withPrefix prefix: Bool) -> FlatBufferBuilder {
        let fbb = FlatBufferBuilder(initialSize: 1)
        let names = [fbb.create(string: "Frodo"), fbb.create(string: "Barney"), fbb.create(string: "Wilma")]
        var offsets: [Offset<UOffset>] = []
        let start1 = Monster1.startMonster(fbb)
        Monster1.add(name: names[0], fbb)
        offsets.append(Monster1.endMonster(fbb, start: start1))
        let start2 = Monster1.startMonster(fbb)
        Monster1.add(name: names[1], fbb)
        offsets.append(Monster1.endMonster(fbb, start: start2))
        let start3 = Monster1.startMonster(fbb)
        Monster1.add(name: names[2], fbb)
        offsets.append(Monster1.endMonster(fbb, start: start3))

        let sortedArray = Monster1.sortVectorOfMonster(offsets: offsets, fbb)

        let str = fbb.create(string: "MyMonster")
        let test1 = fbb.create(string: "test1")
        let test2 = fbb.create(string: "test2")
        let _inv: [Byte] = [0, 1, 2, 3, 4]
        let inv = fbb.createVector(_inv)

        let fred = fbb.create(string: "Fred")
        let mon1Start = Monster1.startMonster(fbb)
        Monster1.add(name: fred, fbb)
        let mon2 = Monster1.endMonster(fbb, start: mon1Start)
        let test4 = fbb.createVector(structs: [MyGame.Example.createTest(a: 30, b: 40),
                                               MyGame.Example.createTest(a: 10, b: 20)],
                                     type: Test1.self)

        let stringTestVector = fbb.createVector(ofOffsets: [test1, test2])

        let mStart = Monster1.startMonster(fbb)
        let posOffset = fbb.create(struct: MyGame.Example.createVec3(x: 1, y: 2, z: 3, test1: 3, test2: .green, test3a: 5, test3b: 6), type: Vec3.self)
        Monster1.add(pos: posOffset, fbb)
        Monster1.add(hp: 80, fbb)
        Monster1.add(name: str, fbb)
        Monster1.addVectorOf(inventory: inv, fbb)
        Monster1.add(testType: .monster, fbb)
        Monster1.add(test: mon2, fbb)
        Monster1.addVectorOf(test4: test4, fbb)
        Monster1.addVectorOf(testarrayofstring: stringTestVector, fbb)
        Monster1.add(testbool: true, fbb)
        Monster1.addVectorOf(testarrayoftables: sortedArray, fbb)
        let end = Monster1.endMonster(fbb, start: mStart)
        Monster1.finish(fbb, end: end, prefix: prefix)
        return fbb
    }

    func mutateMonster(fb: ByteBuffer) {
        let monster = Monster1.getRootAsMonster(bb: fb)
        XCTAssertFalse(monster.mutate(mana: 10))
        XCTAssertEqual(monster.testarrayoftables(at: 0)?.name, "Barney")
        XCTAssertEqual(monster.testarrayoftables(at: 1)?.name, "Frodo")
        XCTAssertEqual(monster.testarrayoftables(at: 2)?.name, "Wilma")

        // Example of searching for a table by the key
        XCTAssertNotNil(monster.testarrayoftablesBy(key: "Frodo"))
        XCTAssertNotNil(monster.testarrayoftablesBy(key: "Barney"))
        XCTAssertNotNil(monster.testarrayoftablesBy(key: "Wilma"))

        XCTAssertEqual(monster.testType, .monster)

        XCTAssertEqual(monster.mutate(inventory: 1, at: 0), true)
        XCTAssertEqual(monster.mutate(inventory: 2, at: 1), true)
        XCTAssertEqual(monster.mutate(inventory: 3, at: 2), true)
        XCTAssertEqual(monster.mutate(inventory: 4, at: 3), true)
        XCTAssertEqual(monster.mutate(inventory: 5, at: 4), true)

        for i in 0..<monster.inventoryCount {
            XCTAssertEqual(monster.inventory(at: i), Byte(i + 1))
        }

        XCTAssertEqual(monster.mutate(inventory: 0, at: 0), true)
        XCTAssertEqual(monster.mutate(inventory: 1, at: 1), true)
        XCTAssertEqual(monster.mutate(inventory: 2, at: 2), true)
        XCTAssertEqual(monster.mutate(inventory: 3, at: 3), true)
        XCTAssertEqual(monster.mutate(inventory: 4, at: 4), true)

        let vec = monster.pos
        XCTAssertEqual(vec?.x, 1)
        XCTAssertTrue(vec?.mutate(x: 55.0) ?? false)
        XCTAssertTrue(vec?.mutate(test1: 55) ?? false)
        XCTAssertEqual(vec?.x, 55.0)
        XCTAssertEqual(vec?.test1, 55.0)
        XCTAssertTrue(vec?.mutate(x: 1) ?? false)
        XCTAssertEqual(vec?.x, 1)
        XCTAssertTrue(vec?.mutate(test1: 3) ?? false)
    }

    func readMonster(fb: ByteBuffer) {
        let monster = Monster1.getRootAsMonster(bb: fb)
        XCTAssertEqual(monster.hp, 80)
        XCTAssertEqual(monster.mana, 150)
        XCTAssertEqual(monster.name, "MyMonster")
        let pos = monster.pos
        XCTAssertEqual(pos?.x, 1)
        XCTAssertEqual(pos?.y, 2)
        XCTAssertEqual(pos?.z, 3)
        XCTAssertEqual(pos?.test1, 3)
        XCTAssertEqual(pos?.test2, .green)
        let test = pos?.test3
        XCTAssertEqual(test?.a, 5)
        XCTAssertEqual(test?.b, 6)
        XCTAssertEqual(monster.testType, .monster)
        let monster2 = monster.test(type: Monster1.self)
        XCTAssertEqual(monster2?.name, "Fred")
        
        XCTAssertEqual(monster.mutate(mana: 10), false)
        
        XCTAssertEqual(monster.mana, 150)
        XCTAssertEqual(monster.inventoryCount, 5)
        var sum: Byte = 0
        for i in 0...monster.inventoryCount {
            sum += monster.inventory(at: i)
        }
        XCTAssertEqual(sum, 10)
        XCTAssertEqual(monster.test4Count, 2)
        let test0 = monster.test4(at: 0)
        let test1 = monster.test4(at: 1)
        var sum0 = 0
        var sum1 = 0
        if let a = test0?.a, let b = test0?.b {
            sum0 = Int(a) + Int(b)
        }
        if let a = test1?.a, let b = test1?.b {
            sum1 = Int(a) + Int(b)
        }
        XCTAssertEqual(sum0 + sum1, 100)
        XCTAssertEqual(monster.testarrayofstringCount, 2)
        XCTAssertEqual(monster.testarrayofstring(at: 0), "test1")
        XCTAssertEqual(monster.testarrayofstring(at: 1), "test2")
        XCTAssertEqual(monster.testbool, true)

        let array = monster.nameSegmentArray
        XCTAssertEqual(String(bytes: array ?? [], encoding: .utf8), "MyMonster")

        if 0 == monster.testarrayofboolsCount  {
            XCTAssertEqual(monster.testarrayofbools.isEmpty, true)
        } else {
            XCTAssertEqual(monster.testarrayofbools.isEmpty, false)
        }
    }
}
