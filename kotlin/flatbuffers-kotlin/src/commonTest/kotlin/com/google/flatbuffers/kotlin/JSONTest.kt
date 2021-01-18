/*
 * Copyright 2021 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.google.flatbuffers.kotlin

import kotlin.test.Test
import kotlin.test.assertEquals

class JSONTest {
  @Test
  fun parseSample() {
    val dataStr = """
      {
        "ary" : [1, 2, 3],
        "boolean_false": false,
"boolean_true": true, "double": 1.2E33,
        "hello":"world"
   ,"interesting": "value",

 "null_value":  null,


  "object" : {
    "field1": "hello"
  }
 }
    """
    val data = dataStr.encodeToByteArray()
    val root = JSONParser().parse(ArrayReadWriteBuffer(data, writePosition = data.size))
    println(root.toJson())
    val map = root.toMap()

    val minified = data.filterNot { it == ' '.toByte() || it == '\n'.toByte() }.toByteArray().decodeToString()
    assertEquals(8, map.size)
    assertEquals("world", map["hello"].toString())
    assertEquals("value", map["interesting"].toString())
    assertEquals(12e32, map["double"].toDouble())
    assertArrayEquals(intArrayOf(1, 2, 3), map["ary"].toIntArray())
    assertEquals(true, map["boolean_true"].toBoolean())
    assertEquals(false, map["boolean_false"].toBoolean())
    assertEquals(true, map["null_value"].isNull)
    assertEquals("hello", map["object"]["field1"].toString())

    val obj = map["object"]
    assertEquals(true, obj.isMap)
    assertEquals("{\"field1\":\"hello\"}", obj.toJson())
    assertEquals(minified, root.toJson())
  }

  @Test
  fun testDoubles() {
    val values = arrayOf(
      "-0.0",
      "1.0",
      "1.7976931348613157",
      "0.0",
      "-0.5",
      "3.141592653589793",
      "2.718281828459045E-3",
      "2.2250738585072014E-308",
      "4.9E-15",
    )
    val parser = JSONParser()
    assertEquals(-0.0, parser.parse(values[0]).toDouble())
    assertEquals(1.0, parser.parse(values[1]).toDouble())
    assertEquals(1.7976931348613157, parser.parse(values[2]).toDouble())
    assertEquals(0.0, parser.parse(values[3]).toDouble())
    assertEquals(-0.5, parser.parse(values[4]).toDouble())
    assertEquals(3.141592653589793, parser.parse(values[5]).toDouble())
    assertEquals(2.718281828459045e-3, parser.parse(values[6]).toDouble())
    assertEquals(2.2250738585072014E-308, parser.parse(values[7]).toDouble())
    assertEquals(4.9E-15, parser.parse(values[8]).toDouble())
  }

  @Test
  fun testInts() {
    val values = arrayOf(
      "-0",
      "0",
      "-1",
      "${Int.MAX_VALUE}",
      "${Int.MIN_VALUE}",
      "${Long.MAX_VALUE}",
      "${Long.MIN_VALUE}",
    )
    val parser = JSONParser()

    assertEquals(parser.parse(values[0]).toInt(), 0)
    assertEquals(parser.parse(values[1]).toInt(), 0)
    assertEquals(parser.parse(values[2]).toInt(), -1)
    assertEquals(parser.parse(values[3]).toInt(), Int.MAX_VALUE)
    assertEquals(parser.parse(values[4]).toInt(), Int.MIN_VALUE)
    assertEquals(parser.parse(values[5]).toLong(), Long.MAX_VALUE)
    assertEquals(parser.parse(values[6]).toLong(), Long.MIN_VALUE)
  }

  @Test
  fun testBooleansAndNull() {
    val values = arrayOf(
      "true",
      "false",
      "null"
    )
    val parser = JSONParser()

    assertEquals(true, parser.parse(values[0]).toBoolean())
    assertEquals(false, parser.parse(values[1]).toBoolean())
    assertEquals(true, parser.parse(values[2]).isNull)
  }

  @Test
  fun testStrings() {
    val values = arrayOf(
      "\"\"",
      "\"a\"",
      "\"hello world\"",
      "\"\\\"\\\\\\/\\b\\f\\n\\r\\t cool\"",
      "\"\\u0000\"",
      "\"\\u0021\"",
      "\"hell\\u24AC\\n\\ro wor \\u0021 ld\""
    )
    val parser = JSONParser()

    // empty
    var ref = parser.parse(values[0])
    assertEquals(true, ref.isString)
    assertEquals("", ref.toString())
    // a
    ref = parser.parse(values[1])
    assertEquals(true, ref.isString)
    assertEquals("a", ref.toString())
    // hello world
    ref = parser.parse(values[2])
    assertEquals(true, ref.isString)
    assertEquals("hello world", ref.toString())
    // "\\\"\\\\\\/\\b\\f\\n\\r\\t\""
    ref = parser.parse(values[3])
    assertEquals(true, ref.isString)
    assertEquals("\"\\/\b${12.toChar()}\n\r\t cool", ref.toString())
    // 0
    ref = parser.parse(values[4])
    assertEquals(true, ref.isString)
    assertEquals(0.toChar().toString(), ref.toString())
    // u0021
    ref = parser.parse(values[5])
    assertEquals(true, ref.isString)
    assertEquals(0x21.toChar().toString(), ref.toString())
    // u20AC
    ref = parser.parse(values[6])
    assertEquals(true, ref.isString)
    assertEquals("hell${0x24AC.toChar()}\n\ro wor ${0x21.toChar()} ld", ref.toString())
  }

  @Test
  fun testUnicode() {
    // took from test/unicode_test.json
    val data = """
      {
        "name": "unicode_test",
        "testarrayofstring": [
          "Цлїςσδε",
          "ﾌﾑｱﾑｶﾓｹﾓ",
          "フムヤムカモケモ",
          "㊀㊁㊂㊃㊄",
          "☳☶☲",
          "𡇙𝌆"
        ],
        "testarrayoftables": [
          {
            "name": "Цлїςσδε"
          },
          {
            "name": "☳☶☲"
          },
          {
            "name": "フムヤムカモケモ"
          },
          {
            "name": "㊀㊁㊂㊃㊄"
          },
          {
            "name": "ﾌﾑｱﾑｶﾓｹﾓ"
          },
          {
            "name": "𡇙𝌆"
          }
        ]
      }
    """.trimIndent()
    val parser = JSONParser()
    val ref = parser.parse(data)

    // name
    assertEquals(3, ref.toMap().size)
    assertEquals("unicode_test", ref["name"].toString())
    // testarrayofstring
    assertEquals(6, ref["testarrayofstring"].toVector().size)
    assertEquals("Цлїςσδε", ref["testarrayofstring"][0].toString())
    assertEquals("ﾌﾑｱﾑｶﾓｹﾓ", ref["testarrayofstring"][1].toString())
    assertEquals("フムヤムカモケモ", ref["testarrayofstring"][2].toString())
    assertEquals("㊀㊁㊂㊃㊄", ref["testarrayofstring"][3].toString())
    assertEquals("☳☶☲", ref["testarrayofstring"][4].toString())
    assertEquals("𡇙𝌆", ref["testarrayofstring"][5].toString())
    // testarrayoftables
    assertEquals(6, ref["testarrayoftables"].toVector().size)
    assertEquals("Цлїςσδε", ref["testarrayoftables"][0]["name"].toString())
    assertEquals("☳☶☲", ref["testarrayoftables"][1]["name"].toString())
    assertEquals("フムヤムカモケモ", ref["testarrayoftables"][2]["name"].toString())
    assertEquals("㊀㊁㊂㊃㊄", ref["testarrayoftables"][3]["name"].toString())
    assertEquals("ﾌﾑｱﾑｶﾓｹﾓ", ref["testarrayoftables"][4]["name"].toString())
    assertEquals("𡇙𝌆", ref["testarrayoftables"][5]["name"].toString())
  }

  @Test
  fun testArrays() {
    val values = arrayOf(
      "[]",
      "[1]",
      "[0,1, 2,3  , 4 ]",
      "[1.0, 2.2250738585072014E-308,  4.9E-320]",
      "[1.0, 2,  \"hello world\"]   ",
      "[ 1.1, 2, [ \"hello\" ] ]",
      "[[[1]]]"
    )
    val parser = JSONParser()

    // empty
    var ref = parser.parse(values[0])
    assertEquals(true, ref.isVector)
    assertEquals(0, parser.parse(values[0]).toVector().size)
    // single
    ref = parser.parse(values[1])
    assertEquals(true, ref.isTypedVector)
    assertEquals(1, ref[0].toInt())
    // ints
    ref = parser.parse(values[2])
    assertEquals(true, ref.isTypedVector)
    assertEquals(T_VECTOR_INT, ref.type)
    assertEquals(5, ref.toVector().size)
    for (i in 0..4) {
      assertEquals(i, ref[i].toInt())
    }
    // floats
    ref = parser.parse(values[3])
    assertEquals(true, ref.isTypedVector)
    assertEquals(T_VECTOR_FLOAT, ref.type)
    assertEquals(3, ref.toVector().size)
    assertEquals(1.0, ref[0].toDouble())
    assertEquals(2.2250738585072014E-308, ref[1].toDouble())
    assertEquals(4.9E-320, ref[2].toDouble())
    // mixed
    ref = parser.parse(values[4])
    assertEquals(false, ref.isTypedVector)
    assertEquals(T_VECTOR, ref.type)
    assertEquals(1.0, ref[0].toDouble())
    assertEquals(2, ref[1].toInt())
    assertEquals("hello world", ref[2].toString())
    // nester array
    ref = parser.parse(values[5])
    assertEquals(false, ref.isTypedVector)
    assertEquals(T_VECTOR, ref.type)
    assertEquals(1.1, ref[0].toDouble())
    assertEquals(2, ref[1].toInt())
    assertEquals("hello", ref[2][0].toString())
  }

  @Test
  fun testInvalidJson() {
    val fails = arrayOf(
      "[1,,2]",
      "[\"Unclosed array\"",
      "{unquoted_key: \"keys must be quoted\"}",
      "[\"extra comma\",]",
      "[\"double extra comma\",,]",
      "[   , \"<-- missing value\"]",
      "[\"Comma after the close\"],",
      "[\"Extra close\"]]",
      "{\"Extra comma\": true,}",
      "{\"Extra value after close\": true} \"misplaced quoted value\"",
      "{\"Illegal expression\": 1 + 2}",
      "{\"Illegal invocation\": alert()}",
      "{\"Numbers cannot have leading zeroes\": 013}",
      "{\"Numbers cannot be hex\": 0x14}",
      "[\"Illegal backslash escape: \\x15\"]",
      "[\\naked]",
      "[\"Illegal backslash escape: \\017\"]",
      "{\"Missing colon\" null}",
      "{\"Double colon\":: null}",
      "{\"Comma instead of colon\", null}",
      "[\"Colon instead of comma\": false]",
      "[\"Bad value\", truth]",
      "['single quote']",
      "[\"\ttab\tcharacter\tin\tstring\t\"]\n",
      "[\"tab\\   character\\   in\\  string\\  \"]",
      "[\"line\nbreak\"]",
      "[\"line\\\nbreak\"]",
      "[0e]",
      "[0e+]",
      "[0e+-1]",
      "{\"Comma instead if closing brace\": true,",
      "[\"mismatch\"}"
    )
    val parser = JSONParser()
    var count = 0
    fails.forEach {
      try {
        parser.parse(it)
        throw RuntimeException("$it should fail")
      } catch (e: IllegalStateException) {
        println("\"$it\" ${e.message}")
        count++
      }
    }
    assertEquals(fails.size, count)
  }
}
