@file:OptIn(ExperimentalUnsignedTypes::class)

package com.google.flatbuffers.kotlin.benchmark

import Attacker
import BookReader
import MyGame.Example.AnyE
import MyGame.Example.Color
import MyGame.Example.Monster
import MyGame.Example.Vec3
import com.google.flatbuffers.kotlin.FlatBufferBuilder
import com.google.flatbuffers.kotlin.Offset
import org.openjdk.jmh.annotations.*
import java.util.concurrent.TimeUnit

@State(Scope.Benchmark)
@BenchmarkMode(Mode.AverageTime)
@OutputTimeUnit(TimeUnit.NANOSECONDS)
@Measurement(iterations = 20, time = 1, timeUnit = TimeUnit.NANOSECONDS)
open class FlatbufferBenchmark {

  val fbb = FlatBufferBuilder(1024 * 100000)
  val repetition = 10000

  @Benchmark
  fun monstersBuilder() {
    fbb.clear()
    val names = arrayOf(fbb.createString("Frodo"), fbb.createString("Barney"), fbb.createString("Wilma"))
    val off = arrayOf(
      Monster.createMonster(fbb, names[0]), Monster.createMonster(fbb, names[1]),
      Monster.createMonster(fbb, names[2])
    )
    val sortMons = fbb.createSortedVectorOfTables(Monster(), off)
    val str = fbb.createString("MyMonster")

    val inv = Monster.createInventoryVector(fbb, byteArrayOf(0, 1, 2, 3, 4).asUByteArray())

    val mon2 = Monster.createMonster(fbb, fbb.createString("Fred"))

    Monster.startTest4Vector(fbb, 2)
    MyGame.Example.Test.createTest(fbb, 10.toShort(), 20.toByte())
    MyGame.Example.Test.createTest(fbb, 30.toShort(), 40.toByte())
    val test4 = fbb.endVector<MyGame.Example.Test>()

    val testArrayOfString =
      Monster.createTestarrayofstringVector(fbb, arrayOf(fbb.createString("test1"), fbb.createString("test2")))

    val allMonsters = Array(repetition) {
      Monster.createMonster(fbb, str) {
        pos = Vec3.createVec3(
          fbb, 1.0f, 2.0f, 3.0f, 3.0,
          Color.Green, 5.toShort(), 6.toByte()
        )
        hp = 80
        mana = 150
        inventory = inv
        testType = AnyE.Monster
        test = mon2.toUnion()
        this.test4 = test4
        testarrayofstring = testArrayOfString
        testbool = true
        testhashu32Fnv1 = (Int.MAX_VALUE + 1L).toUInt()
        testarrayoftables = sortMons
      }
    }
    fbb.createVectorOfTables(allMonsters)
  }

  @Benchmark
  fun monstersManual() {
    fbb.clear()
    val names = arrayOf(fbb.createString("Frodo"), fbb.createString("Barney"), fbb.createString("Wilma"))
    val off = Array<Offset<Monster>>(3) { Offset(0) }
    Monster.startMonster(fbb)
    Monster.addName(fbb, names[0])
    off[0] = Monster.endMonster(fbb)
    Monster.startMonster(fbb)
    Monster.addName(fbb, names[1])
    off[1] = Monster.endMonster(fbb)
    Monster.startMonster(fbb)
    Monster.addName(fbb, names[2])
    off[2] = Monster.endMonster(fbb)
    val sortMons = fbb.createSortedVectorOfTables(Monster(), off)

    // We set up the same values as monsterdata.json:

    val str = fbb.createString("MyMonster")

    val inv = Monster.createInventoryVector(fbb, byteArrayOf(0, 1, 2, 3, 4).asUByteArray())

    val fred = fbb.createString("Fred")
    Monster.startMonster(fbb)
    Monster.addName(fbb, fred)
    val mon2 = Monster.endMonster(fbb)

    Monster.startTest4Vector(fbb, 2)
    MyGame.Example.Test.createTest(fbb, 10.toShort(), 20.toByte())
    MyGame.Example.Test.createTest(fbb, 30.toShort(), 40.toByte())
    val test4 = fbb.endVector<MyGame.Example.Test>()

    val testArrayOfString =
      Monster.createTestarrayofstringVector(fbb, arrayOf(fbb.createString("test1"), fbb.createString("test2")))

    val allMonsters = Array(repetition) {
      Monster.startMonster(fbb)
      Monster.addPos(
        fbb, Vec3.createVec3(
          fbb, 1.0f, 2.0f, 3.0f, 3.0,
          Color.Green, 5.toShort(), 6.toByte()
        )
      )
      Monster.addHp(fbb, 80.toShort())
      Monster.addName(fbb, str)
      Monster.addMana(fbb, 150)
      Monster.addInventory(fbb, inv)
      Monster.addTestType(fbb, AnyE.Monster)
      Monster.addTest(fbb, mon2.toUnion())
      Monster.addTest4(fbb, test4)
      Monster.addTestarrayofstring(fbb, testArrayOfString)
      Monster.addTestbool(fbb, true)
      Monster.addTesthashu32Fnv1(fbb, (Int.MAX_VALUE + 1L).toUInt())
      Monster.addTestarrayoftables(fbb, sortMons)
      Monster.endMonster(fbb)
    }
    fbb.createVectorOfTables(allMonsters)
  }

  @Benchmark
  fun moviesBuilder() {
    fbb.clear()
    val allMovies = Array(repetition) {
      val att = Attacker.createAttacker(fbb) { swordAttackDamage = it }.toUnion()
      val charsType = Movie.createCharactersTypeVector(
        fbb,
        arrayOf(CharacterE.BookFan, CharacterE.BookFan, CharacterE.BookFan)
      )
      val characters = Movie.createCharactersVector(
          fbb, arrayOf(
            BookReader.createBookReader(fbb, 10).toUnion(),
            BookReader.createBookReader(fbb, 20).toUnion(),
            BookReader.createBookReader(fbb, 30).toUnion()
          )
        )
      Movie.createMovie(fbb,
        mainCharacterType = CharacterE.MuLan,
        mainCharacter = att) {
        charactersType = charsType
        this.characters = characters
      }
    }
    fbb.createVectorOfTables(allMovies)
  }

  @Benchmark
  fun moviesManual() {
    fbb.clear()
    val allMovies = Array(repetition) {
      Attacker.startAttacker(fbb)
      Attacker.addSwordAttackDamage(fbb, it)
      val att = Attacker.endAttacker(fbb)

      val charsType = Movie.createCharactersTypeVector(
        fbb,
        arrayOf(CharacterE.BookFan, CharacterE.BookFan, CharacterE.BookFan)
      )

      val charsVec = Movie.createCharactersVector(
        fbb, arrayOf(
          BookReader.createBookReader(fbb, 10).toUnion(),
          BookReader.createBookReader(fbb, 20).toUnion(),
          BookReader.createBookReader(fbb, 30).toUnion()
        )
      )

      Movie.startMovie(fbb)
      Movie.addMainCharacterType(fbb, CharacterE.MuLan)
      Movie.addMainCharacter(fbb, att.toUnion())

      Movie.addCharactersType(fbb, charsType)
      Movie.addCharacters(fbb, charsVec)
      Movie.endMovie(fbb)
    }

    fbb.createVectorOfTables(allMovies)
  }
}
