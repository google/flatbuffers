// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
// https://developers.google.com/protocol-buffers/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

package com.google.flatbuffers;

import static com.google.flatbuffers.UnsafeUtil.addressOffset;
import static java.lang.Character.MAX_SURROGATE;
import static java.lang.Character.MIN_SURROGATE;
import static java.lang.Character.isSurrogatePair;
import static java.lang.Character.toCodePoint;

import java.nio.ByteBuffer;
import java.util.Arrays;

/**
 * A set of low-level, high-performance static utility methods related
 * to the UTF-8 character encoding.  This class has no dependencies
 * outside of the core JDK libraries.
 *
 * <p>There are several variants of UTF-8.  The one implemented by
 * this class is the restricted definition of UTF-8 introduced in
 * Unicode 3.1, which mandates the rejection of "overlong" byte
 * sequences as well as rejection of 3-byte surrogate codepoint byte
 * sequences.  Note that the UTF-8 decoder included in Oracle's JDK
 * has been modified to also reject "overlong" byte sequences, but (as
 * of 2011) still accepts 3-byte surrogate codepoint byte sequences.
 *
 * <p>The byte sequences considered valid by this class are exactly
 * those that can be roundtrip converted to Strings and back to bytes
 * using the UTF-8 charset, without loss: <pre> {@code
 * Arrays.equals(bytes, new String(bytes, Internal.UTF_8).getBytes(Internal.UTF_8))
 * }</pre>
 *
 * <p>See the Unicode Standard,</br>
 * Table 3-6. <em>UTF-8 Bit Distribution</em>,</br>
 * Table 3-7. <em>Well Formed UTF-8 Byte Sequences</em>.
 */
final public class Utf8Unsafe extends Utf8 {

  @Override
  public int encodedLength(CharSequence in) {
    return Utf8Safe.computeEncodedLength(in);
  }

  private static String decodeUtf8Array(byte[] bytes, int index,
                                        int size) throws IllegalArgumentException {
    if ((index | size | bytes.length - index - size) < 0) {
      throw new ArrayIndexOutOfBoundsException(
          String.format("buffer length=%d, index=%d, size=%d", bytes.length, index, size));
    }

    int offset = index;
    final int limit = offset + size;

    // The longest possible resulting String is the same as the number of input bytes, when it is
    // all ASCII. For other cases, this over-allocates and we will truncate in the end.
    char[] resultArr = new char[size];
    int resultPos = 0;

    // Optimize for 100% ASCII (Hotspot loves small simple top-level loops like this).
    // This simple loop stops when we encounter a byte >= 0x80 (i.e. non-ASCII).
    while (offset < limit) {
      byte b = UnsafeUtil.getByte(bytes, offset);
      if (!DecodeUtil.isOneByte(b)) {
        break;
      }
      offset++;
      DecodeUtil.handleOneByte(b, resultArr, resultPos++);
    }

    while (offset < limit) {
      byte byte1 = UnsafeUtil.getByte(bytes, offset++);
      if (DecodeUtil.isOneByte(byte1)) {
        DecodeUtil.handleOneByte(byte1, resultArr, resultPos++);
        // It's common for there to be multiple ASCII characters in a run mixed in, so add an
        // extra optimized loop to take care of these runs.
        while (offset < limit) {
          byte b = UnsafeUtil.getByte(bytes, offset);
          if (!DecodeUtil.isOneByte(b)) {
            break;
          }
          offset++;
          DecodeUtil.handleOneByte(b, resultArr, resultPos++);
        }
      } else if (DecodeUtil.isTwoBytes(byte1)) {
        if (offset >= limit) {
          throw new IllegalArgumentException("Invalid UTF-8");
        }
        DecodeUtil.handleTwoBytes(
            byte1, /* byte2 */ UnsafeUtil.getByte(bytes, offset++), resultArr, resultPos++);
      } else if (DecodeUtil.isThreeBytes(byte1)) {
        if (offset >= limit - 1) {
          throw new IllegalArgumentException("Invalid UTF-8");
        }
        DecodeUtil.handleThreeBytes(
            byte1,
            /* byte2 */ UnsafeUtil.getByte(bytes, offset++),
            /* byte3 */ UnsafeUtil.getByte(bytes, offset++),
            resultArr,
            resultPos++);
      } else {
        if (offset >= limit - 2) {
          throw new IllegalArgumentException("Invalid UTF-8");
        }
        DecodeUtil.handleFourBytes(
            byte1,
            /* byte2 */ UnsafeUtil.getByte(bytes, offset++),
            /* byte3 */ UnsafeUtil.getByte(bytes, offset++),
            /* byte4 */ UnsafeUtil.getByte(bytes, offset++),
            resultArr,
            resultPos++);
        // 4-byte case requires two chars.
        resultPos++;
      }
    }

    if (resultPos < resultArr.length) {
      resultArr = Arrays.copyOf(resultArr, resultPos);
    }
    return UnsafeUtil.moveToString(resultArr);
  }

  private static String decodeUtf8Direct(ByteBuffer buffer, int index, int size)
      throws IllegalArgumentException {
    // Bitwise OR combines the sign bits so any negative value fails the check.
    if ((index | size | buffer.limit() - index - size) < 0) {
      throw new ArrayIndexOutOfBoundsException(
          String.format("buffer limit=%d, index=%d, limit=%d", buffer.limit(), index, size));
    }
    long address = UnsafeUtil.addressOffset(buffer) + index;
    final long addressLimit = address + size;

    // The longest possible resulting String is the same as the number of input bytes, when it is
    // all ASCII. For other cases, this over-allocates and we will truncate in the end.
    char[] resultArr = new char[size];
    int resultPos = 0;

    // Optimize for 100% ASCII (Hotspot loves small simple top-level loops like this).
    // This simple loop stops when we encounter a byte >= 0x80 (i.e. non-ASCII).
    while (address < addressLimit) {
      byte b = UnsafeUtil.getByte(address);
      if (!DecodeUtil.isOneByte(b)) {
        break;
      }
      address++;
      DecodeUtil.handleOneByte(b, resultArr, resultPos++);
    }

    while (address < addressLimit) {
      byte byte1 = UnsafeUtil.getByte(address++);
      if (DecodeUtil.isOneByte(byte1)) {
        DecodeUtil.handleOneByte(byte1, resultArr, resultPos++);
        // It's common for there to be multiple ASCII characters in a run mixed in, so add an
        // extra optimized loop to take care of these runs.
        while (address < addressLimit) {
          byte b = UnsafeUtil.getByte(address);
          if (!DecodeUtil.isOneByte(b)) {
            break;
          }
          address++;
          DecodeUtil.handleOneByte(b, resultArr, resultPos++);
        }
      } else if (DecodeUtil.isTwoBytes(byte1)) {
        if (address >= addressLimit) {
          throw new IllegalArgumentException("Invalid UTF-8");
        }
        DecodeUtil.handleTwoBytes(
            byte1, /* byte2 */ UnsafeUtil.getByte(address++), resultArr, resultPos++);
      } else if (DecodeUtil.isThreeBytes(byte1)) {
        if (address >= addressLimit - 1) {
          throw new IllegalArgumentException("Invalid UTF-8");
        }
        DecodeUtil.handleThreeBytes(
            byte1,
            /* byte2 */ UnsafeUtil.getByte(address++),
            /* byte3 */ UnsafeUtil.getByte(address++),
            resultArr,
            resultPos++);
      } else {
        if (address >= addressLimit - 2) {
          throw new IllegalArgumentException("Invalid UTF-8");
        }
        DecodeUtil.handleFourBytes(
            byte1,
            /* byte2 */ UnsafeUtil.getByte(address++),
            /* byte3 */ UnsafeUtil.getByte(address++),
            /* byte4 */ UnsafeUtil.getByte(address++),
            resultArr,
            resultPos++);
        // 4-byte case requires two chars.
        resultPos++;
      }
    }

    if (resultPos < resultArr.length) {
      resultArr = Arrays.copyOf(resultArr, resultPos);
    }
    return UnsafeUtil.moveToString(resultArr);
  }

  /**
   * Decodes the given UTF-8 portion of the {@link ByteBuffer} into a {@link String}.
   *
   * @throws IllegalArgumentException if the input is not valid UTF-8.
   */
  @Override
  public String decodeUtf8(ByteBuffer buffer, int index, int size)
      throws IllegalArgumentException {
    if (buffer.hasArray()) {
      final int offset = buffer.arrayOffset();
      return decodeUtf8Array(buffer.array(), offset + index, size);
    } else if (buffer.isDirect()) {
      return decodeUtf8Direct(buffer, index, size);
    }
    return Utf8Safe.decodeUtf8Buffer(buffer, index, size);
  }

  private static void encodeUtf8Direct(CharSequence in, ByteBuffer out) {
    final long address = addressOffset(out);
    long outIx = address + out.position();
    final long outLimit = address + out.limit();
    final int inLimit = in.length();
    if (inLimit > outLimit - outIx) {
      // Not even enough room for an ASCII-encoded string.
      throw new ArrayIndexOutOfBoundsException(
          "Failed writing " + in.charAt(inLimit - 1) + " at index " + out.limit());
    }

    // Designed to take advantage of
    // https://wikis.oracle.com/display/HotSpotInternals/RangeCheckElimination
    int inIx = 0;
    for (char c; inIx < inLimit && (c = in.charAt(inIx)) < 0x80; ++inIx) {
      UnsafeUtil.putByte(outIx++, (byte) c);
    }
    if (inIx == inLimit) {
      // We're done, it was ASCII encoded.
      out.position((int) (outIx - address));
      return;
    }

    for (char c; inIx < inLimit; ++inIx) {
      c = in.charAt(inIx);
      if (c < 0x80 && outIx < outLimit) {
        UnsafeUtil.putByte(outIx++, (byte) c);
      } else if (c < 0x800 && outIx <= outLimit - 2L) { // 11 bits, two UTF-8 bytes
        UnsafeUtil.putByte(outIx++, (byte) ((0xF << 6) | (c >>> 6)));
        UnsafeUtil.putByte(outIx++, (byte) (0x80 | (0x3F & c)));
      } else if ((c < MIN_SURROGATE || MAX_SURROGATE < c) && outIx <= outLimit - 3L) {
        // Maximum single-char code point is 0xFFFF, 16 bits, three UTF-8 bytes
        UnsafeUtil.putByte(outIx++, (byte) ((0xF << 5) | (c >>> 12)));
        UnsafeUtil.putByte(outIx++, (byte) (0x80 | (0x3F & (c >>> 6))));
        UnsafeUtil.putByte(outIx++, (byte) (0x80 | (0x3F & c)));
      } else if (outIx <= outLimit - 4L) {
        // Minimum code point represented by a surrogate pair is 0x10000, 17 bits, four UTF-8
        // bytes
        final char low;
        if (inIx + 1 == inLimit || !isSurrogatePair(c, (low = in.charAt(++inIx)))) {
          throw new UnpairedSurrogateException((inIx - 1), inLimit);
        }
        int codePoint = toCodePoint(c, low);
        UnsafeUtil.putByte(outIx++, (byte) ((0xF << 4) | (codePoint >>> 18)));
        UnsafeUtil.putByte(outIx++, (byte) (0x80 | (0x3F & (codePoint >>> 12))));
        UnsafeUtil.putByte(outIx++, (byte) (0x80 | (0x3F & (codePoint >>> 6))));
        UnsafeUtil.putByte(outIx++, (byte) (0x80 | (0x3F & codePoint)));
      } else {
        if ((MIN_SURROGATE <= c && c <= MAX_SURROGATE)
                && (inIx + 1 == inLimit || !isSurrogatePair(c, in.charAt(inIx + 1)))) {
          // We are surrogates and we're not a surrogate pair.
          throw new UnpairedSurrogateException(inIx, inLimit);
        }
        // Not enough space in the output buffer.
        throw new ArrayIndexOutOfBoundsException("Failed writing " + c + " at index " + outIx);
      }
    }

    // All bytes have been encoded.
    out.position((int) (outIx - address));
  }

  private static int encodeUtf8Array(final CharSequence in, final byte[] out,
                                     final int offset, final int length) {
    long outIx = offset;
    final long outLimit = outIx + length;
    final int inLimit = in.length();
    if (inLimit > length || out.length - length < offset) {
      // Not even enough room for an ASCII-encoded string.
      throw new ArrayIndexOutOfBoundsException(
          "Failed writing " + in.charAt(inLimit - 1) + " at index " + (offset + length));
    }

    // Designed to take advantage of
    // https://wikis.oracle.com/display/HotSpotInternals/RangeCheckElimination
    int inIx = 0;
    for (char c; inIx < inLimit && (c = in.charAt(inIx)) < 0x80; ++inIx) {
      UnsafeUtil.putByte(out, outIx++, (byte) c);
    }
    if (inIx == inLimit) {
      // We're done, it was ASCII encoded.
      return (int) outIx;
    }

    for (char c; inIx < inLimit; ++inIx) {
      c = in.charAt(inIx);
      if (c < 0x80 && outIx < outLimit) {
        UnsafeUtil.putByte(out, outIx++, (byte) c);
      } else if (c < 0x800 && outIx <= outLimit - 2L) { // 11 bits, two UTF-8 bytes
        UnsafeUtil.putByte(out, outIx++, (byte) ((0xF << 6) | (c >>> 6)));
        UnsafeUtil.putByte(out, outIx++, (byte) (0x80 | (0x3F & c)));
      } else if ((c < MIN_SURROGATE || MAX_SURROGATE < c) && outIx <= outLimit - 3L) {
        // Maximum single-char code point is 0xFFFF, 16 bits, three UTF-8 bytes
        UnsafeUtil.putByte(out, outIx++, (byte) ((0xF << 5) | (c >>> 12)));
        UnsafeUtil.putByte(out, outIx++, (byte) (0x80 | (0x3F & (c >>> 6))));
        UnsafeUtil.putByte(out, outIx++, (byte) (0x80 | (0x3F & c)));
      } else if (outIx <= outLimit - 4L) {
        // Minimum code point represented by a surrogate pair is 0x10000, 17 bits, four UTF-8
        // bytes
        final char low;
        if (inIx + 1 == inLimit || !isSurrogatePair(c, (low = in.charAt(++inIx)))) {
          throw new UnpairedSurrogateException((inIx - 1), inLimit);
        }
        int codePoint = toCodePoint(c, low);
        UnsafeUtil.putByte(out, outIx++, (byte) ((0xF << 4) | (codePoint >>> 18)));
        UnsafeUtil.putByte(out, outIx++, (byte) (0x80 | (0x3F & (codePoint >>> 12))));
        UnsafeUtil.putByte(out, outIx++, (byte) (0x80 | (0x3F & (codePoint >>> 6))));
        UnsafeUtil.putByte(out, outIx++, (byte) (0x80 | (0x3F & codePoint)));
      } else {
        if ((MIN_SURROGATE <= c && c <= MAX_SURROGATE)
                && (inIx + 1 == inLimit || !isSurrogatePair(c, in.charAt(inIx + 1)))) {
          // We are surrogates and we're not a surrogate pair.
          throw new UnpairedSurrogateException(inIx, inLimit);
        }
        // Not enough space in the output buffer.
        throw new ArrayIndexOutOfBoundsException("Failed writing " + c + " at index " + outIx);
      }
    }

    // All bytes have been encoded.
    return (int) outIx;
  }

  /**
   * Encodes an input character sequence ({@code in}) to UTF-8 in the target buffer ({@code out}).
   * Upon returning from this method, the {@code out} position will point to the position after
   * the last encoded byte. This method requires paired surrogates, and therefore does not
   * support chunking.
   *
   * <p>To ensure sufficient space in the output buffer, either call {@link #encodedLength} to
   * compute the exact amount needed, or leave room for
   * {@code Utf8Unsafe.MAX_BYTES_PER_CHAR * in.length()}, which is the largest possible number
   * of bytes that any input can be encoded to.
   *
   * @param in the source character sequence to be encoded
   * @param out the target buffer
   * @throws UnpairedSurrogateException if {@code in} contains ill-formed UTF-16 (unpaired
   *     surrogates)
   * @throws ArrayIndexOutOfBoundsException if {@code in} encoded in UTF-8 is longer than
   *     {@code out.remaining()}
   */
  @Override
  public void encodeUtf8(CharSequence in, ByteBuffer out) {
    if (out.hasArray()) {
      final int start = out.arrayOffset();
      int endIndex =
          encodeUtf8Array(in, out.array(), start + out.position(), out.remaining());
      out.position(endIndex - start);
    } else if (out.isDirect()) {
      encodeUtf8Direct(in, out);
    } else {
      Utf8Safe.encodeUtf8Buffer(in, out);
    }
  }
}
