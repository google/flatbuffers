package com.google.flatbuffers.kotlin

import kotlin.test.Test
import kotlin.test.assertEquals

class BuffersTest {

  @Test
  fun readBufferStringTest() {
    val text = "Hello world!"
    val bytes = text.encodeToByteArray()
    val fullRead = ArrayReadBuffer(bytes)
    val helloRead = ArrayReadBuffer(bytes, limit = 5)
    val worldRead = fullRead.slice(6, 6)

    assertEquals(bytes.size, fullRead.limit)
    assertEquals(text, fullRead.getString(0, fullRead.limit))
    assertEquals("Hello" , helloRead.getString(0, helloRead.limit))
    assertEquals("world!" , worldRead.getString())
    assertEquals(fullRead.getString(0, 5) , helloRead.getString(0, helloRead.limit))
    assertEquals(fullRead.getString(6, 6) , worldRead.getString(0, worldRead.limit))

    for (i in 0 until helloRead.limit) {
      assertEquals(fullRead[i], helloRead[i])
    }
    for (i in 0 until worldRead.limit) {
      assertEquals(fullRead[6 + i], worldRead[i])
    }
  }

  @Test
  fun readWriteBufferPrimitivesTest() {
    val text = "Hello world!"
    val bytes = text.encodeToByteArray()
    val wt = ArrayReadWriteBuffer(bytes)

    wt.put("Tests")
    val pos0 = wt.writePosition
    assertEquals("Tests world!", wt.getString(0, wt.capacity))
    wt.put(Int.MAX_VALUE)
    assertEquals(Int.MAX_VALUE, wt.getInt(pos0))

    val pos = wt.writePosition
    wt.put(Double.NEGATIVE_INFINITY)
    assertEquals(Double.NEGATIVE_INFINITY, wt.getDouble(pos))

    val jap = " are really すごい!".encodeToByteArray()
    wt.writePosition = pos0
    wt.put(jap)
    assertEquals("Tests are really すごい!", wt.getString())
  }

  @Test
  fun readWriteBufferGrowthTest() {
    val a = ArrayReadWriteBuffer(1)
    assertEquals(1, a.capacity)
    a.put(0.toByte())
    assertEquals(1, a.capacity)
    a.put(0xFF.toShort())
    assertEquals(8, a.capacity)
    a.put(ByteArray(12) { it.toByte() })
    assertEquals(16, a.capacity)
    a.requestCapacity(513, false)
    assertEquals(1024, a.capacity)
    a.requestCapacity(234, false)
    assertEquals(1024, a.capacity)
  }
}
