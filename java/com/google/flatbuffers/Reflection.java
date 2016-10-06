package com.google.flatbuffers;

import com.google.flatbuffers.reflection.BaseType;
import com.google.flatbuffers.reflection.Field;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import static com.google.flatbuffers.reflection.BaseType.Short;

/**
 * Help class to manipulate flatbuffers document by reflection.
 */
public final class Reflection {
  private Reflection() {
  }

  public static Table getRootTable(ByteBuffer _bb) {
    return getRootTable(_bb, new Table());
  }

  public static Table getRootTable(ByteBuffer _bb, Table obj) {
    _bb.order(ByteOrder.LITTLE_ENDIAN);
    return obj._init(_bb.getInt(_bb.position()) + _bb.position(), _bb);
  }

  public static boolean hasValue(Table table, Field field) {
    int o = table.__offset(field.offset());
    return o != 0;
  }

  public static short getShortField(Table table, Field field) {
    return getShortField(table, field, (short) field.defaultInteger());
  }

  public static short getShortField(Table table, Field field, short defaultValue) {
    checkFieldType(field.type().baseType(), Short);
    int o = table.__offset(field.offset());
    return o != 0 ? table.bb.getShort(o + table.bb_pos) : defaultValue;
  }

  public static boolean setShortField(Table table, Field field, short value) {
    int o = table.__offset(field.offset());
    if (o != 0) {
      table.bb.putShort(o + table.bb_pos, value);
      return true;
    } else {
      return false;
    }
  }

  /**
   * Check if the 2 given types are the same. If not throw an {@link IllegalArgumentException}
   *
   * @param actual   the actual type to check against the expected.
   * @param expected the expected type whose the actual must be equal to.
   * @throws IllegalArgumentException if actual and expected types are not equal.
   * @see BaseType
   */
  private static void checkFieldType(byte actual, byte expected) {
    if (actual != expected) {
      throw new IllegalArgumentException("Invalid type expected: " + BaseType.name(expected) + ", but was: " + BaseType.name(actual));
    }
  }

}
