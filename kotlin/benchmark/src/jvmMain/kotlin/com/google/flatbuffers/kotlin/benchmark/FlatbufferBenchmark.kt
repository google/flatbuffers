@file:OptIn(ExperimentalUnsignedTypes::class)

package com.google.flatbuffers.kotlin.benchmark


import com.google.flatbuffers.kotlin.FlatBufferBuilder
import jmonster.JAllMonsters
import jmonster.JMonster
import jmonster.JVec3
import monster.AllMonsters.Companion.createAllMonsters
import monster.AllMonsters.Companion.createMonstersVector
import monster.Monster
import monster.Monster.Companion.createInventoryVector
import monster.Monster.Companion.createMonster
import monster.Vec3
import org.openjdk.jmh.annotations.*
import java.util.concurrent.TimeUnit

@State(Scope.Benchmark)
@BenchmarkMode(Mode.AverageTime)
@OutputTimeUnit(TimeUnit.NANOSECONDS)
@Measurement(iterations = 20, time = 1, timeUnit = TimeUnit.NANOSECONDS)
open class FlatbufferBenchmark {

  val repetition = 1000000
  val fbKotlin = FlatBufferBuilder(1024 * repetition)
  val fbJava = com.google.flatbuffers.FlatBufferBuilder(1024 * repetition)

  @OptIn(ExperimentalUnsignedTypes::class)
  @Benchmark
  fun monstersKotlin() {
    fbKotlin.clear()
    val monsterName = fbKotlin.createString("MonsterName");
    val inv = createInventoryVector(fbKotlin, byteArrayOf(0, 1, 2, 3, 4).asUByteArray())
    val monsters = createMonstersVector(fbKotlin, Array(repetition) {
      Monster.startMonster(fbKotlin)
      Monster.addName(fbKotlin, monsterName)
      Monster.addPos(fbKotlin, Vec3.createVec3(fbKotlin, 1.0f, 2.0f, 3.0f))
      Monster.addHp(fbKotlin, 80)
      Monster.addMana(fbKotlin, 150)
      Monster.addInventory(fbKotlin, inv)
      Monster.endMonster(fbKotlin)
    })
    val allMonsters = createAllMonsters(fbKotlin, monsters)
    fbKotlin.finish(allMonsters)
  }

  @Benchmark
  fun monstersjava() {
    fbJava.clear()
    val monsterName = fbJava.createString("MonsterName");
    val inv = JMonster.createInventoryVector(fbJava, byteArrayOf(0, 1, 2, 3, 4).asUByteArray())
    val monsters = JAllMonsters.createMonstersVector(fbJava, IntArray(repetition) {
      JMonster.startJMonster(fbJava)
      JMonster.addName(fbJava, monsterName)
      JMonster.addPos(fbJava, JVec3.createJVec3(fbJava, 1.0f, 2.0f, 3.0f))
      JMonster.addHp(fbJava, 80)
      JMonster.addMana(fbJava, 150)
      JMonster.addInventory(fbJava, inv)
      JMonster.endJMonster(fbJava)
    })
    val allMonsters = JAllMonsters.createJAllMonsters(fbJava, monsters)
    fbJava.finish(allMonsters)
  }

}
