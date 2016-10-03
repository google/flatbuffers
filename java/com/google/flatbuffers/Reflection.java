package com.google.flatbuffers;

import com.google.flatbuffers.reflection.BaseType;
import com.google.flatbuffers.reflection.Field;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

/**
 * Help class to manipulate flatbuffers document by reflection.
 */
public final class Reflection {
  private Reflection() {
    throw new AssertionError("Not for you!");
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
    field.type().baseType();
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

  private static void checkFieldType(byte actual, byte expected) {
    if (actual != expected) {
      throw new IllegalArgumentException("Invalid type expected: " + BaseType.name(expected) + ", but was: " + BaseType.name(actual));
    }
  }

}
