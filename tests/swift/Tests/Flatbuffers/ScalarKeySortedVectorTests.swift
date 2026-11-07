import FlatBuffers
import Foundation
import Testing

@Suite
struct ScalarKeySortedVectorTests {

  func buildMonster(counts: [UInt16]) -> ByteBuffer {
    var fbb = FlatBufferBuilder()
    var offsets: [Offset] = []
    for (i, count) in counts.enumerated() {
      offsets.append(
        MyGame_Example_Stat.createStat(
          &fbb,
          idOffset: fbb.create(string: "s\(i)"),
          val: Int64(i),
          count: count))
    }
    let vector = fbb.createVector(ofOffsets: offsets)
    let name = fbb.create(string: "scalar_key_test")
    let start = Monster.startMonster(&fbb)
    Monster.add(name: name, &fbb)
    Monster.addVectorOf(scalarKeySortedTables: vector, &fbb)
    let root = Monster.endMonster(&fbb, start: start)
    Monster.finish(&fbb, end: root)
    return fbb.sizedBuffer
  }

  @Test func lookupByNonZeroScalarKey() throws {
    // Vector stored in ascending key order, all keys non-zero.
    var buf = buildMonster(counts: [1, 3, 5])
    let monster: Monster = getRoot(byteBuffer: &buf)
    #expect(monster.scalarKeySortedTables.count == 3)
    #expect(monster.scalarKeySortedTablesBy(key: 1)?.count == 1)
    #expect(monster.scalarKeySortedTablesBy(key: 3)?.count == 3)
    #expect(monster.scalarKeySortedTablesBy(key: 5)?.count == 5)
    #expect(monster.scalarKeySortedTablesBy(key: 4) == nil)
    #expect(monster.scalarKeySortedTablesBy(key: 0) == nil)
  }

  @Test func sortVectorOfScalarKeySortsAscending() throws {
    var fbb = FlatBufferBuilder()
    var offsets: [Offset] = []
    for (i, count) in [UInt16(5), 1, 3].enumerated() {
      offsets.append(
        MyGame_Example_Stat.createStat(
          &fbb,
          idOffset: fbb.create(string: "s\(i)"),
          val: Int64(i),
          count: count))
    }
    let sorted = MyGame_Example_Stat.sortVectorOfStat(offsets: offsets, &fbb)
    let name = fbb.create(string: "scalar_key_test")
    let start = Monster.startMonster(&fbb)
    Monster.add(name: name, &fbb)
    Monster.addVectorOf(scalarKeySortedTables: sorted, &fbb)
    let root = Monster.endMonster(&fbb, start: start)
    Monster.finish(&fbb, end: root)
    var buf = fbb.sizedBuffer
    let monster: Monster = getRoot(byteBuffer: &buf)
    let counts = (0..<monster.scalarKeySortedTables.count).map {
      monster.scalarKeySortedTables[$0].count
    }
    #expect(counts == [1, 3, 5])
    // A sorted vector must be searchable with the fixed binary search.
    #expect(monster.scalarKeySortedTablesBy(key: 3)?.count == 3)
  }

  @Test func sortVectorOfHighUInt64Keys() throws {
    // High-bit UInt64 keys. The old code fed scalar key slots into the
    // string-oriented Table.compare, which misreads them as string
    // uoffsets and sorts wrongly (and can trap, depending on buffer
    // layout).
    let ids: [UInt64] = [
      0xFFFF_FFFF_FFFF_FFF2,
      0xFFFF_FFFF_FFFF_FFF1,
      0xFFFF_FFFF_FFFF_FFF3,
    ]
    var fbb = FlatBufferBuilder()
    var offsets: [Offset] = []
    for id in ids {
      offsets.append(MyGame_Example_Referrable.createReferrable(&fbb, id: id))
    }
    let sorted = MyGame_Example_Referrable.sortVectorOfReferrable(
      offsets: offsets, &fbb)
    let name = fbb.create(string: "scalar_key_test")
    let start = Monster.startMonster(&fbb)
    Monster.add(name: name, &fbb)
    Monster.addVectorOf(vectorOfReferrables: sorted, &fbb)
    let root = Monster.endMonster(&fbb, start: start)
    Monster.finish(&fbb, end: root)
    var buf = fbb.sizedBuffer
    let monster: Monster = getRoot(byteBuffer: &buf)
    let read = (0..<monster.vectorOfReferrables.count).map {
      monster.vectorOfReferrables[$0].id
    }
    #expect(read == ids.sorted())
  }
}
