package com.google.flatbuffers.kotlin

import Example.*
import java.io.DataOutputStream
import java.io.File
import java.io.FileOutputStream
import java.io.RandomAccessFile
import java.nio.ByteBuffer


fun main(args: Array<String>) {

        val data = RandomAccessFile(File("/home/ubuntu/git/flatbuffers/tests/monsterdata_test.mon"), "r").use {
            val temp = ByteArray(it.length().toInt())
            it.readFully(temp)
            temp
        }

        // Now test it:

        val bb = ByteBuffer.wrap(data)
        testBuffer(bb)

        // Second, let's create a FlatBuffer from scratch in Java, and test it also.
        // We use an initial size of 1 to exercise the reallocation algorithm,
        // normally a size larger than the typical FlatBuffer you generate would be
        // better for performance.
        val builder = FlatBufferBuilder(1)

        // We set up the same values as monsterdata.json:


	with (builder) {
            with(Monster) {
                val str = stringOf("MyMonster")

                val inv = inventoryOf(0, 1, 2, 3, 4)

                val fred = stringOf("Fred")

                val mon2 = monsterOf { name(fred) }

                val test4 = test4Of(2) {
                    testOf(10.toShort(), 20.toByte())
                    testOf(30.toShort(), 40.toByte())
                }

                val testArrayOfString = testarrayofstringOf(stringOf("test1"), stringOf("test2"))

		val mon = monsterOf {
                	pos(vec3Of(1.0f, 2.0f, 3.0f, 3.0, Color.Green, 5.toShort(), 6.toByte()))
                	hp(80.toShort())
                	name(str)
                	inventory(inv)
                	testType(Example.Any.Monster)
                	test(mon2)
                	test4(test4)
                	testarrayofstring(testArrayOfString)
                	testbool(false)
                	testhashu32Fnv1(Integer.MAX_VALUE + 1L)
                }
                finishBuffer(mon)
            }
        }

        /*val str = builder.createString("MyMonster");

        val inv = with(Monster) { builder.inventoryOf(0, 1, 2, 3, 4) }

        val fred = builder.createString("Fred")
        val mon2 = with (Monster) {builder.start().name(fred).end()}
        with(Monster) { builder.startTest4Array(2) }
        builder.testOf(10.toShort(), 20.toByte())
            builder.testOf(30.toShort(), 40.toByte())
        
        val test4 = builder.endArray()

        val testArrayOfString = with (Monster) {
            with (builder) {
                testarrayofstringOf(createString("test1"), createString("test2"))
            }
        }

        with (builder) {
            with (Monster) {
                val mon = start()
                        .pos(vec3Of(1.0f, 2.0f, 3.0f, 3.0, Color.Green, 5.toShort(), 6.toByte()))
                        .hp(80.toShort())
                        .name(str)
                        .inventory(inv)
                        .testType(Example.Any.Monster)
                        .test(mon2)
                        .test4(test4)
                        .testarrayofstring(testArrayOfString)
                        .testbool(false)
                        .testhashu32Fnv1(Integer.MAX_VALUE + 1L)
                        .end()
                finishBuffer(mon)
            }
        }*/

        /*with (Monster) {

            val mon =
                    builder.start()
                            .pos(builder.vec3Of(1.0f, 2.0f, 3.0f, 3.0, Color.Green, 5.toShort(), 6.toByte()) )
                            .hp(80.toShort())
                            .name(str)
                            .inventory(inv)
                            .testType(Example.Any.Monster)
                            .test(mon2)
                            .test4(test4)
                            .testarrayofstring(testArrayOfString)
                            .testbool(false)
                            .testhashu32Fnv1(Integer.MAX_VALUE + 1L)
                            .end()


            builder.finishBuffer(mon)
        }*/

        // Write the result to a file for debugging purposes:
        // Note that the binaries are not necessarily identical, since the JSON
        // parser may serialize in a slightly different order than the above
        // Java code. They are functionally equivalent though.

        DataOutputStream(FileOutputStream("monsterdata_java_wire.mon")).use {
            it.write(builder.dataBuffer.array(), builder.dataBuffer.position(), builder.offset());
        }

        // Test it:
        testExtendedBuffer(builder.dataBuffer)

        // Make sure it also works with read only ByteBuffers. This is slower,
        // since creating strings incurs an additional copy
        // (see Table.__string).
        testExtendedBuffer(builder.dataBuffer.asReadOnlyBuffer())

        testEnums()

        //Attempt to mutate Monster fields and check whether the buffer has been mutated properly
        // revert to original values after testing
        val monster = Monster().wrap(builder.dataBuffer)

        // mana is optional and does not exist in the buffer so the mutation should fail
        // the mana field should retain its default value
        testEq(monster.mutateMana(10.toShort()), false)
        testEq(monster.mana, 150.toShort())

        // testType is an existing field and mutating it should succeed
        testEq(monster.testType, Example.Any.Monster)
        testEq(monster.mutateTestType(Example.Any.NONE), true)
        testEq(monster.testType, Example.Any.NONE)
        testEq(monster.mutateTestType(Example.Any.Monster), true)
        testEq(monster.testType, Example.Any.Monster)

        //mutate the inventory vector
        testEq(monster.mutateInventory(0, 1), true);
        testEq(monster.mutateInventory(1, 2), true);
        testEq(monster.mutateInventory(2, 3), true);
        testEq(monster.mutateInventory(3, 4), true);
        testEq(monster.mutateInventory(4, 5), true);

        for (i in  0 until monster.inventorySize) testEq(monster.inventory(i), i + 1);

        //reverse mutation
        testEq(monster.mutateInventory(0, 0), true)
        testEq(monster.mutateInventory(1, 1), true)
        testEq(monster.mutateInventory(2, 2), true)
        testEq(monster.mutateInventory(3, 3), true)
        testEq(monster.mutateInventory(4, 4), true)

        // get a struct field and edit one of its fields
        val pos = monster.pos!!
        testEq(pos.x, 1.0f)
        pos.x = 55.0f
        testEq(pos.x, 55.0f)
        pos.x = 1.0f
        testEq(pos.x, 1.0f)

        testExtendedBuffer(builder.dataBuffer.asReadOnlyBuffer())

        println("FlatBuffers test: completed successfully")
    }


    fun testBuffer(bb: ByteBuffer) {
        testEq(Monster.hasIdentifier(bb), true);

        val monster = Monster(bb)

        testEq(monster.hp, 80.toShort())
        testEq(monster.mana, 150.toShort())  // default

        testEq(monster.name, "MyMonster")
        // monster.friendly() // can't access, deprecated

        val pos = monster.pos!!
        testEq(pos.x, 1.0f);
        testEq(pos.y, 2.0f);
        testEq(pos.z, 3.0f);
        testEq(pos.test1, 3.0);
        testEq(pos.test2, Color.Green);
        val t = pos.test3;
        testEq(t.a, 5.toShort());
        testEq(t.b, 6.toByte());

        testEq(monster.testType, Example.Any.Monster)
        val monster2 = Monster()
        testEq(monster.test(monster2) != null, true);
        testEq(monster2.name, "Fred");

        testEq(monster.inventorySize, 5);
        var invsum = 0;
        for (i in  0 until monster.inventorySize) invsum += monster.inventory(i);
        testEq(invsum, 10);

        // Alternative way of accessing a vector:
        val ibb = monster.inventoryAsByteBuffer
        invsum = 0;
        while (ibb.position() < ibb.limit()) invsum += ibb.get();
        testEq(invsum, 10);

        val test_0 = monster.test4(0)!!
        val test_1 = monster.test4(1)!!
        testEq(monster.test4Size, 2)
        testEq(test_0.a + test_0.b + test_1.a + test_1.b, 100)

        testEq(monster.testarrayofstringSize, 2);
        testEq(monster.testarrayofstring(0), "test1")
        testEq(monster.testarrayofstring(1), "test2")

        testEq(monster.testbool, false)
    }

    // this method checks additional fields not present in the binary buffer read from file
    // these new tests are performed on top of the regular tests
    fun testExtendedBuffer(bb: ByteBuffer) {
        testBuffer(bb);

        val monster = Monster(bb)

        testEq(monster.testhashu32Fnv1, Integer.MAX_VALUE + 1L)
    }


    fun testEnums() {
        testEq(Color.Red.name, "Red");
        testEq(Color.Blue.name, "Blue");
        testEq(Example.Any.valueOf("NONE"), Example.Any.NONE);
        testEq(Example.Any.valueOf("Monster"), Example.Any.Monster);
    }

    fun <T> testEq(a: T, b: T) = if (a != b) {
        println("${(a as? Any)?.javaClass?.name} + ${(b as? Any)?.javaClass?.name}")
        println("FlatBuffers test FAILED: '$a' != '$b'")
        // assert false;
        System.exit(1);
    } else Unit

