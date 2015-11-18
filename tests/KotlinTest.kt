package com.google.flatbuffers.kotlin

import MyGame.Example.Color
import MyGame.Example.Monster
import java.io.File
import java.io.RandomAccessFile
import java.nio.ByteBuffer


fun main(args: Array<String>) {
    println("Hello, World!")

    val data = RandomAccessFile(File("monsterdata_test.mon"), "r").use {
        val temp = ByteArray(it.length().toInt())
        it.readFully(temp)
        temp
    }

    // Now test it:

    val bb = ByteBuffer.wrap(data);
    testBuffer(bb);

    println("Hello, World!")

}


fun testBuffer(bb:ByteBuffer) {
    //testEq(Monster.MonsterBufferHasIdentifier(bb), true);

    val monster = Monster.rootAsMonster(bb)

    testEq(monster.hp, 80.toShort());
    testEq(monster.mana, 150.toShort());  // default

    testEq(monster.name, "MyMonster");
    // monster.friendly() // can't access, deprecated

    val pos = monster.pos();
    testEq(pos!!.x(), 1.0f);
    testEq(pos.y(), 2.0f);
    testEq(pos.z(), 3.0f);
    testEq(pos.test1(), 3.0);
    testEq(pos.test2(), Color.Green);
    val t = pos.test3();
    testEq(t.a(), 5.toShort());
    testEq(t.b(), 6.toByte());

    testEq(monster.test_type, Monster);
    val monster2 = Monster()
    testEq(monster.test(monster2) != null, true);
    testEq(monster2.name, "Fred");

    testEq(monster.inventorySize, 5);
    var invsum = 0;
    for (i in  0 until monster.inventorySize) invsum += monster.inventory(i);
    testEq(invsum, 10);

/*    // Alternative way of accessing a vector:
    val ibb = monster.inventoryAsByteBuffer();
    invsum = 0;
    while (ibb.position() < ibb.limit()) invsum += ibb.get();
    testEq(invsum, 10);*/

    val test_0 = monster.test4(0);
    val test_1 = monster.test4(1);
    testEq(monster.test4Size, 2);
    testEq(test_0!!.a() + test_0.b() + test_1!!.a() + test_1.b(), 100);

    testEq(monster.testarrayofstringSize, 2);
    testEq(monster.testarrayofstring(0), "test1");
    testEq(monster.testarrayofstring(1), "test2");

    testEq(monster.testbool, false);
}

fun <T> testEq(a: T, b: T) {
    if (a != b) {
        System.out.println("${(a as? Any)?.javaClass?.name} + ${(b as? Any)?.javaClass?.name}")
        System.out.println("FlatBuffers test FAILED: '$a' != '$b'")
        // assert false;
        System.exit(1);
    }
}

