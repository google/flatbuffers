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

import java.lang.reflect.Field;
import java.nio.Buffer;
import java.nio.ByteBuffer;
import java.security.AccessController;
import java.security.PrivilegedExceptionAction;

/** Utility class for working with unsafe operations. */
final class UnsafeUtil {
  private static final sun.misc.Unsafe UNSAFE = getUnsafe();
  private static final MemoryAccessor MEMORY_ACCESSOR = getMemoryAccessor();
  private static final boolean HAS_UNSAFE_BYTEBUFFER_OPERATIONS =
      supportsUnsafeByteBufferOperations();
  private static final boolean HAS_UNSAFE_ARRAY_OPERATIONS = supportsUnsafeArrayOperations();

  private static final long BYTE_ARRAY_BASE_OFFSET = arrayBaseOffset(byte[].class);

  private static final long BUFFER_ADDRESS_OFFSET = fieldOffset(bufferAddressField());

  private static final long STRING_VALUE_OFFSET = fieldOffset(stringValueField());

  private UnsafeUtil() {}

  static boolean hasUnsafeArrayOperations() {
    return HAS_UNSAFE_ARRAY_OPERATIONS;
  }

  static boolean hasUnsafeByteBufferOperations() {
    return HAS_UNSAFE_BYTEBUFFER_OPERATIONS;
  }

  private static int arrayBaseOffset(Class<?> clazz) {
    return HAS_UNSAFE_ARRAY_OPERATIONS ? MEMORY_ACCESSOR.arrayBaseOffset(clazz) : -1;
  }

  private static void putObject(Object target, long offset, Object value) {
    MEMORY_ACCESSOR.putObject(target, offset, value);
  }

  static byte getByte(byte[] target, long index) {
    return MEMORY_ACCESSOR.getByte(target, BYTE_ARRAY_BASE_OFFSET + index);
  }

  static void putByte(byte[] target, long index, byte value) {
    MEMORY_ACCESSOR.putByte(target, BYTE_ARRAY_BASE_OFFSET + index, value);
  }

  static byte getByte(long address) {
    return MEMORY_ACCESSOR.getByte(address);
  }

  static void putByte(long address, byte value) {
    MEMORY_ACCESSOR.putByte(address, value);
  }

  /**
   * Gets the offset of the {@code address} field of the given direct {@link ByteBuffer}.
   */
  static long addressOffset(ByteBuffer buffer) {
    return MEMORY_ACCESSOR.getLong(buffer, BUFFER_ADDRESS_OFFSET);
  }

  /**
   * Returns a new {@link String} backed by the given {@code chars}. The char array should not
   * be mutated any more after calling this function.
   */
  static String moveToString(char[] chars) {
    if (STRING_VALUE_OFFSET == -1) {
      // In the off-chance that this JDK does not implement String as we'd expect, just do a copy.
      return new String(chars);
    }
    final String str;
    try {
      str = (String) UNSAFE.allocateInstance(String.class);
    } catch (InstantiationException e) {
      // This should never happen, but return a copy as a fallback just in case.
      return new String(chars);
    }
    putObject(str, STRING_VALUE_OFFSET, chars);
    return str;
  }

  /**
   * Gets the {@code sun.misc.Unsafe} instance, or {@code null} if not available on this platform.
   */
  static sun.misc.Unsafe getUnsafe() {
    sun.misc.Unsafe unsafe = null;
    try {
      unsafe =
          AccessController.doPrivileged(
              new PrivilegedExceptionAction<sun.misc.Unsafe>() {
                @Override
                public sun.misc.Unsafe run() throws Exception {
                  Class<sun.misc.Unsafe> k = sun.misc.Unsafe.class;

                  for (Field f : k.getDeclaredFields()) {
                    f.setAccessible(true);
                    Object x = f.get(null);
                    if (k.isInstance(x)) {
                      return k.cast(x);
                    }
                  }
                  // The sun.misc.Unsafe field does not exist.
                  return null;
                }
              });
    } catch (Throwable e) {
      // Catching Throwable here due to the fact that Google AppEngine raises NoClassDefFoundError
      // for Unsafe.
    }
    return unsafe;
  }

  /** Get a {@link MemoryAccessor} appropriate for the platform, or null if not supported. */
  private static MemoryAccessor getMemoryAccessor() {
    if (UNSAFE == null) {
      return null;
    }
    return new JvmMemoryAccessor(UNSAFE);
  }

  /** Indicates whether or not unsafe array operations are supported on this platform. */
  private static boolean supportsUnsafeArrayOperations() {
    if (UNSAFE == null) {
      return false;
    }
    try {
      Class<?> clazz = UNSAFE.getClass();
      clazz.getMethod("objectFieldOffset", Field.class);
      clazz.getMethod("arrayBaseOffset", Class.class);
      clazz.getMethod("arrayIndexScale", Class.class);
      clazz.getMethod("getInt", Object.class, long.class);
      clazz.getMethod("putInt", Object.class, long.class, int.class);
      clazz.getMethod("getLong", Object.class, long.class);
      clazz.getMethod("putLong", Object.class, long.class, long.class);
      clazz.getMethod("getObject", Object.class, long.class);
      clazz.getMethod("putObject", Object.class, long.class, Object.class);
      clazz.getMethod("getByte", Object.class, long.class);
      clazz.getMethod("putByte", Object.class, long.class, byte.class);
      clazz.getMethod("getBoolean", Object.class, long.class);
      clazz.getMethod("putBoolean", Object.class, long.class, boolean.class);
      clazz.getMethod("getFloat", Object.class, long.class);
      clazz.getMethod("putFloat", Object.class, long.class, float.class);
      clazz.getMethod("getDouble", Object.class, long.class);
      clazz.getMethod("putDouble", Object.class, long.class, double.class);

      return true;
    } catch (Throwable e) {
      return false;
    }
  }

  private static boolean supportsUnsafeByteBufferOperations() {
    if (UNSAFE == null) {
      return false;
    }
    try {
      Class<?> clazz = UNSAFE.getClass();
      // Methods for getting direct buffer address.
      clazz.getMethod("objectFieldOffset", Field.class);
      clazz.getMethod("getLong", Object.class, long.class);

      if (bufferAddressField() == null) {
        return false;
      }

      clazz.getMethod("getByte", long.class);
      clazz.getMethod("putByte", long.class, byte.class);
      clazz.getMethod("getInt", long.class);
      clazz.getMethod("putInt", long.class, int.class);
      clazz.getMethod("getLong", long.class);
      clazz.getMethod("putLong", long.class, long.class);
      clazz.getMethod("copyMemory", long.class, long.class, long.class);
      clazz.getMethod("copyMemory", Object.class, long.class, Object.class, long.class, long.class);
      return true;
    } catch (Throwable e) {
      return false;
    }
  }


  /** Finds the address field within a direct {@link Buffer}. */
  private static Field bufferAddressField() {
    Field field = field(Buffer.class, "address");
    return field != null && field.getType() == long.class ? field : null;
  }

  /** Finds the value field within a {@link String}. */
  private static Field stringValueField() {
    Field field = field(String.class, "value");
    return field != null && field.getType() == char[].class ? field : null;
  }

  /**
   * Returns the offset of the provided field, or {@code -1} if {@code sun.misc.Unsafe} is not
   * available.
   */
  private static long fieldOffset(Field field) {
    return field == null || MEMORY_ACCESSOR == null ? -1 : MEMORY_ACCESSOR.objectFieldOffset(field);
  }

  /**
   * Gets the field with the given name within the class, or {@code null} if not found. If found,
   * the field is made accessible.
   */
  private static Field field(Class<?> clazz, String fieldName) {
    Field field;
    try {
      field = clazz.getDeclaredField(fieldName);
      field.setAccessible(true);
    } catch (Throwable t) {
      // Failed to access the fields.
      field = null;
    }
    return field;
  }

  private abstract static class MemoryAccessor {

    sun.misc.Unsafe unsafe;

    MemoryAccessor(sun.misc.Unsafe unsafe) {
      this.unsafe = unsafe;
    }

    final long objectFieldOffset(Field field) {
      return unsafe.objectFieldOffset(field);
    }

    public abstract byte getByte(Object target, long offset);

    public abstract void putByte(Object target, long offset, byte value);

    public final long getLong(Object target, long offset) {
      return unsafe.getLong(target, offset);
    }

    final void putObject(Object target, long offset, Object value) {
      unsafe.putObject(target, offset, value);
    }

    final int arrayBaseOffset(Class<?> clazz) {
      return unsafe.arrayBaseOffset(clazz);
    }

    public abstract byte getByte(long address);

    public abstract void putByte(long address, byte value);
  }

  private static final class JvmMemoryAccessor extends MemoryAccessor {

    JvmMemoryAccessor(sun.misc.Unsafe unsafe) {
      super(unsafe);
    }

    @Override
    public byte getByte(long address) {
      return unsafe.getByte(address);
    }

    @Override
    public void putByte(long address, byte value) {
      unsafe.putByte(address, value);
    }

    @Override
    public byte getByte(Object target, long offset) {
      return unsafe.getByte(target, offset);
    }

    @Override
    public void putByte(Object target, long offset, byte value) {
      unsafe.putByte(target, offset, value);
    }
  }

}
