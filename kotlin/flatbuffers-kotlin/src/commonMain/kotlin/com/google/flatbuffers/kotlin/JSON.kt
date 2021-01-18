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
@file:Suppress("NOTHING_TO_INLINE")

package com.google.flatbuffers.kotlin

import com.google.flatbuffers.kotlin.FlexBuffersBuilder.Companion.SHARE_KEYS_AND_STRINGS
import kotlin.experimental.and
import kotlin.math.pow

/**
 * Returns a minified version of this FlexBuffer as a JSON.
 */
public fun Reference.toJson(): String = ArrayReadWriteBuffer(1024).let {
  toJson(it)
  val data = it.data() // it.getString(0, it.writePosition)
  return data.decodeToString(0, it.writePosition)
}

/**
 * Returns a minified version of this FlexBuffer as a JSON.
 * @param out [ReadWriteBuffer] the JSON will be written.
 */
public fun Reference.toJson(out: ReadWriteBuffer) {
  when (type) {
    T_STRING -> {
      val start = buffer.indirect(end, parentWidth)
      val size = buffer.readULong(start - byteWidth, byteWidth).toInt()
      out.jsonEscape(buffer, start, size)
    }
    T_KEY -> {
      val start = buffer.indirect(end, parentWidth)
      val end = buffer.findFirst(0.toByte(), start)
      out.jsonEscape(buffer, start, end - start)
    }
    T_BLOB -> {
      val blob = toBlob()
      out.jsonEscape(out, blob.end, blob.size)
    }
    T_INT -> out.put(toLong().toString())
    T_UINT -> out.put(toULong().toString())
    T_FLOAT -> out.put(toDouble().toString())
    T_NULL -> out.put("null")
    T_BOOL -> out.put(toBoolean().toString())
    T_MAP -> toMap().toJson(out)
    T_VECTOR, T_VECTOR_BOOL, T_VECTOR_FLOAT, T_VECTOR_INT,
    T_VECTOR_UINT, T_VECTOR_KEY, T_VECTOR_STRING_DEPRECATED -> toVector().toJson(out)
    else -> error("Unable to convert type ${type.typeToString()} to JSON")
  }
}

/**
 * Returns a minified version of this FlexBuffer as a JSON.
 */
public fun Map.toJson(): String = ArrayReadWriteBuffer(1024).let { toJson(it); it.toString() }

/**
 * Returns a minified version of this FlexBuffer as a JSON.
 * @param out [ReadWriteBuffer] the JSON will be written.
 */
public fun Map.toJson(out: ReadWriteBuffer) {
  out.put('{'.toByte())
  // key values pairs
  for (i in 0 until size) {
    val key = keyAt(i)
    out.jsonEscape(buffer, key.start, key.sizeInBytes)
    out.put(':'.toByte())
    get(i).toJson(out)
    if (i != size - 1) {
      out.put(','.toByte())
    }
  }
  // close bracket
  out.put('}'.toByte())
}

/**
 * Returns a minified version of this FlexBuffer as a JSON.
 */
public fun Vector.toJson(): String = ArrayReadWriteBuffer(1024).let { toJson(it); it.toString() }

/**
 * Returns a minified version of this FlexBuffer as a JSON.
 * @param out that the JSON is being concatenated.
 */
public fun Vector.toJson(out: ReadWriteBuffer) {
  out.put('['.toByte())
  for (i in 0 until size) {
    get(i).toJson(out)
    if (i != size - 1) {
      out.put(','.toByte())
    }
  }
  out.put(']'.toByte())
}

/**
 * JSONParser class is used to parse a JSON as FlexBuffers. Calling [JSONParser.parse] fiils [output]
 * and returns a [Reference] ready to be used.
 */
public class JSONParser(public var output: FlexBuffersBuilder = FlexBuffersBuilder(1024, SHARE_KEYS_AND_STRINGS)) {
  private var readPos = 0

  private inline fun isPlainStringChar(c: Byte): Boolean {
    val flags = parseFlags
    // return c in 0x20..0x7f && c != 0x22.toByte() && c != 0x5c.toByte()
    return (flags[c.toInt() and 0xFF] and 1) != 0.toByte()
  }

  private inline fun isWhitespace(c: Byte): Boolean {
    val flags = parseFlags
    // return c == '\r'.toByte() || c == '\n'.toByte() || c == '\t'.toByte() || c == ' '.toByte()
    return (flags[c.toInt() and 0xFF] and 2) != 0.toByte()
  }

  private fun reset() {
    readPos = 0
    output.clear()
  }

  // keeps increasing [readPos] until finds a non-whitespace byte
  private inline fun skipWhitespace(data: ReadBuffer): Byte {
    val limit = data.limit
    if (data is ArrayReadBuffer) {
      // enables range check elimination
      val ary = data.data()
      return skipWhitespace(limit) { ary[it] }
    }
    return skipWhitespace(limit) { data[it] }
  }

  private inline fun skipWhitespace(limit: Int, crossinline fetch: (Int) -> Byte): Byte {
    var pos = readPos
    while (pos < limit) {
      val d = fetch(pos++)
      if (!isWhitespace(d)) {
        readPos = pos
        return d
      }
    }
    readPos = limit
    return LETTER_EOF
  }

  /**
   * Parse a json as [String] and returns a [Reference] to a FlexBuffer.
   */
  public fun parse(data: String): Reference = parse(ArrayReadBuffer(data.encodeToByteArray()))

  /**
   * Parse a json as [ByteArray] and returns a [Reference] to a FlexBuffer.
   */
  public fun parse(data: ByteArray): Reference = parse(ArrayReadBuffer(data))

  /**
   * Parse a json as [ReadBuffer] and returns a [Reference] to a FlexBuffer.
   */
  public fun parse(data: ReadBuffer): Reference {
    reset()
    parseValue(data, skipWhitespace(data), null)
    if (readPos < data.limit) {
      val tok = skipWhitespace(data)
      if (tok != LETTER_EOF) {
        makeError(data, "Extraneous charaters after parse has finished", tok)
      }
    }
    output.finish()
    return getRoot(output.buffer)
  }

  private fun parseValue(data: ReadBuffer, first: Byte, key: String? = null): FlexBufferType {
    return when (first) {
      LETTER_OPEN_OBJECT -> parseObject(data, key)
      LETTER_OPEN_ARRAY -> parseArray(data, key)
      LETTER_t -> parseTrue(data, key)
      LETTER_n -> parseNull(data, key)
      LETTER_f -> parseFalse(data, key)
      LETTER_DOUBLE_QUOTE -> parseString(data, key)
      LETTER_0, LETTER_1, LETTER_2, LETTER_3, LETTER_4, LETTER_5,
      LETTER_6, LETTER_7, LETTER_8, LETTER_9, LETTER_MINUS -> parseNumber(data, data.data(), first, key)
      else -> makeError(data, "Unexpected Character while parsing", first)
    }
  }

  private fun parseObject(data: ReadBuffer, key: String? = null): FlexBufferType {
    val fPos = output.startMap()
    var lastTok = LETTER_OPEN_OBJECT
    var tok: Byte = skipWhitespace(data)
    val limit = data.limit
    while (readPos <= limit) {
      when (tok) {
        LETTER_CLOSE_OBJECT -> {
          if (lastTok == LETTER_COMMA) {
            makeError(data, "Expecting a value after ${LETTER_COMMA.print()}", tok)
          }
          output.endMap(fPos, key); return T_MAP
        }
        LETTER_COMMA, 0.toByte() -> {
          if (lastTok == LETTER_OPEN_OBJECT || lastTok == LETTER_COMMA) {
            makeError(data, "Expecting a key/value after ${lastTok.print()}", tok)
          }
          lastTok = tok
          tok = skipWhitespace(data)
        }
        LETTER_DOUBLE_QUOTE -> {
          val childKey = readString(data)
          tok = skipWhitespace(data)
          if (tok.toChar() != ':')
            makeError(data, "Expecting ':' after object Key", tok)
          parseValue(data, skipWhitespace(data), childKey)
          lastTok = tok
          tok = skipWhitespace(data)
          if (tok != LETTER_CLOSE_OBJECT && tok != LETTER_COMMA)
            makeError(
              data,
              "Expecting '${LETTER_CLOSE_OBJECT.print()}' or '${LETTER_COMMA.print()}' after a key/value",
              tok
            )
        }
        else -> makeError(data, "Expecting start of object key", tok)
      }
    }
    makeError(data, "Unable to parse the object", tok)
  }

  private fun parseArray(data: ReadBuffer, key: String? = null): FlexBufferType {
    val fPos = output.startVector()
    var lastTok: Byte = LETTER_OPEN_ARRAY // arrays always start with '['
    var tok: Byte = skipWhitespace(data)
    var elementType = T_INVALID
    var multiType = false
    val limit = data.limit
    while (readPos <= limit) {
      when (tok) {
        LETTER_CLOSE_ARRAY -> {
          if (lastTok == LETTER_COMMA) {
            makeError(data, "Expecting a value after ${LETTER_COMMA.print()}", tok)
          }
          return if (!multiType && elementType.isScalar()) {
            output.endTypedVector(fPos, key)
            elementType.toElementTypedVector()
          } else {
            output.endVector(key, fPos)
            T_VECTOR
          }
        }
        LETTER_COMMA, 0.toByte() -> {
          if (lastTok == LETTER_OPEN_ARRAY || lastTok == LETTER_COMMA) {
            makeError(data, "Expecting a value after ${lastTok.print()}", tok)
          }
          lastTok = tok
          tok = skipWhitespace(data)
        }
        LETTER_EOF -> makeError(data, "Expecting value or ${LETTER_CLOSE_ARRAY.print()}", tok)
        else -> {
          lastTok = LETTER_EOF // Anything but '[' since we will be reading a value
          val newType = parseValue(data, tok)

          if (elementType == T_INVALID) {
            elementType = newType
          } else if (newType != elementType) {
            multiType = true
          }
          tok = skipWhitespace(data)
          if (tok != LETTER_CLOSE_ARRAY && tok != LETTER_COMMA)
            makeError(
              data,
              "Expecting ${LETTER_CLOSE_ARRAY.print()} or ${LETTER_COMMA.print()} after a value",
              tok
            )
        }
      }
    }
    makeError(data, "Unable to parse the array", tok)
  }

  private fun parseNumber(data: ReadBuffer, array: ByteArray, first: Byte, key: String?): FlexBufferType {
    val ary = array
    var cursor = readPos - 1
    var tok = first
    var useDouble = false
    val limit = ary.size
    var sign = 1
    var double = 0.0
    var long = 0L
    var digits = 0

    if (tok == LETTER_MINUS) {
      cursor++
      checkEOF(data, cursor)
      tok = ary[cursor]
      sign = -1
    }

    // peek first byte
    when (tok) {
      LETTER_0 -> {
        cursor++
        if (cursor != limit) {
          tok = ary[cursor]
        }
      }
      !in LETTER_0..LETTER_9 -> makeError(data, "Invalid Number", tok)
      else -> {
        do {
          val digit = tok - LETTER_0
          // double = 10.0 * double + digit
          long = 10 * long + digit
          digits++
          cursor++
          if (cursor == limit) break
          tok = ary[cursor]
        } while (tok in LETTER_0..LETTER_9)
      }
    }

    var exponent = 0
    // If we find '.' we need to convert to double
    if (tok == LETTER_DOT) {
      useDouble = true
      checkEOF(data, cursor)
      tok = ary[++cursor]
      if (tok < LETTER_0 || tok > LETTER_9) {
        makeError(data, "Invalid Number", tok)
      }
      do {
        // double = double * 10 + (tok - LETTER_0)
        long = 10 * long + (tok - LETTER_0)
        digits++
        --exponent
        cursor++
        if (cursor == limit) break
        tok = ary[cursor]
      } while (tok in LETTER_0..LETTER_9)
    }

    // If we find 'e' we need to convert to double
    if (tok == LETTER_e || tok == LETTER_E) {
      useDouble = true
      ++cursor
      checkEOF(data, cursor)
      tok = ary[cursor]
      var negativeExponent = false
      if (tok == LETTER_MINUS) {
        ++cursor
        checkEOF(data, cursor)
        negativeExponent = true
        tok = ary[cursor]
      } else if (tok == LETTER_PLUS) {
        ++cursor
        checkEOF(data, cursor)
        tok = ary[cursor]
      }
      if (tok < LETTER_0 || tok > LETTER_9) {
        makeError(data, "Missing exponent", tok)
      }
      var exp = 0
      do {
        val digit = tok - LETTER_0
        exp = 10 * exp + digit
        ++cursor
        if (cursor == limit) break
        tok = ary[cursor]
      } while (tok in LETTER_0..LETTER_9)

      exponent += if (negativeExponent) -exp else exp
    }

    if (digits > 17 || exponent < -19 || exponent > 19) {
      // if the float number is not simple enough
      // we use language's Double parsing, which is slower but
      // produce more expected results for extreme numbers.
      val firstPos = readPos - 1
      val str = data.getString(firstPos, cursor - firstPos)
      if (useDouble) {
        double = str.toDouble()
        output[key] = double
      } else {
        long = str.toLong()
        output[key] = long
      }
    } else {
      // this happens on single numbers outside any object
      // or array
      if (useDouble || exponent != 0) {
        double = if (long == 0L) 0.0 else long.toDouble() * exponent.pow10()
        double *= sign
        output[key] = double
      } else {
        long *= sign
        output[key] = long
      }
    }
    readPos = cursor
    return if (useDouble) T_FLOAT else T_INT
  }

  private fun parseString(data: ReadBuffer, key: String?): FlexBufferType {
    output[key] = readString(data)
    return T_STRING
  }

  private fun parseTrue(data: ReadBuffer, key: String? = null): FlexBufferType {
    checkEOF(data, readPos + 2)
    // 0x65757274 is equivalent to ['t', 'r', 'u', 'e' ] as a 4 byte Int
    if (data.getInt(readPos - 1) != 0x65757274) {
      makeError(data, "Expecting keyword \"true\"", data[readPos])
    }
    output[key] = true
    readPos += 3
    return T_BOOL
  }

  private fun parseFalse(data: ReadBuffer, key: String? = null): FlexBufferType {
    checkEOF(data, readPos + 3)
    // 0x65736c61 is equivalent to ['a', 'l', 's', 'e' ] as a 4 byte Int
    if (data.getInt(readPos) != 0x65736c61) {
      makeError(data, "Expecting keyword \"false\"", data[readPos])
    }
    output[key] = false
    readPos += 4
    return T_BOOL
  }

  private fun parseNull(data: ReadBuffer, key: String? = null): FlexBufferType {
    checkEOF(data, readPos + 2)
    // 0x6c6c756e  is equivalent to ['n', 'u', 'l', 'l' ] as a 4 byte Int
    if (data.getInt(readPos - 1) != 0x6c6c756e) {
      makeError(data, "Expecting keyword \"null\"", data[readPos])
    }
    output.putNull(key)
    readPos += 3
    return T_NULL
  }

  private fun readString(data: ReadBuffer): String {
    val limit = data.limit
    if (data is ArrayReadBuffer) {
      val ary = data.data()
      // enables range check elimination
      return readString(data, limit) { ary[it] }
    }
    return readString(data, limit) { data[it] }
  }

  private inline fun readString(data: ReadBuffer, limit: Int, crossinline fetch: (Int) -> Byte): String {
    var cursorPos = readPos
    var foundEscape = false
    var currentChar: Byte = 0
    // we loop over every 4 bytes until find any non-plain char
    while (limit - cursorPos >= 4) {
      currentChar = fetch(cursorPos)
      if (!isPlainStringChar(currentChar)) {
        foundEscape = true
        break
      }
      currentChar = fetch(cursorPos + 1)
      if (!isPlainStringChar(currentChar)) {
        cursorPos += 1
        foundEscape = true
        break
      }
      currentChar = fetch(cursorPos + 2)
      if (!isPlainStringChar(currentChar)) {
        cursorPos += 2
        foundEscape = true
        break
      }
      currentChar = fetch(cursorPos + 3)
      if (!isPlainStringChar(currentChar)) {
        cursorPos += 3
        foundEscape = true
        break
      }
      cursorPos += 4
    }
    if (!foundEscape) {
      // if non-plain string char is not found we loop over
      // the remaining bytes
      while (true) {
        if (cursorPos >= limit) {
          error("Unexpected end of string")
        }
        currentChar = fetch(cursorPos)
        if (!isPlainStringChar(currentChar)) {
          break
        }
        ++cursorPos
      }
    }
    if (currentChar == LETTER_DOUBLE_QUOTE) {
      val str = data.getString(readPos, cursorPos - readPos)
      readPos = cursorPos + 1
      return str
    }
    if (currentChar in 0..0x1f) {
      error("Illegal Codepoint")
    } else {
      // backslash or >0x7f
      return readStringSlow(data, currentChar, cursorPos)
    }
  }

  private fun readStringSlow(data: ReadBuffer, first: Byte, lastPos: Int): String {
    var cursorPos = lastPos

    var endOfString = lastPos
    while (true) {
      val pos = data.findFirst(LETTER_DOUBLE_QUOTE, endOfString)
      when {
        pos == -1 -> makeError(data, "Unexpected EOF, missing end of string'\"'", first)
        data[pos - 1] == LETTER_BACKSLASH -> endOfString = pos + 1
        else -> {
          endOfString = pos; break
        }
      }
    }
    // copy everything before the escape
    val builder = StringBuilder(data.getString(readPos, lastPos - readPos))
    while (true) {
      when (val pos = data.findFirst(LETTER_BACKSLASH, cursorPos, endOfString)) {
        -1 -> {
          val doubleQuotePos = data.findFirst(LETTER_DOUBLE_QUOTE, endOfString)
          if (doubleQuotePos == -1) makeError(data, "Reached EOF before enclosing string", first)
          val rest = data.getString(cursorPos, doubleQuotePos - cursorPos)
          builder.append(rest)
          readPos = doubleQuotePos + 1
          return builder.toString()
        }

        else -> {
          // we write everything up to \
          builder.append(data.getString(cursorPos, pos - cursorPos))
          val c = data[pos + 1]
          builder.append(readEscapedChar(data, c, pos))
          cursorPos = pos + if (c == LETTER_u) 6 else 2
        }
      }
    }
  }

  // byte1 is expected to be first char before `\`
  private inline fun readEscapedChar(data: ReadBuffer, byte1: Byte, cursorPos: Int): Char {
    return when (byte1) {
      LETTER_u -> {
        checkEOF(data, cursorPos + 1 + 4)
        var result: Char = 0.toChar()
        var i = cursorPos + 2 // cursorPos is on '\\', cursorPos + 1 is 'u'
        val end = i + 4
        while (i < end) {
          val part: Byte = data[i]
          result = (result.toInt() shl 4).toChar()
          result += when (part) {
            in LETTER_0..LETTER_9 -> part - LETTER_0
            in LETTER_a..LETTER_f -> part - LETTER_a + 10
            in LETTER_A..LETTER_F -> part - LETTER_A + 10
            else -> makeError(data, "Invalid utf8 escaped character", -1)
          }
          i++
        }
        result
      }
      LETTER_b -> '\b'
      LETTER_t -> '\t'
      LETTER_r -> '\r'
      LETTER_n -> '\n'
      LETTER_f -> 12.toChar() // '\f'
      LETTER_DOUBLE_QUOTE, LETTER_BACKSLASH, LETTER_FORWARDSLASH -> byte1.toChar()
      else -> makeError(data, "Invalid escape sequence.", byte1)
    }
  }

  private fun Byte.print(): String = when (this) {
    in 0x21..0x7E -> "'${this.toChar()}'" // visible ascii chars
    LETTER_EOF -> "EOF"
    else -> "'0x${this.toString(16)}'"
  }

  private inline fun makeError(data: ReadBuffer, msg: String, tok: Byte): Nothing {
    val (line, column) = calculateErrorPosition(data, readPos)
    error("Error At ($line, $column): $msg, got ${tok.print()}")
  }

  private inline fun checkEOF(data: ReadBuffer, pos: Int) {
    if (pos >= data.limit)
      makeError(data, "Unexpected end of file", -1)
  }

  private fun calculateErrorPosition(data: ReadBuffer, endPos: Int): Pair<Int, Int> {
    var line = 1
    var column = 1
    var current = 0
    while (current < endPos - 1) {
      if (data[current++] == LETTER_NEWLINE) {
        ++line
        column = 1
      } else {
        ++column
      }
    }
    return Pair(line, column)
  }
}

internal fun Int.pow10() = when {
  this > 308 -> error("JSON does not support INFINITY in numbers")
  this < -323 -> 10.0.pow(this) // slow path
  else -> doubleConstants[this + 323]
}

internal inline fun Int.toPaddedHex(): String = "\\u${this.toString(16).padStart(4, '0')}"

private inline fun ReadWriteBuffer.jsonEscape(data: ReadBuffer, start: Int, size: Int) {
  val replacements = JSON_ESCAPE_CHARS
  put(LETTER_DOUBLE_QUOTE)
  var last = start
  val length: Int = size
  val ary = data.data()
  for (i in start until start + length) {
    val c = ary[i].toUByte()
    var replacement: ByteArray?
    if (c.toInt() < 128) {
      replacement = replacements[c.toInt()]
      if (replacement == null) {
        continue
      }
    } else {
      continue
    }
    if (last < i) {
      put(ary, last, i - last)
    }
    put(replacement, 0, replacement.size)
    last = i + 1
  }
  if (last < (last + length)) {
    put(ary, last, (start + length) - last)
  }
  put(LETTER_DOUBLE_QUOTE)
}

// Following escape strategy defined in RFC7159.
private val JSON_ESCAPE_CHARS: Array<ByteArray?> = arrayOfNulls<ByteArray>(128).apply {
  this['\n'.toInt()] = "\\n".encodeToByteArray()
  this['\t'.toInt()] = "\\t".encodeToByteArray()
  this['\r'.toInt()] = "\\r".encodeToByteArray()
  this['\b'.toInt()] = "\\b".encodeToByteArray()
  this[0x0c] = "\\f".encodeToByteArray()
  this['"'.toInt()] = "\\\"".encodeToByteArray()
  this['\\'.toInt()] = "\\\\".encodeToByteArray()
  for (i in 0..0x1f) {
    this[i] = "\\u${i.toPaddedHex()}".encodeToByteArray()
  }
}

private const val LETTER_NEWLINE = '\n'.toByte()
private const val LETTER_OPEN_OBJECT = '{'.toByte()
private const val LETTER_CLOSE_OBJECT = '}'.toByte()
private const val LETTER_OPEN_ARRAY = '['.toByte()
private const val LETTER_CLOSE_ARRAY = ']'.toByte()
private const val LETTER_DOUBLE_QUOTE = '"'.toByte()
private const val LETTER_SINGLE_QUOTE = '\''.toByte()
private const val LETTER_BACKSLASH = '\\'.toByte()
private const val LETTER_FORWARDSLASH = '/'.toByte()
private const val LETTER_f = 'f'.toByte()
private const val LETTER_a = 'a'.toByte()
private const val LETTER_r = 'r'.toByte()
private const val LETTER_t = 't'.toByte()
private const val LETTER_n = 'n'.toByte()
private const val LETTER_b = 'b'.toByte()
private const val LETTER_e = 'e'.toByte()
private const val LETTER_E = 'E'.toByte()
private const val LETTER_u = 'u'.toByte()
private const val LETTER_A = 'A'.toByte()
private const val LETTER_F = 'F'.toByte()
private const val LETTER_EOF = (-1).toByte()
private const val LETTER_COMMA = ','.toByte()
private const val LETTER_0 = '0'.toByte()
private const val LETTER_1 = '1'.toByte()
private const val LETTER_2 = '2'.toByte()
private const val LETTER_3 = '3'.toByte()
private const val LETTER_4 = '4'.toByte()
private const val LETTER_5 = '5'.toByte()
private const val LETTER_6 = '6'.toByte()
private const val LETTER_7 = '7'.toByte()
private const val LETTER_8 = '8'.toByte()
private const val LETTER_9 = '9'.toByte()
private const val LETTER_MINUS = '-'.toByte()
private const val LETTER_PLUS = '+'.toByte()
private const val LETTER_DOT = '.'.toByte()

private val doubleConstants = doubleArrayOf(
  1e-323, 1e-322, 1e-321, 1e-320, 1e-319, 1e-318, 1e-317, 1e-316, 1e-315, 1e-314,
  1e-313, 1e-312, 1e-311, 1e-310, 1e-309, 1e-308, 1e-307, 1e-306, 1e-305, 1e-304,
  1e-303, 1e-302, 1e-301, 1e-300, 1e-299, 1e-298, 1e-297, 1e-296, 1e-295, 1e-294,
  1e-293, 1e-292, 1e-291, 1e-290, 1e-289, 1e-288, 1e-287, 1e-286, 1e-285, 1e-284,
  1e-283, 1e-282, 1e-281, 1e-280, 1e-279, 1e-278, 1e-277, 1e-276, 1e-275, 1e-274,
  1e-273, 1e-272, 1e-271, 1e-270, 1e-269, 1e-268, 1e-267, 1e-266, 1e-265, 1e-264,
  1e-263, 1e-262, 1e-261, 1e-260, 1e-259, 1e-258, 1e-257, 1e-256, 1e-255, 1e-254,
  1e-253, 1e-252, 1e-251, 1e-250, 1e-249, 1e-248, 1e-247, 1e-246, 1e-245, 1e-244,
  1e-243, 1e-242, 1e-241, 1e-240, 1e-239, 1e-238, 1e-237, 1e-236, 1e-235, 1e-234,
  1e-233, 1e-232, 1e-231, 1e-230, 1e-229, 1e-228, 1e-227, 1e-226, 1e-225, 1e-224,
  1e-223, 1e-222, 1e-221, 1e-220, 1e-219, 1e-218, 1e-217, 1e-216, 1e-215, 1e-214,
  1e-213, 1e-212, 1e-211, 1e-210, 1e-209, 1e-208, 1e-207, 1e-206, 1e-205, 1e-204,
  1e-203, 1e-202, 1e-201, 1e-200, 1e-199, 1e-198, 1e-197, 1e-196, 1e-195, 1e-194,
  1e-193, 1e-192, 1e-191, 1e-190, 1e-189, 1e-188, 1e-187, 1e-186, 1e-185, 1e-184,
  1e-183, 1e-182, 1e-181, 1e-180, 1e-179, 1e-178, 1e-177, 1e-176, 1e-175, 1e-174,
  1e-173, 1e-172, 1e-171, 1e-170, 1e-169, 1e-168, 1e-167, 1e-166, 1e-165, 1e-164,
  1e-163, 1e-162, 1e-161, 1e-160, 1e-159, 1e-158, 1e-157, 1e-156, 1e-155, 1e-154,
  1e-153, 1e-152, 1e-151, 1e-150, 1e-149, 1e-148, 1e-147, 1e-146, 1e-145, 1e-144,
  1e-143, 1e-142, 1e-141, 1e-140, 1e-139, 1e-138, 1e-137, 1e-136, 1e-135, 1e-134,
  1e-133, 1e-132, 1e-131, 1e-130, 1e-129, 1e-128, 1e-127, 1e-126, 1e-125, 1e-124,
  1e-123, 1e-122, 1e-121, 1e-120, 1e-119, 1e-118, 1e-117, 1e-116, 1e-115, 1e-114,
  1e-113, 1e-112, 1e-111, 1e-110, 1e-109, 1e-108, 1e-107, 1e-106, 1e-105, 1e-104,
  1e-103, 1e-102, 1e-101, 1e-100, 1e-99, 1e-98, 1e-97, 1e-96, 1e-95, 1e-94, 1e-93,
  1e-92, 1e-91, 1e-90, 1e-89, 1e-88, 1e-87, 1e-86, 1e-85, 1e-84, 1e-83, 1e-82, 1e-81,
  1e-80, 1e-79, 1e-78, 1e-77, 1e-76, 1e-75, 1e-74, 1e-73, 1e-72, 1e-71, 1e-70, 1e-69,
  1e-68, 1e-67, 1e-66, 1e-65, 1e-64, 1e-63, 1e-62, 1e-61, 1e-60, 1e-59, 1e-58, 1e-57,
  1e-56, 1e-55, 1e-54, 1e-53, 1e-52, 1e-51, 1e-50, 1e-49, 1e-48, 1e-47, 1e-46, 1e-45,
  1e-44, 1e-43, 1e-42, 1e-41, 1e-40, 1e-39, 1e-38, 1e-37, 1e-36, 1e-35, 1e-34, 1e-33,
  1e-32, 1e-31, 1e-30, 1e-29, 1e-28, 1e-27, 1e-26, 1e-25, 1e-24, 1e-23, 1e-22, 1e-21,
  1e-20, 1e-19, 1e-18, 1e-17, 1e-16, 1e-15, 1e-14, 1e-13, 1e-12, 1e-11, 1e-10, 1e-9,
  1e-8, 1e-7, 1e-6, 1e-5, 1e-4, 1e-3, 1e-2, 1e-1, 1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7,
  1e8, 1e9, 1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19, 1e20, 1e21,
  1e22, 1e23, 1e24, 1e25, 1e26, 1e27, 1e28, 1e29, 1e30, 1e31, 1e32, 1e33, 1e34, 1e35,
  1e36, 1e37, 1e38, 1e39, 1e40, 1e41, 1e42, 1e43, 1e44, 1e45, 1e46, 1e47, 1e48, 1e49,
  1e50, 1e51, 1e52, 1e53, 1e54, 1e55, 1e56, 1e57, 1e58, 1e59, 1e60, 1e61, 1e62, 1e63,
  1e64, 1e65, 1e66, 1e67, 1e68, 1e69, 1e70, 1e71, 1e72, 1e73, 1e74, 1e75, 1e76, 1e77,
  1e78, 1e79, 1e80, 1e81, 1e82, 1e83, 1e84, 1e85, 1e86, 1e87, 1e88, 1e89, 1e90, 1e91,
  1e92, 1e93, 1e94, 1e95, 1e96, 1e97, 1e98, 1e99, 1e100, 1e101, 1e102, 1e103, 1e104,
  1e105, 1e106, 1e107, 1e108, 1e109, 1e110, 1e111, 1e112, 1e113, 1e114, 1e115, 1e116,
  1e117, 1e118, 1e119, 1e120, 1e121, 1e122, 1e123, 1e124, 1e125, 1e126, 1e127, 1e128,
  1e129, 1e130, 1e131, 1e132, 1e133, 1e134, 1e135, 1e136, 1e137, 1e138, 1e139, 1e140,
  1e141, 1e142, 1e143, 1e144, 1e145, 1e146, 1e147, 1e148, 1e149, 1e150, 1e151, 1e152,
  1e153, 1e154, 1e155, 1e156, 1e157, 1e158, 1e159, 1e160, 1e161, 1e162, 1e163, 1e164,
  1e165, 1e166, 1e167, 1e168, 1e169, 1e170, 1e171, 1e172, 1e173, 1e174, 1e175, 1e176,
  1e177, 1e178, 1e179, 1e180, 1e181, 1e182, 1e183, 1e184, 1e185, 1e186, 1e187, 1e188,
  1e189, 1e190, 1e191, 1e192, 1e193, 1e194, 1e195, 1e196, 1e197, 1e198, 1e199, 1e200,
  1e201, 1e202, 1e203, 1e204, 1e205, 1e206, 1e207, 1e208, 1e209, 1e210, 1e211, 1e212,
  1e213, 1e214, 1e215, 1e216, 1e217, 1e218, 1e219, 1e220, 1e221, 1e222, 1e223, 1e224,
  1e225, 1e226, 1e227, 1e228, 1e229, 1e230, 1e231, 1e232, 1e233, 1e234, 1e235, 1e236,
  1e237, 1e238, 1e239, 1e240, 1e241, 1e242, 1e243, 1e244, 1e245, 1e246, 1e247, 1e248,
  1e249, 1e250, 1e251, 1e252, 1e253, 1e254, 1e255, 1e256, 1e257, 1e258, 1e259, 1e260,
  1e261, 1e262, 1e263, 1e264, 1e265, 1e266, 1e267, 1e268, 1e269, 1e270, 1e271, 1e272,
  1e273, 1e274, 1e275, 1e276, 1e277, 1e278, 1e279, 1e280, 1e281, 1e282, 1e283, 1e284,
  1e285, 1e286, 1e287, 1e288, 1e289, 1e290, 1e291, 1e292, 1e293, 1e294, 1e295, 1e296,
  1e297, 1e298, 1e299, 1e300, 1e301, 1e302, 1e303, 1e304, 1e305, 1e306, 1e307, 1e308
)

// This template utilizes the One Definition Rule to create global arrays in a
// header. As seen in:
// https://github.com/chadaustin/sajson/blob/master/include/sajson.h
// bit 0 (1) - set if: plain ASCII string character
// bit 1 (2) - set if: whitespace
// bit 4 (0x10) - set if: 0-9 e E .
private val parseFlags = byteArrayOf(
// 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
  0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 2, 0, 0, // 0
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 1
  3, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0x11, 1, // 2
  0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 1, 1, 1, 1, 1, 1, // 3
  1, 1, 1, 1, 1, 0x11, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 4
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, // 5
  1, 1, 1, 1, 1, 0x11, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 6
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 7

  // 128-255
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
)
