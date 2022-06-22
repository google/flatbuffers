import std/unittest
import flatbuffers
import MyGame/Example/Test_generated
import MyGame/Example/Monster_generated
import MyGame/Example/Vec3_generated
import MyGame/Example/Color_generated
import MyGame/Example/Any_generated

proc verifyMonster(monster: var Monster) =
  check(monster.hp == 80)
  check(monster.mana == 150)
  check(monster.name == "MyMonster")
  check(monster.pos.x == 1)
  check(monster.pos.y == 2)
  check(monster.pos.z == 3)
  check(monster.pos.test1 == 3)
  check(monster.pos.test2 == Color.Green.uint8)
  check(monster.pos.test3.a == 5)
  check(monster.pos.test3.b == 6)
  check(monster.testType == Any.Monster.uint8)
  let monster2 = Monster(tab: monster.test.tab)
  check(monster2.name == "Fred")
  check((monster.mana = 10) == false)
  check(monster.mana == 150)
  check(monster.inventoryLength == 5)
  var sum: uint8 = 0
  for i in countup(0, monster.inventoryLength, 1):
    sum += monster.inventory(i)
  check(sum == 10)
  check(monster.test4Length == 2)

  let test0 = monster.test4(0)
  let test1 = monster.test4(1)
  var sum0 = test0.a + test0.b
  var sum1 = test1.a + test1.b
  check(sum0 + sum1 == 100)

  check(monster.testarrayofstringLength == 2)
  check(monster.testarrayofstring(0) == "test1")
  check(monster.testarrayofstring(1) == "test2")
  check(monster.testbool == true)


suite "TestMyGame":

  test "testData":
    let data: seq[byte] = @[byte(48), 0, 0, 0, 77, 79, 78, 83, 0, 0, 0, 0, 36, 0, 72, 0, 40, 0, 0, 0, 38, 0, 32, 0, 0, 0, 28, 0, 0, 0, 27, 0, 20, 0, 16, 0, 12, 0, 4, 0, 0, 0, 0, 0, 0, 0, 11, 0, 36, 0, 0, 0, 164, 0, 0, 0, 0, 0, 0, 1, 60, 0, 0, 0, 68, 0, 0, 0, 76, 0, 0, 0, 0, 0, 0, 1, 88, 0, 0, 0, 120, 0, 0, 0, 0, 0, 80, 0, 0, 0, 128, 63, 0, 0, 0, 64, 0, 0, 64, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 64, 2, 0, 5, 0, 6, 0, 0, 0, 2, 0, 0, 0, 64, 0, 0, 0, 48, 0, 0, 0, 2, 0, 0, 0, 30, 0, 40, 0, 10, 0, 20, 0, 152, 255, 255, 255, 4, 0, 0, 0, 4, 0, 0, 0, 70, 114, 101, 100, 0, 0, 0, 0, 5, 0, 0, 0, 0, 1, 2, 3, 4, 0, 0, 0, 5, 0, 0, 0, 116, 101, 115, 116, 50, 0, 0, 0, 5, 0, 0, 0, 116, 101, 115, 116, 49, 0, 0, 0, 9, 0, 0, 0, 77, 121, 77, 111, 110, 115, 116, 101, 114, 0, 0, 0, 3, 0, 0, 0, 20, 0, 0, 0, 36, 0, 0, 0, 4, 0, 0, 0, 240, 255, 255, 255, 32, 0, 0, 0, 248, 255, 255, 255, 36, 0, 0, 0, 12, 0, 8, 0, 0, 0, 0, 0, 0, 0, 4, 0, 12, 0, 0, 0, 28, 0, 0, 0, 5, 0, 0, 0, 87, 105, 108, 109, 97, 0, 0, 0, 6, 0, 0, 0, 66, 97, 114, 110, 101, 121, 0, 0, 5, 0, 0, 0, 70, 114, 111, 100, 111, 0, 0, 0]
    var monster: Monster
    GetRootAs(monster, data, 0)
    verifyMonster(monster)

  test "testCreateString":
    var fbb = newBuilder(0)
    let name = fbb.Create("Frodo")
    fbb.Finish(name)
    check(fbb.FinishedBytes() == @[byte(4), 0, 0, 0, 5, 0, 0, 0, 70, 114, 111, 100, 111, 0, 0, 0])

  test "testCreateVector":
    var fbb = newBuilder(0)
    let vec = fbb.Create(@[byte(0), 1, 2, 3, 4])
    fbb.Finish(vec)
    check(fbb.FinishedBytes() == @[byte(4), 0, 0, 0, 5, 0, 0, 0, 0, 1, 2, 3, 4, 0, 0, 0])

  test "createSimpleMonster":
    var fbb = newBuilder(0)
    let names = [
      fbb.Create("Frodo"),
      fbb.Create("Barney"),
      fbb.Create("Wilma"),
    ]
    fbb.MonsterStart()
    fbb.MonsterAddName(names[0])
    let monster = fbb.MonsterEnd()
    fbb.Finish(monster)
    check(fbb.FinishedBytes() == @[byte(16), 0, 0, 0, 12, 0, 8, 0, 0, 0, 0, 0, 0, 0, 4, 0, 12, 0, 0, 0, 28, 0, 0, 0, 5, 0, 0, 0, 87, 105, 108, 109, 97, 0, 0, 0, 6, 0, 0, 0, 66, 97, 114, 110, 101, 121, 0, 0, 5, 0, 0, 0, 70, 114, 111, 100, 111, 0, 0, 0])

  test "testCreateTestVector":
    var fbb = newBuilder(0)
    discard fbb.MonsterStartTest4Vector(2)
    discard fbb.CreateTest(a=30, b=40)
    discard fbb.CreateTest(a=10, b=20)
    let test4 = fbb.EndVector(2)
    fbb.Finish(test4)
    check(fbb.FinishedBytes() == @[byte(4), 0, 0, 0, 2, 0, 0, 0, 10, 0, 20, 0, 30, 0, 40, 0])

  test "testTableWithStruct":
    var fbb = newBuilder(0)
    fbb.MonsterStart()
    fbb.MonsterAddPos(fbb.CreateVec3(x= 1,
        y= 2,
        z= 3,
        test1= 3,
        test2= Color.Green.uint8,
        test3_a= 5, test3_b= 6))

    let monster_end = fbb.MonsterEnd()
    fbb.Finish(monster_end)
    check(fbb.FinishedBytes() == @[byte(12), 0, 0, 0, 0, 0, 6, 0, 36, 0, 4, 0, 6, 0, 0, 0, 0, 0, 128, 63, 0, 0, 0, 64, 0, 0, 64, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 64, 2, 0, 5, 0, 6, 0, 0, 0])

  test "testCreateMonster":
    var fbb = newBuilder(0)
    let names = [
      fbb.Create("Frodo"),
      fbb.Create("Barney"),
      fbb.Create("Wilma"),
    ]

    var offsets: seq[uoffset] = @[]
    fbb.MonsterStart()
    fbb.MonsterAddName(names[0])
    offsets.add(fbb.MonsterEnd())
    fbb.MonsterStart()
    fbb.MonsterAddName(names[1])
    offsets.add(fbb.MonsterEnd())
    fbb.MonsterStart()
    fbb.MonsterAddName(names[2])
    offsets.add(fbb.MonsterEnd())

    let str = fbb.Create("MyMonster")
    let test1 = fbb.Create("test1")
    let test2 = fbb.Create("test2")
    let inv = fbb.Create(@[byte(0), 1, 2, 3, 4])
    let fred = fbb.Create("Fred")
    fbb.MonsterStart()
    fbb.MonsterAddName(fred)
    let mon2 = fbb.MonsterEnd()

    discard fbb.MonsterStartTest4Vector(2)
    discard fbb.CreateTest(a=30, b=40)
    discard fbb.CreateTest(a=10, b=20)
    let test4 = fbb.EndVector(2)

    discard fbb.MonsterStartTestarrayofstringVector(2)
    fbb.PrependOffsetRelative(test1)
    fbb.PrependOffsetRelative(test2)
    let stringTestVector = fbb.EndVector(2)

    discard fbb.MonsterStartTestarrayoftablesVector(3)
    fbb.PrependOffsetRelative(offsets[0])
    fbb.PrependOffsetRelative(offsets[1])
    fbb.PrependOffsetRelative(offsets[2])
    let tableTestVector = fbb.EndVector(3)

    fbb.MonsterStart()
    fbb.MonsterAddPos(fbb.CreateVec3(x= 1,
        y= 2,
        z= 3,
        test1= 3,
        test2= Color.Green.uint8,
        test3_a= 5, test3_b= 6))
    fbb.MonsterAddHp(80)
    fbb.MonsterAddName(str)
    fbb.MonsterAddInventory(inv)
    fbb.MonsterAddTestType(ord(Any.Monster))
    fbb.MonsterAddTest(mon2)
    fbb.MonsterAddTest4(test4)
    fbb.MonsterAddTestarrayofstring(stringTestVector)
    fbb.MonsterAddTestbool(true)
    fbb.MonsterAddTestarrayoftables(tableTestVector)
    let monster_end = fbb.MonsterEnd()
    fbb.Finish(monster_end)
    check(fbb.FinishedBytes() == @[byte(40), 0, 0, 0, 36, 0, 72, 0, 40, 0, 0, 0, 38, 0, 32, 0, 0, 0, 28, 0, 0, 0, 27, 0, 20, 0, 16, 0, 12, 0, 4, 0, 0, 0, 0, 0, 0, 0, 11, 0, 36, 0, 0, 0, 68, 0, 0, 0, 0, 0, 0, 1, 76, 0, 0, 0, 84, 0, 0, 0, 92, 0, 0, 0, 0, 0, 0, 1, 104, 0, 0, 0, 136, 0, 0, 0, 0, 0, 80, 0, 0, 0, 128, 63, 0, 0, 0, 64, 0, 0, 64, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 64, 2, 0, 5, 0, 6, 0, 0, 0, 3, 0, 0, 0, 108, 0, 0, 0, 112, 0, 0, 0, 128, 0, 0, 0, 2, 0, 0, 0, 52, 0, 0, 0, 60, 0, 0, 0, 2, 0, 0, 0, 10, 0, 20, 0, 30, 0, 40, 0, 168, 255, 255, 255, 4, 0, 0, 0, 4, 0, 0, 0, 70, 114, 101, 100, 0, 0, 0, 0, 5, 0, 0, 0, 0, 1, 2, 3, 4, 0, 0, 0, 5, 0, 0, 0, 116, 101, 115, 116, 50, 0, 0, 0, 5, 0, 0, 0, 116, 101, 115, 116, 49, 0, 0, 0, 9, 0, 0, 0, 77, 121, 77, 111, 110, 115, 116, 101, 114, 0, 0, 0, 240, 255, 255, 255, 32, 0, 0, 0, 248, 255, 255, 255, 36, 0, 0, 0, 12, 0, 8, 0, 0, 0, 0, 0, 0, 0, 4, 0, 12, 0, 0, 0, 28, 0, 0, 0, 5, 0, 0, 0, 87, 105, 108, 109, 97, 0, 0, 0, 6, 0, 0, 0, 66, 97, 114, 110, 101, 121, 0, 0, 5, 0, 0, 0, 70, 114, 111, 100, 111, 0, 0, 0])


    # var test_mutating_bool: TestMutatingBool
    # GetRootAs(test_mutating_bool, builder.FinishedBytes(), 0)
    # XCTAssertEqual(bytes.sizedByteArray, )
    # let monster = MyGame_Example_Monster.getRootAsMonster(bb: bytes.buffer)
    # readMonster(monster: monster)
    # mutateMonster(fb: bytes.buffer)
    # readMonster(monster: monster)


  # test "testReadFromOtherLanguages":
  #   let path = FileManager.default.currentDirectoryPath
  #   let url = URL(fileURLWithPath: path, isDirectory: true)
  #     .appendingPathComponent("monsterdata_test").appendingPathExtension("mon")
  #   guard let data = try? Data(contentsOf: url) else { return }
  #   let _data = ByteBuffer(data: data)
  #   readVerifiedMonster(fb: _data)
  # }

  # test "testCreateMonster":
  #   let bytes = createMonster(withPrefix: false)
  #   // swiftformat:disable all
  #   check(bytes.sizedByteArray == [48, 0, 0, 0, 77, 79, 78, 83, 0, 0, 0, 0, 36, 0, 72, 0, 40, 0, 0, 0, 38, 0, 32, 0, 0, 0, 28, 0, 0, 0, 27, 0, 20, 0, 16, 0, 12, 0, 4, 0, 0, 0, 0, 0, 0, 0, 11, 0, 36, 0, 0, 0, 164, 0, 0, 0, 0, 0, 0, 1, 60, 0, 0, 0, 68, 0, 0, 0, 76, 0, 0, 0, 0, 0, 0, 1, 88, 0, 0, 0, 120, 0, 0, 0, 0, 0, 80, 0, 0, 0, 128, 63, 0, 0, 0, 64, 0, 0, 64, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 64, 2, 0, 5, 0, 6, 0, 0, 0, 2, 0, 0, 0, 64, 0, 0, 0, 48, 0, 0, 0, 2, 0, 0, 0, 30, 0, 40, 0, 10, 0, 20, 0, 152, 255, 255, 255, 4, 0, 0, 0, 4, 0, 0, 0, 70, 114, 101, 100, 0, 0, 0, 0, 5, 0, 0, 0, 0, 1, 2, 3, 4, 0, 0, 0, 5, 0, 0, 0, 116, 101, 115, 116, 50, 0, 0, 0, 5, 0, 0, 0, 116, 101, 115, 116, 49, 0, 0, 0, 9, 0, 0, 0, 77, 121, 77, 111, 110, 115, 116, 101, 114, 0, 0, 0, 3, 0, 0, 0, 20, 0, 0, 0, 36, 0, 0, 0, 4, 0, 0, 0, 240, 255, 255, 255, 32, 0, 0, 0, 248, 255, 255, 255, 36, 0, 0, 0, 12, 0, 8, 0, 0, 0, 0, 0, 0, 0, 4, 0, 12, 0, 0, 0, 28, 0, 0, 0, 5, 0, 0, 0, 87, 105, 108, 109, 97, 0, 0, 0, 6, 0, 0, 0, 66, 97, 114, 110, 101, 121, 0, 0, 5, 0, 0, 0, 70, 114, 111, 100, 111, 0, 0, 0])
  #   // swiftformat:enable all
  #   let monster = Monster.getRootAsMonster(bb: bytes.buffer)
  #   readMonster(monster: monster)
  #   mutateMonster(fb: bytes.buffer)
  #   readMonster(monster: monster)
  # }

  # test "testCreateMonsterResizedBuffer":
  #   let bytes = createMonster(withPrefix: false)
  #   // swiftformat:disable all
  #   check(bytes.sizedByteArray == [48, 0, 0, 0, 77, 79, 78, 83, 0, 0, 0, 0, 36, 0, 72, 0, 40, 0, 0, 0, 38, 0, 32, 0, 0, 0, 28, 0, 0, 0, 27, 0, 20, 0, 16, 0, 12, 0, 4, 0, 0, 0, 0, 0, 0, 0, 11, 0, 36, 0, 0, 0, 164, 0, 0, 0, 0, 0, 0, 1, 60, 0, 0, 0, 68, 0, 0, 0, 76, 0, 0, 0, 0, 0, 0, 1, 88, 0, 0, 0, 120, 0, 0, 0, 0, 0, 80, 0, 0, 0, 128, 63, 0, 0, 0, 64, 0, 0, 64, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 64, 2, 0, 5, 0, 6, 0, 0, 0, 2, 0, 0, 0, 64, 0, 0, 0, 48, 0, 0, 0, 2, 0, 0, 0, 30, 0, 40, 0, 10, 0, 20, 0, 152, 255, 255, 255, 4, 0, 0, 0, 4, 0, 0, 0, 70, 114, 101, 100, 0, 0, 0, 0, 5, 0, 0, 0, 0, 1, 2, 3, 4, 0, 0, 0, 5, 0, 0, 0, 116, 101, 115, 116, 50, 0, 0, 0, 5, 0, 0, 0, 116, 101, 115, 116, 49, 0, 0, 0, 9, 0, 0, 0, 77, 121, 77, 111, 110, 115, 116, 101, 114, 0, 0, 0, 3, 0, 0, 0, 20, 0, 0, 0, 36, 0, 0, 0, 4, 0, 0, 0, 240, 255, 255, 255, 32, 0, 0, 0, 248, 255, 255, 255, 36, 0, 0, 0, 12, 0, 8, 0, 0, 0, 0, 0, 0, 0, 4, 0, 12, 0, 0, 0, 28, 0, 0, 0, 5, 0, 0, 0, 87, 105, 108, 109, 97, 0, 0, 0, 6, 0, 0, 0, 66, 97, 114, 110, 101, 121, 0, 0, 5, 0, 0, 0, 70, 114, 111, 100, 111, 0, 0, 0])
  #   // swiftformat:enable all
  #   readVerifiedMonster(fb: bytes.sizedBuffer)
  # }

  # test "testCreateMonsterPrefixed":
  #   let bytes = createMonster(withPrefix: true)
  #   // swiftformat:disable all
  #   check(bytes.sizedByteArray, [44, 1, 0, 0, 44, 0, 0, 0, 77, 79, 78, 83, 36, 0, 72, 0, 40, 0, 0, 0, 38, 0, 32, 0, 0, 0, 28, 0, 0, 0, 27, 0, 20, 0, 16, 0, 12, 0, 4, 0, 0, 0, 0, 0, 0, 0, 11, 0, 36, 0, 0, 0, 164, 0, 0, 0, 0, 0, 0, 1, 60, 0, 0, 0, 68, 0, 0, 0, 76, 0, 0, 0, 0, 0, 0, 1, 88, 0, 0, 0, 120, 0, 0, 0, 0, 0, 80, 0, 0, 0, 128, 63, 0, 0, 0, 64, 0, 0, 64, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 64, 2, 0, 5, 0, 6, 0, 0, 0, 2, 0, 0, 0, 64, 0, 0, 0, 48, 0, 0, 0, 2, 0, 0, 0, 30, 0, 40, 0, 10, 0, 20, 0, 152, 255, 255, 255, 4, 0, 0, 0, 4, 0, 0, 0, 70, 114, 101, 100, 0, 0, 0, 0, 5, 0, 0, 0, 0, 1, 2, 3, 4, 0, 0, 0, 5, 0, 0, 0, 116, 101, 115, 116, 50, 0, 0, 0, 5, 0, 0, 0, 116, 101, 115, 116, 49, 0, 0, 0, 9, 0, 0, 0, 77, 121, 77, 111, 110, 115, 116, 101, 114, 0, 0, 0, 3, 0, 0, 0, 20, 0, 0, 0, 36, 0, 0, 0, 4, 0, 0, 0, 240, 255, 255, 255, 32, 0, 0, 0, 248, 255, 255, 255, 36, 0, 0, 0, 12, 0, 8, 0, 0, 0, 0, 0, 0, 0, 4, 0, 12, 0, 0, 0, 28, 0, 0, 0, 5, 0, 0, 0, 87, 105, 108, 109, 97, 0, 0, 0, 6, 0, 0, 0, 66, 97, 114, 110, 101, 121, 0, 0, 5, 0, 0, 0, 70, 114, 111, 100, 111, 0, 0, ==0])
  #   // swiftformat:enable all

  #   var buffer = bytes.buffer
  #   readMonster(monster: getPrefixedSizeRoot(byteBuffer: &buffer))
  # }

  # test "testCreateMonsterUsingCreateMonsterMethodWithNilPos":
  #   var fbb = FlatBufferBuilder(initialSize: 1)
  #   let name = fbb.Create("Frodo")
  #   let mStart = fbb.MonsterStart()
  #   Monster.add(name: name, &fbb)
  #   let root = Monster.endMonster(&fbb, start: mStart)
  #   fbb.finish(offset: root)
  #   let newMonster = Monster.getRootAsMonster(bb: fbb.sizedBuffer)
  #   XCTAssertNil(newMonster.pos)
  #   check(newMonster.name, "Fro ==o")
  # }

  # test "testCreateMonsterUsingCreateMonsterMethodWithPosX":
  #   var fbb = FlatBufferBuilder(initialSize: 1)
  #   let name = fbb.Create("Barney")
  #   let mStart = fbb.MonsterStart()
  #   Monster.add(
  #     pos: Vec3(
  #       x: 10,
  #       y: 0,
  #       z: 0,
  #       test1: 0,
  #       test2: .blue,
  #       test3: .init()),
  #     &fbb)
  #   Monster.add(name: name, &fbb)
  #   let root = Monster.endMonster(&fbb, start: mStart)
  #   fbb.finish(offset: root)

  #   let newMonster = Monster.getRootAsMonster(bb: fbb.sizedBuffer)
  #   check(newMonster.pos!.x == 10)
  #   check(newMonster.name, "Barn ==y")
  # }

  # test "testReadMonsterFromUnsafePointerWithoutCopying":
  #   // swiftformat:disable all
  #   var array: [UInt8] = [48, 0, 0, 0, 77, 79, 78, 83, 0, 0, 0, 0, 36, 0, 72, 0, 40, 0, 0, 0, 38, 0, 32, 0, 0, 0, 28, 0, 0, 0, 27, 0, 20, 0, 16, 0, 12, 0, 4, 0, 0, 0, 0, 0, 0, 0, 11, 0, 36, 0, 0, 0, 164, 0, 0, 0, 0, 0, 0, 1, 60, 0, 0, 0, 68, 0, 0, 0, 76, 0, 0, 0, 0, 0, 0, 1, 88, 0, 0, 0, 120, 0, 0, 0, 0, 0, 80, 0, 0, 0, 128, 63, 0, 0, 0, 64, 0, 0, 64, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 64, 2, 0, 5, 0, 6, 0, 0, 0, 2, 0, 0, 0, 64, 0, 0, 0, 48, 0, 0, 0, 2, 0, 0, 0, 30, 0, 40, 0, 10, 0, 20, 0, 152, 255, 255, 255, 4, 0, 0, 0, 4, 0, 0, 0, 70, 114, 101, 100, 0, 0, 0, 0, 5, 0, 0, 0, 0, 1, 2, 3, 4, 0, 0, 0, 5, 0, 0, 0, 116, 101, 115, 116, 50, 0, 0, 0, 5, 0, 0, 0, 116, 101, 115, 116, 49, 0, 0, 0, 9, 0, 0, 0, 77, 121, 77, 111, 110, 115, 116, 101, 114, 0, 0, 0, 3, 0, 0, 0, 20, 0, 0, 0, 36, 0, 0, 0, 4, 0, 0, 0, 240, 255, 255, 255, 32, 0, 0, 0, 248, 255, 255, 255, 36, 0, 0, 0, 12, 0, 8, 0, 0, 0, 0, 0, 0, 0, 4, 0, 12, 0, 0, 0, 28, 0, 0, 0, 5, 0, 0, 0, 87, 105, 108, 109, 97, 0, 0, 0, 6, 0, 0, 0, 66, 97, 114, 110, 101, 121, 0, 0, 5, 0, 0, 0, 70, 114, 111, 100, 111, 0, 0, 0]
  #   // swiftformat:enable all
  #   let unpacked = array
  #     .withUnsafeMutableBytes { (memory) -> MonsterT in
  #       let bytes = ByteBuffer(
  #         assumingMemoryBound: memory.baseAddress!,
  #         capacity: memory.count)
  #       var monster = Monster.getRootAsMonster(bb: bytes)
  #       readFlatbufferMonster(monster: &monster)
  #       let unpacked = monster.unpack()
  #       return unpacked
  #     }
  #   readObjectApi(monster: unpacked)
  # }

  # test "testArrayOfBools":
  #   let boolArray = [false, true, false, true, false, true, false]
  #   var fbb = FlatBufferBuilder(initialSize: 1)
  #   let name = fbb.Create("Frodo")
  #   let bools = fbb.createVector(boolArray)
  #   let root = Monster.createMonster(
  #     &fbb,
  #     nameOffset: name,
  #     testarrayofboolsVectorOffset: bools)
  #   fbb.finish(offset: root)
  #   let monster = Monster.getRootAsMonster(bb: fbb.sizedBuffer)

  #   let values = monster.testarrayofbools

  #   check(boolArray == values)

  #   for i in 0..<monster.testarrayofboolsCount {
  #     check(boolArray[Int(i)], monster.testarrayofbools(==i))
  #   }
  # }

  # test "readVerifiedMonster":
  #   var byteBuffer = fb
  #   XCTAssertNoThrow(
  #     try readMonster(
  #       monster: getCheckedRoot(
  #         byteBuffer: &byteBuffer) as Monster))
  # }

  # # test "readMonster(monster: Monste":
  # #   var monster = monster
  # #   readFlatbufferMonster(monster: &monster)
  # #   let unpacked: MonsterT? = monster.unpack()
  # #   readObjectApi(monster: unpacked!)
  # #   guard let buffer = unpacked.serialize()
  # #   else { fatalError("Couldnt generate bytebuffer") }
  # #   var newMonster = Monster.getRootAsMonster(bb: buffer)
  # #   readFlatbufferMonster(monster: &newMonster)
  # # }

  # test "createMonster(withPrefix prefix: Bool) -> FlatBufferBuild":
  #   var fbb = FlatBufferBuilder(initialSize: 1)
  #   let names = [
  #     fbb.Create("Frodo"),
  #     fbb.Create("Barney"),
  #     fbb.Create("Wilma"),
  #   ]
  #   var offsets: [Offset] = []
  #   let start1 = fbb.MonsterStart()
  #   Monster.add(name: names[0], &fbb)
  #   offsets.append(Monster.endMonster(&fbb, start: start1))
  #   let start2 = fbb.MonsterStart()
  #   Monster.add(name: names[1], &fbb)
  #   offsets.append(Monster.endMonster(&fbb, start: start2))
  #   let start3 = fbb.MonsterStart()
  #   Monster.add(name: names[2], &fbb)
  #   offsets.append(Monster.endMonster(&fbb, start: start3))

  #   let sortedArray = Monster.sortVectorOfMonster(offsets: offsets, &fbb)

  #   let str = fbb.Create("MyMonster")
  #   let test1 = fbb.Create("test1")
  #   let test2 = fbb.Create("test2")
  #   let _inv: [Byte] = [0, 1, 2, 3, 4]
  #   let inv = fbb.createVector(_inv)

  #   let fred = fbb.Create("Fred")
  #   let mon1Start = fbb.MonsterStart()
  #   Monster.add(name: fred, &fbb)
  #   let mon2 = Monster.endMonster(&fbb, start: mon1Start)

  #   let test4 = fbb.createVector(ofStructs: [
  #     Test(a: 30, b: 40),
  #     Test(a: 10, b: 20),
  #   ])

  #   let stringTestVector = fbb.createVector(ofOffsets: [test1, test2])
  #   let mStart = fbb.MonsterStart()
  #   Monster.add(
  #     pos: Vec3(
  #       x: 1,
  #       y: 2,
  #       z: 3,
  #       test1: 3,
  #       test2:  Color.Green,
  #       test3: .init(a: 5, b: 6)),
  #     &fbb)
  #   Monster.add(hp: 80, &fbb)
  #   Monster.add(name: str, &fbb)
  #   Monster.addVectorOf(inventory: inv, &fbb)
  #   Monster.add(testType: .monster, &fbb)
  #   Monster.add(test: mon2, &fbb)
  #   Monster.addVectorOf(test4: test4, &fbb)
  #   Monster.addVectorOf(testarrayofstring: stringTestVector, &fbb)
  #   Monster.add(testbool: true, &fbb)
  #   Monster.addVectorOf(testarrayoftables: sortedArray, &fbb)
  #   let end = Monster.endMonster(&fbb, start: mStart)
  #   Monster.finish(&fbb, end: end, prefix: prefix)
  #   return fbb
  # }

  # test "mutateMonster(fb: ByteBuffe":
  #   let monster = Monster.getRootAsMonster(bb: fb)
  #   XCTAssertFalse(monster.mana = 10)
  #   check(monster.testarrayoftables(0).name, "Barn ==y")
  #   check(monster.testarrayoftables(1).name, "Fro ==o")
  #   check(monster.testarrayoftables(2).name, "Wil ==a")

  #   // Example of searching for a table by the key
  #   XCTAssertNotNil(monster.testarrayoftablesBy(key: "Frodo"))
  #   XCTAssertNotNil(monster.testarrayoftablesBy(key: "Barney"))
  #   XCTAssertNotNil(monster.testarrayoftablesBy(key: "Wilma"))

  #   check(monster.testType, ==.monster)

  #   check(monster.inventory = 1 0) == true)
  #   check(monster.inventory = 2 1) == true)
  #   check(monster.inventory = 3 2) == true)
  #   check(monster.inventory = 4 3) == true)
  #   check(monster.inventory = 5 4) == true)

  #   for i in 0..<monster.inventoryCount {
  #     check(monster.inventory(i), Byte(i + ==1))
  #   }

  #   check(monster.inventory = 0 0) == true)
  #   check(monster.inventory = 1 1) == true)
  #   check(monster.inventory = 2 2) == true)
  #   check(monster.inventory = 3 3) == true)
  #   check(monster.inventory = 4 4) == true)

  #   let vec = monster.mutablePos
  #   check(vec.x == 1)
  #   XCTAssertTrue(vec.x = 550) ?? false)
  #   XCTAssertTrue(vec.test1 = 55 ?? false)
  #   check(vec.x, 5 ==.0)
  #   check(vec.test1, 5 ==.0)
  #   XCTAssertTrue(vec.x = 1 ?? false)
  #   check(vec.x == 1)
  #   XCTAssertTrue(vec.test1 = 3 ?? false)
  # }

  # test "readFlatbufferMonster(monster: inout Monste":
  # }

  # test "readObjectApi(monster: Monster":
  #   check(monster.hp == 80)
  #   check(monster.mana == 150)
  #   check(monster.name, "MyMonst ==r")
  #   let pos = monster.pos
  #   check(pos.x == 1)
  #   check(pos.y == 2)
  #   check(pos.z == 3)
  #   check(pos.test1 == 3)
  #   check(pos.test2, == Color.Green)
  #   let test = pos.test3
  #   check(test.a == 5)
  #   check(test.b == 6)
  #   let monster2 = monster.test.value as? MonsterT
  #   check(monster2.name, "Fr ==d")
  #   check(monster.mana == 150)
  #   monster.mana = 10
  #   check(monster.mana == 10)
  #   monster.mana = 150
  #   check(monster.mana == 150)

  #   check(monster.inventory.count == 5)
  #   var sum: Byte = 0
  #   for i in monster.inventory {
  #     sum += i
  #   }
  #   check(sum == 10)
  #   check(monster.test4.count == 2)
  #   let test0 = monster.test4[0]
  #   let test1 = monster.test4[1]
  #   var sum0 = 0
  #   var sum1 = 0
  #   if let a = test0.a, let b = test0.b {
  #     sum0 = Int(a) + Int(b)
  #   }
  #   if let a = test1.a, let b = test1.b {
  #     sum1 = Int(a) + Int(b)
  #   }
  #   check(sum0 + sum1 == 100)
  #   check(monster.testbool == true)
  # }

  # test "testEncoding":
  #   let fbb = createMonster(withPrefix: false)
  #   var sizedBuffer = fbb.sizedBuffer
  #   do {
  #     let reader: Monster = try getCheckedRoot(byteBuffer: &sizedBuffer)
  #     let encoder = JSONEncoder()
  #     encoder.keyEncodingStrategy = .convertToSnakeCase
  #     let data = try encoder.encode(reader)
  #     check(data, jsonData.data(using: .ut ==8))
  #   } catch {
  #     XCTFail(error.localizedDescription)
  #   }
  # }

  # var jsonData: String {
  #   """
  #   {\"hp\":80,\"inventory\":[0,1,2,3,4],\"test\":{\"name\":\"Fred\"},\"testarrayofstring\":[\"test1\",\"test2\"],\"testarrayoftables\":[{\"name\":\"Barney\"},{\"name\":\"Frodo\"},{\"name\":\"Wilma\"}],\"test4\":[{\"a\":30,\"b\":40},{\"a\":10,\"b\":20}],\"testbool\":true,\"test_type\":\"Monster\",\"pos\":{\"y\":2,\"test3\":{\"a\":5,\"b\":6},\"z\":3,\"x\":1,\"test1\":3,\"test2\":\"Green\"},\"name\":\"MyMonster\"}
  #   """
  # }
  # test "MutatingBool":
  #   var builder = newBuilder(1024)
  #   builder.TestMutatingBoolStart()
  #   builder.TestMutatingBoolAddB(builder.CreateProperty(false))
  #   let root = builder.TestMutatingBoolEnd()
  #   builder.Finish(root)

  #   var test_mutating_bool: TestMutatingBool
  #   GetRootAs(test_mutating_bool, builder.FinishedBytes(), 0)
  #   var prop2 = test_mutating_bool.b
  #   check(prop2.property == false)
  #   prop2.property = false
  #   check(prop2.property == false)
  #   prop2.property = true
  #   check(prop2.property == true)
